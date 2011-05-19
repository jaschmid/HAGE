#include "VirtualTextureGenerator.h"
#include "../source/core/dependancies/Snappy/snappy.h"

#include <stdio.h>


namespace HAGE
{


bool SparseVirtualTextureGenerator::New(u32 pagesize,u32 pagedepth, u32 bordersize)
{
	_settings.borderSize = bordersize;
	_settings.nSharedDataEntries = 0;
	_settings.pageDepth = pagedepth;
	_settings.pageSize = pagesize;
	_settings.pageInnerSize = _settings.pageSize - 2*_settings.borderSize;
	
	if(_vtPacker)
		delete _vtPacker;
	if(_sourceDataStorage)
		delete _sourceDataStorage;
	if(_sharedDataStorage)
		delete _sharedDataStorage;
	if(_pageDataStorage)
		delete _pageDataStorage;

	_vtPacker = new VTPacker(getInnerPageSize(),pagedepth);

	_sharedDataStorage = new TempSharedStorage();
	_sourceDataStorage = new TempImageStorage(_sharedDataStorage);
	_pageDataStorage = new TempPageStorage(pagesize,pagedepth,_sharedDataStorage);
	

	{/*
		FreeImage_Initialise();
		FIBITMAP* pBitmap = FreeImage_Load(FIF_PNG,"AlignTest.png",0);
		SVTDataLayer_Raw rawImage;
		ImageData<R8G8B8A8> image_data(1024,1024);
		for(int y = 0; y < 1024; ++y)
			for(int x = 0; x < 1024; ++x)
			{
				RGBQUAD color;
				FreeImage_GetPixelColor(pBitmap,x,y,&color);
				image_data(x,y).SetData(*(u32*)&color);
			}
		rawImage.Initialize(image_data);
		u32 storageIndex = _sourceDataStorage->StoreImage(&rawImage);

		std::vector<std::pair<VTPacker::Packing,f32>> internalRelationArray;
		
		PlacedTexture placement( this,_vtPacker->AddItem(internalRelationArray,image_data.XSize(),image_data.YSize()),storageIndex);
	
		_textureList.push_back(placement);
		FreeImage_Unload(pBitmap);*/
	}

	return true;
}

void SparseVirtualTextureGenerator::Commit()
{
	//make sure all textures are placed

	printf("Finalizing Texture placement\n");

	FinalizePlacement();

	// write the placed textures to file (lowest level)
	
	u32 totalTextures = _textureList.size();

	printf("Rendering HSVT Base Texture (%u textures): ", totalTextures);

	u32 current = 0;
	u32 step = totalTextures / 10;

	for(auto it = _textureList.begin(); it != _textureList.end(); ++it)
	{

		u32 xLoc = it->packing.GetX();
		u32 yLoc = it->packing.GetY();
		u32 xSize = it->packing.GetXSize();
		u32 ySize = it->packing.GetYSize();

		ISVTDataLayer* pTexture = _sourceDataStorage->RetrieveImage(it->savedImage);

		assert(xSize == pTexture->GetSize().x);
		assert(ySize == pTexture->GetSize().y);

		writeImageToMipLayer(pTexture, SVTLAYER_DIFFUSE_COLOR ,getNumLayers() -1 , Vector2<u32>(xLoc,yLoc));

		delete pTexture;
		++current;
		if(current % step == 0)
			printf(".");
	}

	printf("\nGenerating Mipmaps:\n");

	//make mipmaps
	finalizeDepthLayer(0);

	//debug stuff
	/*
	std::vector<Color> tempBuffer;
	tempBuffer.resize(GetPageOuterPixelCount());

	// without borders
	for(int i = 0; i< std::min((int)GetNumLayers(),(int)5);++i)
	{
		std::vector<u32> buffer;
		u32 xSize = GetPageInnerSize()*GetNumXPagesAtDepth(i);
		buffer.resize(xSize*xSize);

		mergePagesToImage(
					0,GetNumXPagesAtDepth(i),
					0,GetNumXPagesAtDepth(i),
					i,(Color*)buffer.data(),0,tempBuffer);

		char temp[256];
		sprintf(temp,"MipLevel%i.png",i);
				
		OutputImage(temp,(Color*)buffer.data(),xSize,xSize);
	}

	//with borders
	for(int i = 0; i< std::min((int)GetNumLayers(),(int)5);++i)
	{
		std::vector<u32> buffer;
		u32 xSize = GetPageOuterSize()*GetNumXPagesAtDepth(i);
		buffer.resize(xSize*xSize);

		mergePagesToImage(
					0,GetNumXPagesAtDepth(i),
					0,GetNumXPagesAtDepth(i),
					i,(Color*)buffer.data(),nullptr,tempBuffer,SparseVirtualTextureFile::PAGE_FINAL);

		char temp[256];
		sprintf(temp,"MipLevel_borders_%i.png",i);

		OutputImage(temp,(Color*)buffer.data(),xSize,xSize);
	}
	/*
	//test image
	{
		std::vector<Color> buffer;
		u32 xSize = 4096;
		buffer.resize(xSize*xSize);

		ReadImageFromVirtualTexture(0,0,xSize,xSize,buffer.data(),6);

		OutputImage("Landscape.png",(Color*)buffer.data(),xSize,xSize);
	}*/



	//SparseVirtualTextureFile::Commit();
}
	
/*
void SparseVirtualTextureGenerator::applyBoxFilter(const SparseVirtualTextureGenerator::BorderedImageReader& source,u32 InputBorder,Color* pOutput,u32 xOutputSize,u32 yOutputSize)
{
	static const u32 FilterSize = 2;
	static const float base = 1.0f/4.0f;
	static const float base_filter[FilterSize][FilterSize] = 
	{
		base,base,
		base,base,
	};

	float trapez_filter[FilterSize+1][FilterSize+1];
	u32 trapez_size;

	//our trapez filter only supports even output sizes
	assert(xOutputSize%2 == 0);
	assert(yOutputSize%2 == 0);
				
	if(FilterSize%2 == 1)
	{
		trapez_size = FilterSize+1;
		for(u32 fy = 0; fy < trapez_size; ++fy)
			for(u32 fx = 0; fx < trapez_size; ++fx)
				trapez_filter[fy][fx] = 0.0f;

		for(u32 fy = 0; fy < FilterSize; ++fy)
			for(u32 fx = 0; fx < FilterSize; ++fx)
			{
				trapez_filter[fy][fx] += 0.25f*base_filter[fy][fx];
				trapez_filter[fy+1][fx] += 0.25f*base_filter[fy][fx];
				trapez_filter[fy][fx+1] += 0.25f*base_filter[fy][fx];
				trapez_filter[fy+1][fx+1] += 0.25f*base_filter[fy][fx];
			}
	}
	else
	{
		trapez_size = FilterSize;
		for(u32 fy = 0; fy < trapez_size; ++fy)
			for(u32 fx = 0; fx < trapez_size; ++fx)
				trapez_filter[fy][fx] = base_filter[fy][fx];
	}
		*/	
	/*
	float Filter[xFilterSize][yFilterSize] = {
		0.25f,0.25f,
		0.25f,1.0f/9.0f,1.0f/9.0f,
		base/2.0f,base,base,base/2.0f

	};*/
	/*
	assert( source.GetInnerX() == 2*InputBorder + 2*xOutputSize );
	assert( source.GetInnerY() == 2*InputBorder + 2*yOutputSize );

	for(u32 iy = 0; iy < yOutputSize; iy++)
		for(u32 ix = 0; ix < xOutputSize; ix++)
		{
			u32 xInput = InputBorder - (trapez_size-1)/2 + ix*2;
			u32 yInput = InputBorder - (trapez_size-1)/2 + iy*2;
			float red=0.0f,green=0.0f,blue=0.0f,alpha=0.0f;
			for(u32 fy = 0; fy < trapez_size; ++fy)
				for(u32 fx = 0; fx < trapez_size; ++fx)
				{
					const Color& c = source.GetPixelFromInner(xInput+fx,yInput+fy);
					float f= trapez_filter[fy][fx];
					red += f*(float)c.r;
					green += f*(float)c.g;
					blue += f*(float)c.b;
					alpha += f*(float)c.a;
				}

			Color out;
			out.r = (u8)std::max(std::min(red,255.0f),0.0f);
			out.g = (u8)std::max(std::min(green,255.0f),0.0f);
			out.b = (u8)std::max(std::min(blue,255.0f),0.0f);
			out.a = (u8)std::max(std::min(alpha,255.0f),0.0f);
			pOutput[iy*xOutputSize + ix] = out;
		}
}


void SparseVirtualTextureGenerator::splitPagesFromImage(i64 xPageBegin,u64 xPageEnd,u64 yPageBegin,i64 yPageEnd,u32 depth,const Color* pIn,const std::vector<bool>* pageStatus,std::vector<Color>& tempBuffer,u32 flags,u64 inputOffset, u64 inputStride)
{
	i64 nPageSize = GetPageInnerSize();
	if(flags & PAGE_FINAL)
		nPageSize = GetPageOuterSize();


	i64 nXPages = xPageEnd-xPageBegin;
	i64 nYPages = yPageEnd-yPageBegin;
	i64 nXPixels = nXPages*nPageSize;
	if(inputStride)
		nXPixels = inputStride;
	i64 NumXPages =		GetNumXPagesAtDepth(depth);
				
	for(i64 uYPage = 0; uYPage < nYPages; uYPage++)
		for(i64 uXPage = 0; uXPage < nXPages; uXPage++)
			if(pageStatus == nullptr || (*pageStatus)[uYPage*nXPages+uXPage])
			{
				i64 iXAbsolute = (i64)(uXPage + xPageBegin) ;
				i64 iYAbsolute = (i64)(uYPage + yPageBegin) ;
					
				//copy tempBuffer to inputBuffer

				for(int iYPixel = 0; iYPixel < (int)nPageSize; iYPixel++)
					for(int iXPixel = 0; iXPixel < (int)nPageSize; iXPixel++)
						tempBuffer[iYPixel * nPageSize + iXPixel] = pIn[inputOffset+(iYPixel+uYPage*nPageSize)*nXPixels + iXPixel + uXPage*nPageSize];
					
				WritePage(GetPageIndex(iXAbsolute,iYAbsolute,depth),tempBuffer.data(),flags);
			}
}

bool SparseVirtualTextureGenerator::mergePagesToImage(i32 xPageBegin,i32 xPageEnd,i32 yPageBegin,i32 yPageEnd,u32 depth,Color* pOut,std::vector<bool>* pageStatus,std::vector<Color>& tempBuffer,u32 flags)
{
	bool result = false;
	i64 nPageSize = GetPageInnerSize();
	if(flags & PAGE_FINAL)
		nPageSize = GetPageOuterSize();
	i64 nPagePixels = nPageSize*nPageSize;
	i64 nXPages = xPageEnd-xPageBegin;
	i64 nYPages = yPageEnd-yPageBegin;
	i64 nXPixels = nXPages*nPageSize;
	i64 TotalXPages =		GetNumXPagesAtDepth(depth);
	
			
	for(i64 uYPage = 0; uYPage < nYPages; uYPage++)
		for(i64 uXPage = 0; uXPage < nXPages; uXPage++)
		{
			i64 iXAbsolute = (i64)(uXPage + xPageBegin) ;
			i64 iYAbsolute = (i64)(uYPage + yPageBegin) ;

			//magic wrap around

			iXAbsolute = (iXAbsolute+TotalXPages)%TotalXPages;
			iYAbsolute = (iYAbsolute+TotalXPages)%TotalXPages;

			bool status=ReadPage(GetPageIndex(iXAbsolute,iYAbsolute,depth),tempBuffer.data(),flags);
					
			if(pageStatus)
				(*pageStatus)[uYPage*nXPages+uXPage] = status;

			//copy tempBuffer to inputBuffer
			if(status)
			{
				result = true;
				for(int iXPixel = 0; iXPixel < (int)nPageSize; iXPixel++)
					for(int iYPixel = 0; iYPixel < (int)nPageSize; iYPixel++)
						pOut[(iYPixel+uYPage*nPageSize)*nXPixels + iXPixel + uXPage*nPageSize] =
							tempBuffer[iYPixel * nPageSize + iXPixel];
			}
		}
	return result;
}*/

bool SparseVirtualTextureGenerator::finalizeDepthLayer(u32 depth) 
{
	if(depth >= getNumLayers())
		return true;

			
	finalizeDepthLayer(depth+1);
			
	if(depth == 0)
		return true;

	printf("Finalizing Depth Layer %u ",depth);

	static const u64 maxMemoryForFinalizing = 16*1024*1024; //16 MB for finalization
	const u32 maxInputPages = (maxMemoryForFinalizing*4/5)/getPageOuterPixelCount()/sizeof(Color)-1;
	const u32 maxInputBlockDimension = ((u64)sqrt((float)maxInputPages))/2*2;
	const u32 maxInputBlockInnerDimension = maxInputBlockDimension-2;
	const u32 maxOutputBlockDimension = (maxInputBlockDimension-2)/2;
	const u32 maxOutputPages = maxOutputBlockDimension * maxOutputBlockDimension;
	const u32 InputDimension = getNumXPagesAtDepth(depth);
	const u32 OutputDimension = (depth != 0)?getNumXPagesAtDepth(depth-1):0;
	const u32 nXInputBlocks = ((InputDimension-1)/maxInputBlockInnerDimension) +1;
	const u32 nYInputBlocks = ((InputDimension-1)/maxInputBlockInnerDimension) +1;

			
	const u64 nBlocksForTick= nYInputBlocks*nXInputBlocks/10+1;
	u64 nCompletedBlocks = 0;
			
	SVTPage					outputBuffer(getOuterPageSize());
	std::vector<SVTPage>	inputBuffer(maxInputPages,outputBuffer);
			
	for(u32 uYBlock = 0; uYBlock < nYInputBlocks; uYBlock++)
		for(u32 uXBlock = 0; uXBlock < nXInputBlocks; uXBlock++)
		{
			nCompletedBlocks++;
			if(nCompletedBlocks % nBlocksForTick == 0)
				printf(".");

			const u32 nYInputPages = std::min((InputDimension - uYBlock*maxInputBlockInnerDimension),maxInputBlockInnerDimension) + 2;
			const u32 nXInputPages = std::min((InputDimension - uXBlock*maxInputBlockInnerDimension),maxInputBlockInnerDimension) + 2;
			const u32 nYOutputPages = (nYInputPages-2)/2;
			const u32 nXOutputPages = (nXInputPages-2)/2;
			const u32 nInputPages = nYInputPages*nXInputPages;
			const u32 nOutputPages = nYOutputPages*nXOutputPages;
					
			//read input pages
			i32 iXBegin = (i32)(uXBlock*maxInputBlockInnerDimension) - 1;
			i32 iYBegin = (i32)(uYBlock*maxInputBlockInnerDimension) - 1;

			for(i32 iXPage = iXBegin; iXPage < iXBegin+(i32)nXInputPages; ++iXPage)
				for(i32 iYPage = iYBegin; iYPage < iYBegin+(i32)nYInputPages; ++iYPage)
				{
					u32 xPage = (iXPage + InputDimension) % InputDimension;
					u32 yPage = (iYPage + InputDimension) % InputDimension;
					_pageDataStorage->RetrievePage(getPageIndex(xPage,yPage,depth), &inputBuffer[(iYPage-iYBegin)*maxInputBlockDimension + (iXPage-iXBegin)]);
				}

			//generate mipmaps
			const u32 uXOutputBegin = uXBlock*maxOutputBlockDimension;
			const u32 uYOutputBegin = uYBlock*maxOutputBlockDimension;
			const u32 uXOutputEnd = uXBlock*maxOutputBlockDimension + nXOutputPages;
			const u32 uYOutputEnd = uYBlock*maxOutputBlockDimension + nYOutputPages;

			std::array<const SVTPage*,16> rollingWindow;

			for(u32 uYPage = uYOutputBegin; uYPage < uYOutputEnd; uYPage ++)
				for(u32 uXPage = uXOutputBegin; uXPage < uXOutputEnd; uXPage ++)
				{
					for(int y = 0; y <4; y++)
						for(int x = 0; x <4; x++)
							rollingWindow[y*4+x] = &inputBuffer[(y + (uYPage - uYOutputBegin)*2) *maxInputBlockDimension + (x + (uXPage - uXOutputBegin)*2)];
					
					outputBuffer.GenerateMipmap(rollingWindow,getPageBorderSize());

					_pageDataStorage->StorePage(getPageIndex(uXPage,uYPage,depth-1),&outputBuffer);
				}
		}
				
	printf("done\n",depth);
	return true;
}

void SparseVirtualTextureGenerator::writeImageToMipLayer(const ISVTDataLayer* data,u32 data_layer,u32 mip_layer,Vector2<u32> offset)
{
	const u32 innerPageSize = getInnerPageSize();
	const u32 outerPageSize = getOuterPageSize();
	const u32 borderSize = (outerPageSize-innerPageSize)/2;
	const u32 numPages = getNumXPagesAtDepth(mip_layer);

	const Vector2<u32> size = data->GetSize();

	assert( offset.x + size.x <= innerPageSize * numPages );
	assert( offset.y + size.y <= innerPageSize * numPages );

	const u32 xBegin = ( (offset.x >= borderSize) ? offset.x - borderSize : 0 ) / innerPageSize;
	const u32 yBegin = ( (offset.y >= borderSize) ? offset.y - borderSize : 0 ) / innerPageSize;
	const u32 xEnd = std::min((offset.x + size.x + borderSize - 1)/ innerPageSize +1 , numPages);
	const u32 yEnd = std::min((offset.y + size.y + borderSize - 1)/ innerPageSize +1 , numPages);

	//generate lowest layer of the file

	SVTPage	page(outerPageSize);

	for(u32 pageX = xBegin; pageX < xEnd; ++pageX)
		for(u32 pageY = yBegin; pageY < yEnd; ++pageY)
		{
			u32 index = getPageIndex(pageX,pageY,mip_layer);

			Vector2<i32> local_offset( (i32)offset.x - (i32)pageX*innerPageSize + (i32)borderSize, (i32)offset.y - (i32)pageY*innerPageSize + (i32)borderSize);

			_pageDataStorage->RetrievePage(index,&page);

			page.WriteRect(local_offset,data_layer,data);

			_pageDataStorage->StorePage(index,&page);
		}
}

SparseVirtualTextureGenerator::SparseVirtualTextureGenerator() : _vtPacker(nullptr),_sourceDataStorage(nullptr),_sharedDataStorage(nullptr),_pageDataStorage(nullptr)
{
}

SparseVirtualTextureGenerator::~SparseVirtualTextureGenerator()
{
	if(_vtPacker)
		delete _vtPacker;
	if(_sourceDataStorage)
		delete _sourceDataStorage;
	if(_sharedDataStorage)
		delete _sharedDataStorage;
	if(_pageDataStorage)
		delete _pageDataStorage;
}

void SparseVirtualTextureGenerator::FinalizePlacement()
{
	assert(_vtPacker);
	_vtPacker->ResolvePackingLocations();
}

void SparseVirtualTextureGenerator::Save(const char* filename)
{
	SparseVirtualTextureFile file;
	file.Open(this);

	/* debug stuff */

	for(int i = 0; i< std::min((int)getNumLayers(),(int)5);++i)
	{
		u32 xSize = getInnerPageSize()*getNumXPagesAtDepth(i);
		ImageData<R8G8B8A8> buffer(xSize,xSize);

		file.ReadImageFromVirtualTexture(0,0,buffer.GetRect(),i);

		char temp[256];
		sprintf(temp,"MipMap%i.png",i);
				
		OutputImage(temp,(Color*)buffer.GetData(),xSize,xSize);
	}
	for(int i = 5; i< getNumLayers();++i)
	{
		u32 xSize = 1024;
		ImageData<R8G8B8A8> buffer(xSize,xSize);

		file.ReadImageFromVirtualTexture(0,0,buffer.GetRect(),i);

		char temp[256];
		sprintf(temp,"MipMap_partial%i.png",i);
				
		OutputImage(temp,(Color*)buffer.GetData(),xSize,xSize);
	}

	file.Save(filename);
}

SparseVirtualTextureGenerator::TextureReference SparseVirtualTextureGenerator::PlaceTexture(const ISVTDataLayer* pData,RelationArray relationArray)
{
	assert(_vtPacker);

	u32 storageIndex = _sourceDataStorage->StoreImage(pData);

	std::vector<std::pair<VTPacker::Packing,f32>> internalRelationArray;

	for(int i = 0; i < relationArray.size(); ++i)
		internalRelationArray.push_back(std::make_pair(relationArray[i].first->packing,relationArray[i].second));

	PlacedTexture placement( this,_vtPacker->AddItem(internalRelationArray,pData->GetSize().x,pData->GetSize().y),storageIndex);
	
	_textureList.push_back(placement);

	return &_textureList.back();
}

SparseVirtualTextureGenerator::TempSharedStorage::TempSharedStorage() 
{

}

SparseVirtualTextureGenerator::TempSharedStorage::~TempSharedStorage()
{
	for(auto it = dataCache.begin(); it != dataCache.end(); ++it)
	{
		assert(it->second.refCount == 0);
		delete it->second.pData;
	}
}

u32 SparseVirtualTextureGenerator::TempSharedStorage::SharedDataSize(u32 index) const
{
	return SequentialTempStorage::GetDataSize(index);
}

const void* SparseVirtualTextureGenerator::TempSharedStorage::AccessSharedData(u32 index) const
{
	auto found = dataCache.find(index);

	if(found != dataCache.end())
	{
		found->second.refCount = found->second.refCount +1;
		return found->second.pData;
	}

	if(dataCache.size() > max_items)
		garbageCollect();

	dataEntry new_entry;
	new_entry.refCount = 1;
	new_entry.pData = new u8[SharedDataSize(index)];

	SequentialTempStorage::LoadData(index,SharedDataSize(index),(u8*)new_entry.pData);

	dataCache.insert(std::make_pair(index,new_entry));

	return new_entry.pData;
}

void SparseVirtualTextureGenerator::TempSharedStorage::ReleaseSharedData(u32 index) const
{
	auto found = dataCache.find(index);

	assert(found != dataCache.end());

	--found->second.refCount;
}

u32 SparseVirtualTextureGenerator::TempSharedStorage::AllocateSharedData(u32 size,const void* pData)
{
	return SequentialTempStorage::SaveData(size,(const u8*)pData);
}

void SparseVirtualTextureGenerator::TempSharedStorage::garbageCollect() const
{
	for(auto it = dataCache.begin(); it != dataCache.end();)
	{
		if(it->second.refCount == 0)
		{
			auto last = it;
			delete last->second.pData;
			++it;
			dataCache.erase(last);
		}
		else
			++it;
	}
}

SparseVirtualTextureGenerator::TempPageStorage::TempPageStorage(u32 pageSize,u32 maxDepth,const ISVTSharedDataStorage* shared) : _pageSize(pageSize),_maxDepth(maxDepth),_shared(shared)
{
	u32 nPages = SparseVirtualTextureGenerator::getPageIndex(0,0,maxDepth);
	metrics initial;
	initial.pageHeader.flags = 0;
	initial.pageHeader.layers = 0;
	_metrics.resize(nPages,initial);
}
SparseVirtualTextureGenerator::TempPageStorage::~TempPageStorage()
{
}
void SparseVirtualTextureGenerator::TempPageStorage::StorePage(u32 pageIndex,const SVTPage* pData)
{
	u32 size = pData->Serialize(maxPageSize,_tempData.data(),&_metrics[pageIndex].pageHeader,SVTPage::PAGE_COMPRESSED_SNAPPY);

	assert(size != 0xffffffff);

	if(size)
		RandomTempStorage::SaveData(pageIndex,size,_tempData.data());
}
void SparseVirtualTextureGenerator::TempPageStorage::RetrievePage(u32 Index,SVTPage* pDataOut) const
{
	if(_metrics[Index].pageHeader.layers)
	{
		u32 size = RandomTempStorage::GetDataSize(Index);
		assert(size < maxPageSize);
		RandomTempStorage::LoadData(Index,maxPageSize,_tempData.data());
		pDataOut->Deserialize(_pageSize,&_metrics[Index].pageHeader,size,_tempData.data(),_shared);
	}
	else
		pDataOut->Empty();
}

SparseVirtualTextureGenerator::TempImageStorage::TempImageStorage(const ISVTSharedDataStorage* pShared) : _shared(pShared)
{
}

SparseVirtualTextureGenerator::TempImageStorage::~TempImageStorage()
{
}

u32 SparseVirtualTextureGenerator::TempImageStorage::StoreImage(const ISVTDataLayer* pData)
{
	
	u32 size = pData->EstimateSerializeSize();

	std::vector<u8> dataBuffer(size);

	size = pData->Serialize(size,dataBuffer.data());

	u32 newId = SequentialTempStorage::SaveData(size,dataBuffer.data());

	metrics m;
	Vector2<u32> imageSize = pData->GetSize();
	m.encoding = pData->GetEncodingId();
	m.sizeX = imageSize.x;
	m.sizeY = imageSize.y;
	_metrics.push_back(m);

	assert(_metrics.size() -1 == newId);

	return newId;
}

ISVTDataLayer* SparseVirtualTextureGenerator::TempImageStorage::RetrieveImage(u32 Index) const
{
	const metrics& m = _metrics[Index];
	u32 size = SequentialTempStorage::GetDataSize(Index);
	std::vector<u8> dataBuffer(size);
	SequentialTempStorage::LoadData(Index,size,dataBuffer.data());
	ISVTDataLayer* result = ISVTDataLayer::CreateLayer(m.encoding);
	result->Deserialize(Vector2<u32>(m.sizeX,m.sizeY),size,dataBuffer.data(),_shared);

	return result;
}


SparseVirtualTextureGenerator::RandomTempStorage::RandomTempStorage()
{
	static const char init_command[] = "CREATE TABLE ITEMS(ID INTEGER PRIMARY KEY, ITEM BLOB)";
	static const char retrieve_command[] = "SELECT ITEM FROM ITEMS WHERE ID = ?";
	static const char save_command[] = "INSERT OR REPLACE INTO ITEMS (ID,ITEM) VALUES (?,?)";
	static const char get_size_command[] = "SELECT LENGTH(ITEM) FROM ITEMS WHERE ID = ?";

	assert( sqlite3_open("",&storage) == SQLITE_OK);
	sqlite3_stmt* init;
	assert( sqlite3_prepare(storage,init_command,-1,&init,0) == SQLITE_OK);
	assert( sqlite3_step(init) == SQLITE_DONE );
	assert( sqlite3_finalize(init) == SQLITE_OK);
	
	assert( sqlite3_prepare(storage,get_size_command,-1,&get_size_stmt,0) == SQLITE_OK);
	assert( sqlite3_prepare(storage,save_command,-1,&save_stmt,0) == SQLITE_OK);
	assert( sqlite3_prepare(storage,retrieve_command,-1,&retrieve_stmt,0) == SQLITE_OK);
}

SparseVirtualTextureGenerator::RandomTempStorage::~RandomTempStorage()
{
	assert(sqlite3_finalize(retrieve_stmt) == SQLITE_OK);
	assert(sqlite3_finalize(save_stmt) == SQLITE_OK);
	assert(sqlite3_finalize(get_size_stmt) == SQLITE_OK);
	assert(sqlite3_close(storage) == SQLITE_OK);
}

void SparseVirtualTextureGenerator::RandomTempStorage::SaveData(u32 index,u32 numBytes,const u8* pData)
{
	int result;
	assert( (result = sqlite3_bind_int(save_stmt,1,index+1)) == SQLITE_OK);
	assert( (result = sqlite3_bind_blob(save_stmt,2,pData,numBytes,SQLITE_TRANSIENT)) == SQLITE_OK);
	assert( (result = sqlite3_step(save_stmt)) == SQLITE_DONE);
	assert( (result = sqlite3_reset(save_stmt)) == SQLITE_OK);
	assert( (result = sqlite3_clear_bindings(save_stmt)) == SQLITE_OK );
}

u32 SparseVirtualTextureGenerator::RandomTempStorage::GetDataSize(u32 index) const
{
	int result;
	assert( (result = sqlite3_bind_int(get_size_stmt,1,index+1)) == SQLITE_OK);
	assert( (result = sqlite3_step(get_size_stmt)) == SQLITE_ROW);
	int size = sqlite3_column_int(get_size_stmt,0);
	assert( sqlite3_reset(get_size_stmt) == SQLITE_OK);
	assert( sqlite3_clear_bindings(get_size_stmt) == SQLITE_OK );
	return size;
}

u32 SparseVirtualTextureGenerator::RandomTempStorage::LoadData(u32 index,u32 maxNumBytes,u8* pDataOut) const
{
	assert( sqlite3_bind_int(retrieve_stmt,1,index+1) == SQLITE_OK);
	assert( sqlite3_step(retrieve_stmt) == SQLITE_ROW);
	int size = sqlite3_column_bytes(retrieve_stmt,0);
	u32 result = 0;
	if(size <= maxNumBytes)
	{
		memcpy(pDataOut,sqlite3_column_blob(retrieve_stmt,0),size);
		result = size;
	}
	assert( sqlite3_reset(retrieve_stmt) == SQLITE_OK);
	assert( sqlite3_clear_bindings(retrieve_stmt) == SQLITE_OK );
	return result;
}

SparseVirtualTextureGenerator::SequentialTempStorage::SequentialTempStorage()
{
	char tempfileName[256];
	sprintf(tempfileName,"%08x.file",this);
	_tempfilename = tempfileName;
	_tempfile = fopen(_tempfilename.c_str(),"w+b");
	assert(_tempfile);
}

SparseVirtualTextureGenerator::SequentialTempStorage::~SequentialTempStorage()
{
	fclose(_tempfile);

	boost::filesystem::remove( _tempfilename );
}

u32 SparseVirtualTextureGenerator::SequentialTempStorage::SaveData(u32 numBytes,const u8* pData)
{
	Entry entry;
	_fseeki64(_tempfile,0,SEEK_END);
	entry.offset = _ftelli64(_tempfile);
	entry.uncompressed_size = numBytes;
	
	std::vector<u8> compressed(snappy::MaxCompressedLength(entry.uncompressed_size));
	
	size_t compressed_size = compressed.size();
	snappy::RawCompress((const char*)pData,entry.uncompressed_size,(char*)compressed.data(),&compressed_size);
	entry.compressed_size = compressed_size;

	u32 entryNum = _entries.size();
	_entries.push_back(entry);
	u32 written = fwrite(compressed.data(),compressed_size,1,_tempfile);
	assert(written == 1);
	fflush(_tempfile);
	return entryNum;
}

u32 SparseVirtualTextureGenerator::SequentialTempStorage::GetDataSize(u32 index) const
{
	return _entries[index].uncompressed_size;
}

u32 SparseVirtualTextureGenerator::SequentialTempStorage::LoadData(u32 index,u32 maxNumBytes,u8* pDataOut) const
{
	if( _entries[index].uncompressed_size > maxNumBytes )
		return 0;
	
	std::vector<u8> compressed(_entries[index].compressed_size);

	_fseeki64(_tempfile,_entries[index].offset,SEEK_SET);
	u32 read = fread(compressed.data(), _entries[index].compressed_size , 1 , _tempfile);

	snappy::RawUncompress((const char*)compressed.data(),_entries[index].compressed_size,(char*)pDataOut);

	assert(read == 1);
	
	fflush(_tempfile);
	return _entries[index].uncompressed_size;
}

}