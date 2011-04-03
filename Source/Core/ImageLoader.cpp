#include "HAGE.h"
#define FREEIMAGE_LIB
#include <FreeImage.h>

#include <stdio.h>

#include "CImageLoader.h"

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

IResource* CTextureImageLoader::Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
{
	if(nDependanciesInOut == 0)
	{
		*pDependanciesOut = _pDependancies;
		nDependanciesInOut = 1;
		return nullptr;
	}
	else
	{
		assert(nDependanciesInOut == 1);
		return (IResource*)new CTextureImage(TResourceAccess<IImageData>(dependanciesIn[0]));
	}
}

CTextureImageLoader::CTextureImage::CTextureImage(const TResourceAccess<IImageData>& pData) : _pTexture(nullptr)
{
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
	case IImageData::DXTC1:
		uFormat = HAGE::APIWFormat::DXTC1_UNORM;
		break;
	case IImageData::DXTC3:
		uFormat = HAGE::APIWFormat::DXTC3_UNORM;
		break;
	case IImageData::DXTC5:
		uFormat = HAGE::APIWFormat::DXTC5_UNORM;
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

	_pTexture=pAlloc->CreateTexture(pData->GetImageWidth(),pData->GetImageHeight(),pData->GetImageLevels(),uFormat,0,pDataSource);
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
}
CImageDataLoader::~CImageDataLoader()
{
}

IResource* CImageDataLoader::Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
{
	return (IResource*)_pImageData;
}

CImageDataLoader::CImageData::CImageData(IDataStream* pData)
{
	if(pData->IsValid())
	{
		static bool init = false;
		if(!init)
		{
			init = true;
			
			TLS_data backup = *TLS::getData();
			TLS::getData()->domain_ptr = nullptr;
			TLS::getData()->domain_guid = guidNull;
			FreeImage_Initialise();
			*TLS::getData() = backup;
		}

		TLS_data backup = *TLS::getData();
		TLS::getData()->domain_ptr = nullptr;
		TLS::getData()->domain_guid = guidNull;
		FIBITMAP* fibitmap = FreeImage_LoadFromHandle(FIF_PNG,&ImageLoaderIO,(fi_handle)pData);
		*TLS::getData() = backup;

		if(fibitmap)
		{
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
			
			TLS_data backup = *TLS::getData();
			TLS::getData()->domain_ptr = nullptr;
			TLS::getData()->domain_guid = guidNull;
	
			FreeImage_Unload(fibitmap);

			*TLS::getData() = backup;

			_Levels = 1;
			return;
		}
		pData->Seek(0,IDataStream::ORIGIN_BEGINNING);
		if(TryLoadBLP(pData))
			return;
		
		pData->Seek(0,IDataStream::ORIGIN_BEGINNING);
		
		SparseVirtualTextureFile hsvt;
		if(hsvt.Open(pData))
		{
			_Width = 4096;
			_Height = 4096;
			_Format = R8G8B8A8;
			_Levels = 1;
			_Data = new u8[_Width*_Height*sizeof(u32)];

			hsvt.ReadImageFromVirtualTexture(0,0,4096,4096,_Data,6);

			return;
		}
		
		pData->Seek(0,IDataStream::ORIGIN_BEGINNING);


	}

	// loading failed
	_Width = 256;
	_Height = 256;
	_Format = R8G8B8A8;
	_Levels = 1;
	_Data = new u8[_Width*_Height*sizeof(u32)];
	for(int iy =0; iy < _Height; ++iy)
		for(int ix =0; ix < _Width; ++ix)
			((u32*)_Data)[iy*_Width+ix] = ((1-((iy/64)+(ix/64))%2) * 0x00ffffff) | 0xff000000;
}

inline static void forceRead(IDataStream* pData,u64 size,void* pOut)
{
	assert(pData->Read(size,(u8*)pOut) == size);
}

