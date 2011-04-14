#include <HAGE.h>

namespace HAGE {

	// create a new sparse virtual texture file, virtualsize is in # of pages per dimension (pixel size = ((2<<(pagedepth-1)) * virtualsize) ^2 )
	SparseVirtualTextureFile::SparseVirtualTextureFile() :
		bOpen(false),
		tempFile(nullptr),
		dataStream(nullptr)
	{
	}
			
	SparseVirtualTextureFile::~SparseVirtualTextureFile()
	{
		if(tempFile)
			fclose(tempFile);
	}

	bool SparseVirtualTextureFile::New(char* FileName, u32 pagesize,u32 pagedepth, u32 bordersize)
	{

		fileHeader.fourCC[0] = 'H';
		fileHeader.fourCC[1] = 'S';
		fileHeader.fourCC[2] = 'V';
		fileHeader.fourCC[3] = 'T';
		fileHeader.bordersize = bordersize;
		fileHeader.pagedepth = pagedepth;
		fileHeader.majorVersion = 1;
		fileHeader.minorVersion = 0;
		fileHeader.pagesize = pagesize;
		u64 nPages = GetTotalNumPagesUntilDepth(fileHeader.pagedepth);
		pageHeaders.resize(nPages);
		for(int i = 0; i<nPages; ++i)
		{
			pageHeaders[i].file_offset = 0;
			pageHeaders[i].page_flags = 0;
		}

		tempFile = fopen(FileName,"w+b");
		if(!tempFile)
		{
			bOpen = false;
			return false;
		}

		fwrite(&fileHeader,sizeof(FileHeader),1,tempFile);
		fwrite(pageHeaders.data(),sizeof(PageHeader),pageHeaders.size(),tempFile);
		fflush(tempFile);
		bOpen = true;
		return true;
	}

	void SparseVirtualTextureFile::Commit()
	{
		if(!tempFile)
			return;

		_fseeki64(tempFile,0,SEEK_SET);
		fwrite(&fileHeader,sizeof(FileHeader),1,tempFile);
		fwrite(pageHeaders.data(),sizeof(PageHeader),pageHeaders.size(),tempFile);
		fflush(tempFile);
	}

	bool SparseVirtualTextureFile::Open(char* FileName)
	{
		bOpen = false;
		tempFile = fopen(FileName,"r+b");
		if(!tempFile)
			return false;

		size_t nRead = fread(&fileHeader,sizeof(FileHeader),1,tempFile);
		if(!nRead)
		{
			fclose(tempFile);
			return false;
		}

		if( fileHeader.fourCC[0] != 'H' ||
			fileHeader.fourCC[1] != 'S' ||
			fileHeader.fourCC[2] != 'V' ||
			fileHeader.fourCC[3] != 'T')
		{
			fclose(tempFile);
			return false;
		}

			
		u64 nPages = GetTotalNumPagesUntilDepth(fileHeader.pagedepth);
		pageHeaders.resize(nPages);
		nRead = fread(pageHeaders.data(),sizeof(PageHeader),pageHeaders.size(),tempFile);

		if(nRead != nPages)
		{
			fclose(tempFile);
			return false;
		}

		fflush(tempFile);
		bOpen = true;
		return true;
	}
		
	bool SparseVirtualTextureFile::Open(IDataStream* pDataStream)
	{
		bOpen = false;
		dataStream = pDataStream;
		if(!dataStream)
			return false;

		size_t nRead = dataStream->Read(sizeof(FileHeader),(u8*)&fileHeader);
		if(nRead != sizeof(FileHeader))
		{
			return false;
		}

		if( fileHeader.fourCC[0] != 'H' ||
			fileHeader.fourCC[1] != 'S' ||
			fileHeader.fourCC[2] != 'V' ||
			fileHeader.fourCC[3] != 'T')
		{
			return false;
		}

			
		u64 nPages = GetTotalNumPagesUntilDepth(fileHeader.pagedepth);
		pageHeaders.resize(nPages);
		nRead = dataStream->Read(sizeof(PageHeader)*pageHeaders.size(),(u8*)pageHeaders.data());

		if(nRead != sizeof(PageHeader)*pageHeaders.size())
		{
			return false;
		}

		bOpen = true;
		return true;
	}
		
	void SparseVirtualTextureFile::WritePage(u64 PageIndex,const void* pData, u32 flags)
	{
		if(!tempFile)
			return;

		if(!bLastWrite)
		{
			bLastWrite = true;
			fflush(tempFile);
		}

		pageHeaders[PageIndex].page_flags = flags;

		u32 bytes,padding;

		if(flags& PAGE_FINAL)
		{
			bytes = GetPageOuterPixelCount()*sizeof(u32);
			padding = 0;
		}
		else
		{
			bytes = sizeof(u32)*GetPageInnerPixelCount();
			padding = GetPageOuterPixelCount()*sizeof(u32) - bytes;
		}

		if(pageHeaders[PageIndex].file_offset != 0)
		{
			//already in tempfile overwrite
			_fseeki64(tempFile,pageHeaders[PageIndex].file_offset,SEEK_SET);
			pageHeaders[PageIndex].page_size = bytes;
			size_t written = fwrite(pData,bytes,1,tempFile);
			assert(written == 1);
			if(padding)
			{
				written = fwrite(pData,padding,1,tempFile);
				assert(written == 1);
			}
		}
		else
		{
			_fseeki64(tempFile,1,SEEK_END);
			pageHeaders[PageIndex].file_offset = _ftelli64(tempFile);
			pageHeaders[PageIndex].page_size = bytes;
			assert(pageHeaders[PageIndex].file_offset!= 0);
			size_t written = fwrite(pData,bytes,1,tempFile);
			assert(written == 1);
			if(padding)
			{
				written = fwrite(pData,padding,1,tempFile);
				assert(written == 1);
			}
		}
	}

