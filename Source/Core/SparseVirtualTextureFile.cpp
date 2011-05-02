#include <HAGE.h>

namespace HAGE {
	
SparseVirtualTextureFile::SparseVirtualTextureFile() : _dataCache(nullptr),_dataStream(nullptr),_dataSource(nullptr)
{
	_bOpen = false;
}
SparseVirtualTextureFile::~SparseVirtualTextureFile()
{
	if(_dataCache)
		delete _dataCache;
}

void SparseVirtualTextureFile::Open(const ISVTFileDataSource* source)
{
	_bOpen = false;
	if(_dataCache)
		delete _dataCache;
	_dataSource = source;
	ISVTFileDataSource::SVTSettings settings;
	source->GetSettings(&settings);
	_fileHeader.fourCC[0] = 'H';
	_fileHeader.fourCC[1] = 'S';
	_fileHeader.fourCC[2] = 'V';
	_fileHeader.fourCC[3] = 'T';
	_fileHeader.bordersize = settings.borderSize;
	_fileHeader.pagedepth = settings.pageDepth;
	_fileHeader.majorVersion = 1;
	_fileHeader.minorVersion = 0;
	_fileHeader.pagesize = settings.pageSize;
	_fileHeader.nSharedDataHeaders = settings.nSharedDataEntries;
	u32 nPages = GetTotalNumPagesUntilDepth(_fileHeader.pagedepth-1);
	_fileHeader.nPageHeaders = nPages;
	_pageHeaders.resize(nPages);

	_dataCache = new SharedDataCache(source);
	_bOpen = true;
}

void SparseVirtualTextureFile::Open(IDataStream* pDataStream)
{
	_bOpen = false;
	_dataStream = pDataStream;
	if(!_dataStream)
		return;

	size_t nRead = _dataStream->Read(sizeof(FileHeader),(u8*)&_fileHeader);
	if(nRead != sizeof(FileHeader))
		return;

	if( _fileHeader.fourCC[0] != 'H' ||
		_fileHeader.fourCC[1] != 'S' ||
		_fileHeader.fourCC[2] != 'V' ||
		_fileHeader.fourCC[3] != 'T')
	{
		return;
	}

	//read page headers
	if(_fileHeader.nPageHeaders)
	{
		_dataStream->Seek(_fileHeader.offsetPageHeaders,IDataStream::ORIGIN_BEGINNING);
		u64 nPages = _fileHeader.nPageHeaders;
		_pageHeaders.resize(nPages);
		nRead = _dataStream->Read(sizeof(PageHeader)*_pageHeaders.size(),(u8*)_pageHeaders.data());

		if(nRead != sizeof(PageHeader)*_pageHeaders.size())
			return;
	}

	//read shared data headers
	if(_fileHeader.nSharedDataHeaders)
	{
		_dataStream->Seek(_fileHeader.offsetSharedDataHeaders,IDataStream::ORIGIN_BEGINNING);
		u64 nHeaders = _fileHeader.nSharedDataHeaders;
		std::vector<SharedDataHeader> dataHeaders(nHeaders);
		nRead = _dataStream->Read(sizeof(SharedDataHeader)*dataHeaders.size(),(u8*)dataHeaders.data());

		if(nRead != sizeof(SharedDataHeader)*dataHeaders.size())
			return;

		_dataCache = new SharedDataCache(dataHeaders,pDataStream);
	}
	else
		_dataCache = nullptr;


	_bOpen = true;
}

void SparseVirtualTextureFile::Save(const char* destFile) const
{
	assert(_dataStream == nullptr);
	assert(_dataSource != nullptr);

	FILE* target = fopen(destFile,"w+b");

	assert(target);
			
	FileHeader fileHeader = _fileHeader;
	std::vector<PageHeader> pageHeaders(_pageHeaders.size());
	std::vector<SharedDataHeader> sharedHeaders;

	/*temporary space for file header*/
	size_t written = fwrite(&fileHeader,sizeof(FileHeader),1,target);
	assert(written == 1);


	/*temporary space for page headers*/
	if(pageHeaders.size())
	{
		fileHeader.nPageHeaders = pageHeaders.size();
		fileHeader.offsetPageHeaders = _ftelli64(target);

		size_t written = fwrite(pageHeaders.data(),sizeof(PageHeader),pageHeaders.size(),target);
		assert(written == pageHeaders.size());
	}
	else
	{
		fileHeader.nPageHeaders= 0;
		fileHeader.offsetPageHeaders = 0;
	}

	/*temporary space for shared data headers*/
	if(_dataCache)
	{
		fileHeader.offsetSharedDataHeaders = _ftelli64(target);
		fileHeader.nSharedDataHeaders = _dataCache->writeHeaders(target,sharedHeaders);
	}
	else
	{
		fileHeader.offsetSharedDataHeaders = 0;
		fileHeader.nSharedDataHeaders= 0;
	}

	// write pages
	if(pageHeaders.size())
	{
		std::vector<u8> tempBuffer(maxPageSize);
		SVTPage page(fileHeader.pagesize);
		for(size_t i = 0; i < pageHeaders.size(); ++i)
		{
			pageHeaders[i].file_offset =  _ftelli64(target);
			getPage((u64)i,&page);
			// LZMA here for much better compression ratio
			pageHeaders[i].page_size = page.Serialize(maxPageSize,tempBuffer.data(),&pageHeaders[i].pageHeader,SVTPage::PAGE_COMPRESSED_LZMA);
			written = fwrite(tempBuffer.data(),1,pageHeaders[i].page_size,target);
			assert(written == pageHeaders[i].page_size);
		}
	}
	else
		assert( fileHeader.nPageHeaders == 0 );

	// write shared data
	if(_dataCache)
		assert( _dataCache->writeSharedData(target,sharedHeaders) == fileHeader.nSharedDataHeaders);
	else
		assert( fileHeader.nSharedDataHeaders == 0 );

	assert( _fseeki64(target,0,SEEK_SET) == 0);
			
	/*final file header*/
	written = fwrite(&fileHeader,sizeof(FileHeader),1,target);
	assert(written == 1);

	/*final page headers*/
	if(pageHeaders.size())
	{
		assert( fileHeader.nPageHeaders == pageHeaders.size() );
		assert( fileHeader.offsetPageHeaders == _ftelli64(target) );

		size_t written = fwrite(pageHeaders.data(),sizeof(PageHeader),pageHeaders.size(),target);
		assert(written == pageHeaders.size());
	}
	else
		assert( fileHeader.nPageHeaders== 0);

	/*final data headers*/
	if(_dataCache)
	{
		assert( fileHeader.offsetSharedDataHeaders == _ftelli64(target) );
		assert( fileHeader.nSharedDataHeaders == _dataCache->writeHeaders(target,sharedHeaders) );
	}
	else
		assert( fileHeader.offsetSharedDataHeaders == 0 );

	fclose(target);
}
		
void SparseVirtualTextureFile::ReadImageFromVirtualTexture(u64 vtXOffset, u64 vtYOffset,ImageRect<R8G8B8A8>& pDataOut,int level) const
{
	const u32 vtBorderSize = (GetPageOuterSize() - GetPageInnerSize())/2;
	const u32 vtPageSize = GetPageInnerSize();
	const u32 vtXPageBegin = vtXOffset / vtPageSize;
	const u32 vtYPageBegin = vtYOffset / vtPageSize;
	const u32 vtXPageEnd = (vtXOffset + pDataOut.XSize() -1)/ vtPageSize + 1;
	const u32 vtYPageEnd = (vtYOffset + pDataOut.YSize() -1)/ vtPageSize + 1;
	const u32 vtXPixelBegin = vtXOffset % vtPageSize;
	const u32 vtYPixelBegin = vtYOffset % vtPageSize;
	const u32 vtXPixelEnd = (vtXOffset+ pDataOut.XSize() - 1) % vtPageSize + 1;
	const u32 vtYPixelEnd = (vtYOffset+ pDataOut.YSize() - 1) % vtPageSize + 1;
	if(level == -1)
		level = GetMaxDepth();
			
	for(u64 iYPage = vtYPageBegin; iYPage < vtYPageEnd ; ++ iYPage)
		for(u64 iXPage = vtXPageBegin; iXPage < vtXPageEnd ; ++ iXPage)
		{
			const u32 pXPixelBegin = (iXPage==vtXPageBegin)?(vtXPixelBegin):0;
			const u32 pYPixelBegin = (iYPage==vtYPageBegin)?(vtYPixelBegin):0;
			const u32 pXPixelEnd = (iXPage==vtXPageEnd-1)?(vtXPixelEnd):vtPageSize;
			const u32 pYPixelEnd = (iYPage==vtYPageEnd-1)?(vtYPixelEnd):vtPageSize;
			const u32 ImageXOffset = iXPage*vtPageSize - vtXOffset;
			const u32 ImageYOffset = iYPage*vtPageSize - vtYOffset;
			const u32 xSize = pXPixelEnd - pXPixelBegin;
			const u32 ySize = pYPixelEnd - pYPixelBegin;
			const u32 nPage = GetPageIndex(iXPage,iYPage,level);

			ReadPage(nPage,Vector2<u32>(vtBorderSize+pXPixelBegin,vtBorderSize+pYPixelBegin),pDataOut.Rect(Vector4<u32>(ImageXOffset,ImageYOffset,ImageXOffset+xSize,ImageYOffset+ySize)));
		}
}
		
bool SparseVirtualTextureFile::ReadPage(u64 PageIndex,const Vector2<u32>& offset,ImageRect<R8G8B8A8>& pDataOut) const
{
	assert(offset.x + pDataOut.XSize() <= GetPageOuterSize());
	assert(offset.y + pDataOut.YSize() <= GetPageOuterSize());
	SVTPage page(GetPageOuterSize());
	getPage(PageIndex,&page);
	const ISVTDataLayer* pLayer = page.GetLayer(SVTLAYER_DIFFUSE_COLOR);
	if(pLayer)
	{
		assert(pLayer);
		pLayer->GetImageData(offset,SVTLAYER_DIFFUSE_COLOR,pDataOut);
		return true;
	}
	else
		return false;
}

void SparseVirtualTextureFile::getPage(u64 index,SVTPage* pPageOut) const
{
	assert(_bOpen);
	if(_dataSource)
		_dataSource->ReadPage(index,pPageOut);
	else
	{
		_dataStream->Seek( _pageHeaders[index].file_offset , IDataStream::ORIGIN_BEGINNING );
		std::vector<u8> dataBuffer( _pageHeaders[index].page_size );
		u64 read = _dataStream->Read(dataBuffer.size(), dataBuffer.data());
		assert(read == dataBuffer.size());
		pPageOut->Deserialize(_fileHeader.pagesize,&_pageHeaders[index].pageHeader,read,dataBuffer.data(),_dataCache);
	}
}

		
SparseVirtualTextureFile::SharedDataCache::SharedDataCache(const ISVTFileDataSource* source)
{
	_dataSource = source;
	_dataStream = nullptr;

	ISVTFileDataSource::SVTSettings settings;
	source->GetSettings(&settings);
	_sharedDataHeaders.resize( settings.nSharedDataEntries );
}

SparseVirtualTextureFile::SharedDataCache::SharedDataCache(const std::vector<SharedDataHeader>& sharedEntries,IDataStream* dataStream)
{
	_dataSource = nullptr;
	_sharedDataHeaders = sharedEntries;
	_dataStream = dataStream;
}

SparseVirtualTextureFile::SharedDataCache::~SharedDataCache()
{
	for(auto it = _dataCache.begin(); it != _dataCache.end(); ++it)
	{
		assert(it->second.refCount == 0);
		delete it->second.pData;
	}
}

u32 SparseVirtualTextureFile::SharedDataCache::SharedDataSize(u32 index) const
{
	if(_dataSource)
		return _dataSource->SharedDataSize(index);
	else
		return _sharedDataHeaders[index].data_size;
}

const void* SparseVirtualTextureFile::SharedDataCache::AccessSharedData(u32 index) const
{
	if(_dataSource)
		return _dataSource->AccessSharedData(index);

			
	auto found = _dataCache.find(index);

	if(found != _dataCache.end())
	{
		found->second.refCount = found->second.refCount +1;
		return found->second.pData;
	}

	if(_dataCache.size() > max_items)
		garbageCollect();

	dataEntry new_entry;
	new_entry.refCount = 1;
	new_entry.pData = new u8[SharedDataSize(index)];

	_dataStream->Seek( _sharedDataHeaders[index].file_offset , IDataStream::ORIGIN_BEGINNING );
	_dataStream->Read( SharedDataSize(index),(u8*)new_entry.pData);

	_dataCache.insert(std::make_pair(index,new_entry));

	return new_entry.pData;
}

void SparseVirtualTextureFile::SharedDataCache::ReleaseSharedData(u32 index) const
{
	if(_dataSource)
		return _dataSource->ReleaseSharedData(index);
			
	auto found = _dataCache.find(index);

	assert(found != _dataCache.end());

	--found->second.refCount;
}

u32 SparseVirtualTextureFile::SharedDataCache::writeHeaders(FILE* file,std::vector<SharedDataHeader>& headers) const
{
	if(headers.size() != _sharedDataHeaders.size())
		headers.resize(_sharedDataHeaders.size());
	size_t written = fwrite(headers.data(),sizeof(SharedDataHeader),headers.size(),file);
	assert(written == headers.size());
	return written;
}

u32 SparseVirtualTextureFile::SharedDataCache::writeSharedData(FILE* file,std::vector<SharedDataHeader>& headers) const
{
	for(size_t i = 0; i < headers.size(); ++i)
	{
		headers[i].importance = 0;
		headers[i].data_flags = 0;
		headers[i].file_offset = _ftelli64(file);
		headers[i].data_size = SharedDataSize(i);

		const void* data = AccessSharedData(i);

		size_t written = fwrite(data,headers[i].data_size,1,file);

		assert(written == 1);

		ReleaseSharedData(i);
	}
	return headers.size();
}
		
void SparseVirtualTextureFile::SharedDataCache::garbageCollect() const
{
	for(auto it = _dataCache.begin(); it != _dataCache.end();)
	{
		if(it->second.refCount == 0)
		{
			auto last = it;
			delete last->second.pData;
			++it;
			_dataCache.erase(last);
		}
		else
			++it;
	}
}

}