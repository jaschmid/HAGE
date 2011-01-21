#include "HAGE.h"
#define FREEIMAGE_LIB
#include <FreeImage.h>

#include <stdio.h>

namespace HAGE
{

// FreeImage Data Access

unsigned DLL_CALLCONV FI_ReadIDataStream (void *buffer, unsigned size, unsigned count, fi_handle handle)
{
	IDataStream* pStream = (IDataStream*)handle;
	return pStream->Read((u64)size*(u64)count,(u8*)buffer);
}

unsigned DLL_CALLCONV FI_WriteIDataStream (void *buffer, unsigned size, unsigned count, fi_handle handle)
{
	IDataStream* pStream = (IDataStream*)handle;
	assert(!"Writing to Data Stream not supported");
	return 0;
}

int DLL_CALLCONV FI_SeekIDataStream (fi_handle handle, long offset, int origin)
{
	IDataStream* pStream = (IDataStream*)handle;
	IDataStream::ORIGIN seek_origin;
	switch(origin)
	{
	case SEEK_CUR:
		seek_origin = IDataStream::ORIGIN_CURRENT;
		break;
	case SEEK_END:
		seek_origin = IDataStream::ORIGIN_END;
		break;
	case SEEK_SET:
		seek_origin = IDataStream::ORIGIN_BEGINNING;
		break;
	}
	return pStream->Seek(offset,seek_origin);
}

long DLL_CALLCONV FI_TellIDataStream (fi_handle handle)
{
	IDataStream* pStream = (IDataStream*)handle;
	return pStream->Seek(0,IDataStream::ORIGIN_CURRENT);
}

static FreeImageIO ImageLoaderIO = { &FI_ReadIDataStream, &FI_WriteIDataStream, &FI_SeekIDataStream, &FI_TellIDataStream };

IResourceLoader* CTextureImageLoader::Initialize(IDataStream* pStream,const IResource* pPrev)
{
	return new CTextureImageLoader(pStream,pPrev);
}

CTextureImageLoader::CTextureImageLoader(IDataStream* pStream,const IResource* pPrev)
{
	_pDependancies = new std::pair<std::string,guid>[1];
	_pDependancies[0] = std::pair<std::string,guid>(pStream->GetIdentifierString(),guid_of<IImageData>::Get());
}
CTextureImageLoader::~CTextureImageLoader()
{
	if(_pDependancies)
		delete [] _pDependancies;
}

u32 CTextureImageLoader::GetDependancies(const std::pair<std::string,guid>** pDependanciesOut)
{
	*pDependanciesOut = _pDependancies;
	return 1;
}

IResource* CTextureImageLoader::Finalize(const IResource** dependanciesIn,u32 nDependanciesIn)
{
	assert(nDependanciesIn == 1);
	return (IResource*)new CTextureImage((const IImageData*)dependanciesIn[0]);
}

CTextureImageLoader::CTextureImage::CTextureImage(const IImageData* pData) : _pTexture(nullptr)
{
	assert(pData);
	RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
	assert(pAlloc);
	pAlloc->BeginAllocation();

	APIWFormat uFormat;
	bool temp_buffer = false;

	switch(pData->GetImageFormat())
	{
	case IImageData::R8G8B8:
		temp_buffer= true;
		uFormat = HAGE::APIWFormat::R8G8B8A8_UNORM;
		break;
	case IImageData::R8G8B8A8:
		uFormat = HAGE::APIWFormat::R8G8B8A8_UNORM;
		break;
	}

	const void* pDataSource = pData->GetImageData();

	if(temp_buffer)
	{
		u32 out_size = 4;
		u32 in_size = 3;
		u32 width = pData->GetImageWidth();
		u32 height = pData->GetImageHeight();
		void* pTemp = (void*)new u8[out_size*width*height];
		for(int iy =0; iy < height; ++iy)
			for(int ix =0; ix < width; ++ix)
			{
				int id;
				for(id = 0; id<in_size;id++)
					((u8*)pTemp)[id+(iy*width+ix)*out_size] = ((u8*)pDataSource)[id+(iy*width+ix)*in_size];
				for(; id<out_size;id++)
					((u8*)pTemp)[id+(iy*width+ix)*out_size] = 255;
			}
		pDataSource = pTemp;
	}

	_pTexture=pAlloc->CreateTexture(pData->GetImageWidth(),pData->GetImageHeight(),1,uFormat,0,pDataSource);
	assert(_pTexture);

	if(temp_buffer)
		delete pDataSource;

	pAlloc->EndAllocation();
}

CTextureImageLoader::CTextureImage::~CTextureImage()
{
	if(_pTexture)delete _pTexture;
}

const APIWTexture* CTextureImageLoader::CTextureImage::GetTexture() const
{
	return _pTexture;
}

IResourceLoader* CImageDataLoader::Initialize(IDataStream* pStream,const IResource* pPrev)
{
	return new CImageDataLoader(pStream,pPrev);
}

CImageDataLoader::CImageDataLoader(IDataStream* pStream,const IResource* pPrev)
{
	_pImageData = new CImageData(pStream);
	pStream->Close();
}
CImageDataLoader::~CImageDataLoader()
{
}

u32 CImageDataLoader::GetDependancies(const std::pair<std::string,guid>** pDependanciesOut)
{
	return 0;
}

IResource* CImageDataLoader::Finalize(const IResource** dependanciesIn,u32 nDependanciesIn)
{
	return (IResource*)_pImageData;
}

CImageDataLoader::CImageData::CImageData(const IDataStream* pData)
{
	if(pData->GetIdentifierString() == std::string("Null"))
	{
		_Width = 256;
		_Height = 256;
		_Format = R8G8B8A8;
		_Data = new u8[_Width*_Height*sizeof(u32)];
		for(int iy =0; iy < _Height; ++iy)
			for(int ix =0; ix < _Width; ++ix)
				((u32*)_Data)[iy*_Width+ix] = ((1-((iy/64)+(ix/64))%2) * 0x00ffffff) | 0xff000000;
	}
	else
	{
		static bool init = false;
		if(!init)
		{
			init = true;
			FreeImage_Initialise();
		}
		FIBITMAP* fibitmap = FreeImage_LoadFromHandle(FIF_PNG,&ImageLoaderIO,(fi_handle)pData);
		_Width = FreeImage_GetWidth(fibitmap);
		_Height = FreeImage_GetHeight(fibitmap);
		_Format = R8G8B8A8;
		_Data = new u8[_Width*_Height*sizeof(u32)];
		for(int iy =0; iy < _Height; ++iy)
			for(int ix =0; ix < _Width; ++ix)
			{
				RGBQUAD color;
				FreeImage_GetPixelColor(fibitmap,ix,iy,&color);
				((u32*)_Data)[iy*_Width+ix] = ((u32)color.rgbRed << 0) | ((u32)color.rgbGreen << 8) | ((u32)color.rgbBlue << 16) | ((u32)0xff << 24);
			}
		FreeImage_Unload(fibitmap);
	}
}

CImageDataLoader::CImageData::~CImageData()
{
	
}

const void* CImageDataLoader::CImageData::GetImageData() const
{
	return _Data;
}

u32 CImageDataLoader::CImageData::GetImageFormat() const
{
	return _Format;
}

u32 CImageDataLoader::CImageData::GetImageHeight() const
{
	return _Height;
}

u32 CImageDataLoader::CImageData::GetImageWidth() const
{
	return _Width;
}

}