	bool SparseVirtualTextureFile::ReadPage(u64 PageIndex,void* pDataOut,u32 flags)
	{
		if(!bOpen)
			return false;

		if(bLastWrite)
		{
			bLastWrite = false;
			fflush(tempFile);
		}

		u32 bytes,padding;
		void* pWriteTarget = pDataOut;

		if(pageHeaders[PageIndex].page_flags & PAGE_FINAL)
		{
			bytes = GetPageOuterPixelCount()*sizeof(u32);
			padding = 0;

			if(!(flags & PAGE_FINAL))
				pWriteTarget = new u32[bytes/sizeof(u32)];
			else
				pWriteTarget = pDataOut;

		}
		else
		{
			bytes = sizeof(u32)*GetPageInnerPixelCount();
			padding = GetPageOuterPixelCount()*sizeof(u32) - bytes;

			if(flags & PAGE_FINAL)
				pWriteTarget = new u32[bytes/sizeof(u32)];
			else
				pWriteTarget = pDataOut;
		}

		bool bResult = true;

		if(pageHeaders[PageIndex].file_offset != 0)
		{
			if(tempFile)
			{
				_fseeki64(tempFile,pageHeaders[PageIndex].file_offset,SEEK_SET);
				size_t read=fread(pWriteTarget,1,bytes,tempFile);
				assert(read == (size_t)bytes);
				bResult = true;
			}
			else if(dataStream)
			{
				dataStream->Seek(pageHeaders[PageIndex].file_offset,IDataStream::ORIGIN_BEGINNING);
				u64 read = dataStream->Read(bytes,(u8*)pWriteTarget);
				assert(read == bytes);
				bResult = true;
			}
			else
				assert(!"Neither tempFile nor dataStream open");
		}
		else
		{
			memset(pWriteTarget,0,bytes);
			bResult = false;
		}

		if(pWriteTarget != pDataOut)
		{
				
			if(!(flags & PAGE_FINAL))
				stripBorder((const u32*)pWriteTarget,(u32*)pDataOut);
			else if(flags & PAGE_FINAL)
				blackBorder((const u32*)pWriteTarget,(u32*)pDataOut);
			else
				assert(!"Huh?");

			delete pWriteTarget;
		}

		return bResult;
	}

	
		
	void SparseVirtualTextureFile::WriteImageToVirtualTexture(u64 vtXOffset, u64 vtYOffset,u64 ImageX,u64 ImageY,const void* DataIn,int level)
	{
		const u32* pData = (u32*) DataIn;
		const u64 vtPageSize = GetPageInnerSize();
		const u64 vtXPageBegin = vtXOffset / vtPageSize;
		const u64 vtYPageBegin = vtYOffset / vtPageSize;
		const u64 vtXPageEnd = (vtXOffset + ImageX -1)/ vtPageSize + 1;
		const u64 vtYPageEnd = (vtYOffset + ImageY -1)/ vtPageSize + 1;
		const u64 vtXPixelBegin = vtXOffset % vtPageSize;
		const u64 vtYPixelBegin = vtYOffset % vtPageSize;
		const u64 vtXPixelEnd = (vtXOffset+ImageX -1) % vtPageSize +1;
		const u64 vtYPixelEnd = (vtYOffset+ImageY -1) % vtPageSize +1;
		if(level == -1)
			level = GetMaxDepth()-1;

		std::vector<u32> tempBuffer;
		tempBuffer.resize(vtPageSize*vtPageSize);

		for(u64 iYPage = vtYPageBegin; iYPage < vtYPageEnd ; ++ iYPage)
			for(u64 iXPage = vtXPageBegin; iXPage < vtXPageEnd ; ++ iXPage)
			{
				const u64 pXPixelBegin = (iXPage==vtXPageBegin)?(vtXPixelBegin):0;
				const u64 pYPixelBegin = (iYPage==vtYPageBegin)?(vtYPixelBegin):0;
				const u64 pXPixelEnd = (iXPage==vtXPageEnd-1)?(vtXPixelEnd):vtPageSize;
				const u64 pYPixelEnd = (iYPage==vtYPageEnd-1)?(vtYPixelEnd):vtPageSize;
				const u64 ImageXOffset = iXPage*vtPageSize - vtXOffset;
				const u64 ImageYOffset = iYPage*vtPageSize - vtYOffset;
				const u64 nPage = GetPageIndex(iXPage,iYPage,level);

				if( (pXPixelEnd-pXPixelBegin)*(pYPixelEnd-pYPixelBegin) < vtPageSize*vtPageSize )
					ReadPage(nPage,tempBuffer.data());

				for(u64 iYPixel = pYPixelBegin; iYPixel < pYPixelEnd; ++iYPixel)
					for(u64 iXPixel = pXPixelBegin; iXPixel < pXPixelEnd; ++iXPixel)
						tempBuffer[iYPixel*vtPageSize + iXPixel] =
							pData[(ImageYOffset+iYPixel)*ImageX + (ImageXOffset+iXPixel)];

				WritePage(nPage,tempBuffer.data());
			}
	}