bool CImageDataLoader::CImageData::TryLoadBLP(IDataStream* pData)
{
	static const char blpMagic[4] = {'B','L','P','2'};
	u8 Magic[4];
	if(pData->Read(4,Magic)!=4)
		return false;
	if(memcmp(blpMagic,Magic,4) !=0)
		return false;
	// it's a blp file hooray!

	u32 offsets[16],sizes[16], type=0;

	char attr[4];

	forceRead(pData,4,&type);
	forceRead(pData,4,attr);
	forceRead(pData,4,&_Width);
	forceRead(pData,4,&_Height);
	forceRead(pData,4*16,offsets);
	forceRead(pData,4*16,sizes);

	bool hasmipmaps = (attr[3]>0);

	int totalsize = 0;

	if(hasmipmaps)
		for(int i = 0;i<16;++i)
		{
			if(offsets[i] && sizes[i])
				totalsize+=sizes[i];
			else
			{
				_Levels = i-1;
				break;
			}
		}
	else
		_Levels = 1;

	if (type != 1) {
		printf("Error: %s:%s#%d type=%d", pData->GetIdentifierString().c_str());
		return false;
	}

	if (attr[0] == 2) 
	{
		// compressed		
		_Data = new unsigned char[totalsize];

		_Format = IImageData::DXTC1;
		i32 blocksize = 8;

		// guesswork here :(
		if (attr[1]==8 || attr[1]==4) {
			_Format = IImageData::DXTC3;
			blocksize = 16;
		}

		if (attr[1]==8 && attr[2]==7) {
			_Format = IImageData::DXTC5;
			blocksize = 16;
		}

		i32 write_loc = 0;

		int w= _Width,h=_Height;

		// do every mipmap level
		for (int i=0; i<_Levels; i++) {
			if (w==0) w = 1;
			if (h==0) h = 1;
			
			pData->Seek(offsets[i],IDataStream::ORIGIN_BEGINNING);
			forceRead(pData,sizes[i],&((u8*)_Data)[write_loc]);

			int size = ((w+3)/4) * ((h+3)/4) * blocksize;

			write_loc += sizes[i];
				
			w >>= 1;
			h >>= 1;
		}
	}
	else if (attr[0]==1) 
	{
		// uncompressed

		_Format = IImageData::R8G8B8A8;

		u32 pal[256];
		forceRead(pData,1024, pal);

		_Data = new unsigned char[totalsize*4];

		i32 write_loc = 0;

		u8 *buf = new unsigned char[sizes[0]];
		u32 *p = NULL;
		u8 *c = NULL, *a = NULL;

		u32 alphabits = attr[1];
		bool hasalpha = (alphabits!=0);

		int w= _Width,h=_Height;

		for (int i=0; i<_Levels; i++) {
			if (w==0) w = 1;
			if (h==0) h = 1;

			pData->Seek(offsets[i],IDataStream::ORIGIN_BEGINNING);
			forceRead(pData,sizes[i],buf);

			int cnt = 0;
			int alpha = 0;

			p = (u32*)&((u8*)_Data)[write_loc];
			c = buf;
			a = buf + w*h;
			for (int y=0; y<h; y++) 
			{
				for (int x=0; x<w; x++) 
				{
					unsigned int k = pal[*c++];
					k = ((k&0x00FF0000)>>16) | ((k&0x0000FF00)) | ((k& 0x000000FF)<<16);

					if (hasalpha) 
					{
						if (alphabits == 8) 
						{
							alpha = (*a++);
						} 
						else if (alphabits == 4) 
						{
							alpha = (*a & (0xf << cnt++)) * 0x11;
							if (cnt == 2) 
							{
								cnt = 0;
								a++;
							}
						} 
						else if (alphabits == 1) 
						{
							alpha = (*a & (1 << cnt++)) ? 0xff : 0;
							if (cnt == 8) 
							{
								cnt = 0;
								a++;
							}
						}
					} 
					else alpha = 0xff;

					k |= alpha << 24;
					*p++ = k;
				}
			}

			write_loc += sizes[i]*4;

			w >>= 1;
			h >>= 1;
		}

		delete[] buf;
	}

	return true;
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

u32 CImageDataLoader::CImageData::GetImageLevels() const
{
	return _Levels;
}

}