	void SparseVirtualTextureFile::ReadImageFromVirtualTexture(u64 vtXOffset, u64 vtYOffset,u64 ImageX,u64 ImageY,void* pDataOut,int level)
	{
		u32* pData = (u32*) pDataOut;
		const u64 vtPageSize = GetPageInnerSize();
		const u64 vtXPageBegin = vtXOffset / vtPageSize;
		const u64 vtYPageBegin = vtYOffset / vtPageSize;
		const u64 vtXPageEnd = (vtXOffset + ImageX -1)/ vtPageSize + 1;
		const u64 vtYPageEnd = (vtYOffset + ImageY -1)/ vtPageSize + 1;
		const u64 vtXPixelBegin = vtXOffset % vtPageSize;
		const u64 vtYPixelBegin = vtYOffset % vtPageSize;
		const u64 vtXPixelEnd = (vtXOffset+ImageX - 1) % vtPageSize + 1;
		const u64 vtYPixelEnd = (vtYOffset+ImageY - 1) % vtPageSize + 1;
		if(level == -1)
			level = GetMaxDepth()-1;

		std::vector<u32> tempBuffer;
		tempBuffer.resize(vtPageSize*vtPageSize);

		for(u64 iYPage = vtYPageBegin; iYPage < vtYPageEnd ; ++ iYPage)
			for(u64 iXPage = vtXPageBegin; iXPage < vtXPageEnd ; ++ iXPage)
			{
				const u64 pXPixelBegin = (iXPage==vtXPageBegin)?(vtXPixelBegin):0;
				const u64 pYPixelBegin = (iYPage==vtYPageBegin)?(vtYPixelBegin):0;
				const u64 pXPixelEnd = (iXPage==vtXPageEnd-1)?(vtXPixelEnd):vtPageSize;
				const u64 pYPixelEnd = (iYPage==vtYPageEnd-1)?(vtYPixelEnd):vtPageSize;
				const u64 ImageXOffset = iXPage*vtPageSize - vtXOffset;
				const u64 ImageYOffset = iYPage*vtPageSize - vtYOffset;
				const u64 nPage = GetPageIndex(iXPage,iYPage,level);

				ReadPage(nPage,tempBuffer.data());

				for(u64 iYPixel = pYPixelBegin; iYPixel < pYPixelEnd; ++iYPixel)
					for(u64 iXPixel = pXPixelBegin; iXPixel < pXPixelEnd; ++iXPixel)
							pData[(ImageYOffset+iYPixel)*ImageX + (ImageXOffset+iXPixel)] = 
								tempBuffer[iYPixel*vtPageSize + iXPixel];
			}
	}
		
	void SparseVirtualTextureFile::stripBorder(const u32* pIn,u32* pOut) const
	{
		u32 sizeIn = GetPageOuterSize();
		u32 sizeOut = GetPageInnerSize();
		u32 borderSize = (sizeIn-sizeOut)/2;

		for(u32 iy = 0; iy < sizeOut; iy++)
			for(u32 ix = 0; ix < sizeOut; ix++)
				pOut[iy*sizeOut+ix] = pIn[(iy+borderSize)*sizeIn+ix+borderSize];
	}

	void SparseVirtualTextureFile::blackBorder(const u32* pIn,u32* pOut) const
	{
		u32 sizeIn = GetPageInnerSize();
		u32 sizeOut = GetPageOuterSize();
		u32 borderSize = (sizeOut-sizeIn)/2;

		u32 Black = 0;

		for(u32 iy = 0; iy < borderSize; iy++)
			for(u32 ix = 0; ix < sizeOut; ix++)
				pOut[iy*sizeOut+ix] = Black;
			
		for(u32 iy = borderSize; iy < sizeOut-borderSize; iy++)
		{
			for(u32 ix = 0; ix < borderSize; ix++)
				pOut[iy*sizeOut+ix] = Black;
				
			for(u32 ix = borderSize; ix < sizeOut-borderSize; ix++)
				pOut[iy*sizeOut+ix] = pIn[(iy-borderSize)*sizeIn+ix-borderSize];

			for(u32 ix = sizeOut-borderSize; ix < sizeOut; ix++)
				pOut[iy*sizeOut+ix] = Black;
		}
			
		for(u32 iy = sizeOut-borderSize; iy < sizeOut; iy++)
			for(u32 ix = 0; ix < sizeOut; ix++)
				pOut[iy*sizeOut+ix] = Black;
	}		


}