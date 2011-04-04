#include "Hage.h"
#include "OpenGL3APIWrapper.h"

template<HAGE::APIWFormat _format> class _OGLPixelTransferBufferInfo
{
public:
	_OGLPixelTransferBufferInfo()
	{
		static_assert(!"Unknown format");
	}
	const static HAGE::APIWFormat	SourceFormat = 0;
	const static GLenum				DestFormat = 0;
	const static GLenum				DestChannel = 0;
	const static int					nSrcChannels = 0;
	const static int					nDestChannels = 0;
	
	typedef char DestChannelType;
	typedef char SrcChannelType;
};

template<> class _OGLPixelTransferBufferInfo<HAGE::R16_UNORM>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R16_UNORM;
	const static GLenum				DestFormat = GL_R16;
	const static GLenum				DestChannel = GL_UNSIGNED_SHORT;
	const static int				nSrcChannels = 1;
	const static int				nDestChannels = 1;
	
	typedef unsigned short DestChannelType;
	typedef unsigned short SrcChannelType;
};

template<> class _OGLPixelTransferBufferInfo<HAGE::R32_FLOAT>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R32_FLOAT;
	const static GLenum				DestFormat = GL_RED;
	const static GLenum				DestChannel = GL_FLOAT;
	const static int				nSrcChannels = 1;
	const static int				nDestChannels = 1;
	
	typedef float DestChannelType;
	typedef float SrcChannelType;
};

template<> class _OGLPixelTransferBufferInfo<HAGE::R32G32_FLOAT>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R32G32_FLOAT;
	const static GLenum				DestFormat = GL_RGB;
	const static GLenum				DestChannel = GL_FLOAT;
	const static int				nSrcChannels = 2;
	const static int				nDestChannels = 3;
	
	typedef float DestChannelType;
	typedef float SrcChannelType;
};

template<> class _OGLPixelTransferBufferInfo<HAGE::R32G32B32_FLOAT>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R32G32B32_FLOAT;
	const static GLenum				DestFormat = GL_RGB;
	const static GLenum				DestChannel = GL_FLOAT;
	const static int				nSrcChannels = 3;
	const static int				nDestChannels = 3;
	
	typedef float DestChannelType;
	typedef float SrcChannelType;
};

template<> class _OGLPixelTransferBufferInfo<HAGE::R32G32B32A32_FLOAT>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R32G32B32A32_FLOAT;
	const static GLenum				DestFormat = GL_RGBA;
	const static GLenum				DestChannel = GL_FLOAT;
	const static int				nSrcChannels = 4;
	const static int				nDestChannels = 4;
	
	typedef float DestChannelType;
	typedef float SrcChannelType;
};

template<> class _OGLPixelTransferBufferInfo<HAGE::R8G8B8A8_UNORM>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R8G8B8A8_UNORM;
	const static GLenum				DestFormat = GL_RGBA;
	const static GLenum				DestChannel = GL_UNSIGNED_BYTE;
	const static int				nSrcChannels = 4;
	const static int				nDestChannels = 4;
	
	typedef unsigned char DestChannelType;
	typedef unsigned char SrcChannelType;
};

template<> class _OGLPixelTransferBufferInfo<HAGE::R8G8B8A8_UNORM_SRGB>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R8G8B8A8_UNORM;
	const static GLenum				DestFormat = GL_RGBA;
	const static GLenum				DestChannel = GL_UNSIGNED_BYTE;
	const static int				nSrcChannels = 4;
	const static int				nDestChannels = 4;
	
	typedef unsigned char DestChannelType;
	typedef unsigned char SrcChannelType;
};

template<> class _OGLPixelTransferBufferInfo<HAGE::R8G8B8A8_SNORM>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R8G8B8A8_SNORM;
	const static GLenum				DestFormat = GL_RGBA;
	const static GLenum				DestChannel = GL_BYTE;
	const static int				nSrcChannels = 4;
	const static int				nDestChannels = 4;
	
	typedef signed char DestChannelType;
	typedef signed char SrcChannelType;
};
	template<> class _OGLPixelTransferBufferInfo<HAGE::R8G8B8A8_UINT>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R8G8B8A8_UINT;
	const static GLenum				DestFormat = GL_RGBA;
	const static GLenum				DestChannel = GL_UNSIGNED_BYTE;
	const static int				nSrcChannels = 4;
	const static int				nDestChannels = 4;
	
	typedef unsigned char DestChannelType;
	typedef unsigned char SrcChannelType;
};

template<> class _OGLPixelTransferBufferInfo<HAGE::R8G8B8A8_SINT>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R8G8B8A8_SINT;
	const static GLenum				DestFormat = GL_RGBA;
	const static GLenum				DestChannel = GL_BYTE;
	const static int				nSrcChannels = 4;
	const static int				nDestChannels = 4;
	
	typedef signed char DestChannelType;
	typedef signed char SrcChannelType;
};
/*
	UNKNOWN				= 0,
	R32_FLOAT			= 1,
	R32G32_FLOAT		= 2,
	R32G32B32_FLOAT		= 3,
	R32G32B32A32_FLOAT	= 4,
	R8G8B8A8_UNORM		= 5,
	R8G8B8A8_UNORM_SRGB	= 6,
	R8G8B8A8_SNORM		= 7,
	R8G8B8A8_UINT		= 8,
	R8G8B8A8_SINT		= 9,
	R8G8B8A8_TYPELESS	= 10,
	R16_UNORM			= 11,
	*/
class OGLPixelTransferBuffer
{
public:

	OGLPixelTransferBuffer(HAGE::APIWFormat _format,unsigned int width,unsigned int height,const void* pData)
	{
		switch(_format)
		{
		case HAGE::R16_UNORM:
			SetValues<HAGE::R16_UNORM>(width,height,pData);
			break;		
		case HAGE::R32_FLOAT:
			SetValues<HAGE::R32_FLOAT>(width,height,pData);
			break;
		case HAGE::R32G32_FLOAT:
			SetValues<HAGE::R32G32_FLOAT>(width,height,pData);
			break;
		case HAGE::R32G32B32_FLOAT:
			SetValues<HAGE::R32G32B32_FLOAT>(width,height,pData);
			break;
		case HAGE::R32G32B32A32_FLOAT:
			SetValues<HAGE::R32G32B32A32_FLOAT>(width,height,pData);
			break;
		case HAGE::R8G8B8A8_UNORM:
			SetValues<HAGE::R8G8B8A8_UNORM>(width,height,pData);
			break;
		case HAGE::R8G8B8A8_UNORM_SRGB:
			SetValues<HAGE::R8G8B8A8_UNORM_SRGB>(width,height,pData);
			break;
		case HAGE::R8G8B8A8_SNORM:
			SetValues<HAGE::R8G8B8A8_SNORM>(width,height,pData);
			break;
		case HAGE::R8G8B8A8_UINT:
			SetValues<HAGE::R8G8B8A8_UINT>(width,height,pData);
			break;
		case HAGE::R8G8B8A8_SINT:
			SetValues<HAGE::R8G8B8A8_SINT>(width,height,pData);
			break;
		case HAGE::DXTC1_UNORM:
		case HAGE::DXTC3_UNORM:
		case HAGE::DXTC5_UNORM:
			_pData = pData;
			_DestFormat = 0;
			_DestChannel = HAGE::APIWFormatImagePhysicalSize(_format,width,height);
			break;
		}
	}

	const void* _pData;
	GLenum _DestFormat;
	GLenum _DestChannel;
private:
	template<HAGE::APIWFormat _format> void SetValues(unsigned int width,unsigned int height,const void* pData)
	{
		typedef  _OGLPixelTransferBufferInfo<_format> FormatInfo;
		_DestFormat = FormatInfo::DestFormat;
		_DestChannel = FormatInfo::DestChannel;		
		if(pData == nullptr)
			_pData = pData;
		else
		{
			void* tData = new FormatInfo::DestChannelType[width*height*FormatInfo::nDestChannels];
			for(int iy=0;iy<height;iy++)
				for(int ix=0;ix<width;ix++)
					for(int ic=0;ic<FormatInfo::nDestChannels;ic++)
					{
						if(ic < FormatInfo::nSrcChannels)
							((FormatInfo::DestChannelType*)tData)[(iy*width+ix)*FormatInfo::nDestChannels + ic] = (FormatInfo::DestChannelType)(  ((FormatInfo::SrcChannelType*)pData)[((iy)*width+ix)*FormatInfo::nSrcChannels + ic] );
						else
							((FormatInfo::DestChannelType*)tData)[(iy*width+ix)*FormatInfo::nDestChannels + ic] = (FormatInfo::DestChannelType)0;
					}
			_pData=tData;
		}
	}

};

OGL3Texture::OGL3Texture(OpenGL3APIWrapper* pWrapper,HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels, HAGE::APIWFormat format,HAGE::u32 miscFlags,const void* pData)
{
	glError();
	_xSize = xSize;
	_ySize = ySize;
	_mipLevels = mipLevels;
	_format = format;
	_miscFlags = miscFlags;

	_bClearColor= false;
	_bClearDepth = false;
	_bClearStencil = false;


	glGenTextures(1,&_tbo);
	glError();
	GLenum target= GL_TEXTURE_2D;
	if(miscFlags & HAGE::TEXTURE_CUBE)
		target = GL_TEXTURE_CUBE_MAP;
	glBindTexture(target,_tbo);
	glError();

	GLenum GL_format = APIWFormatToOGLFormat(format);

	//create texture
	glError();
	int w=xSize,h=ySize;
	int offset = 0;
	for(int i = 0; i < _mipLevels; ++i)
	{
		OGLPixelTransferBuffer Buffer(format,w,h,pData?(&((HAGE::u8*)pData)[offset]):nullptr);
		int psize = HAGE::APIWFormatImagePhysicalSize(_format,w,h);
		GLenum GL_pix_format = Buffer._DestFormat;

		if(miscFlags&HAGE::TEXTURE_GPU_DEPTH_STENCIL)
		{
			switch(GL_format)
			{
			case GL_R16:
				GL_format = GL_DEPTH_COMPONENT16;
				break;
			case GL_R32F:
				GL_format = GL_DEPTH_COMPONENT32F;
				break;
			default:
				assert(!"Unsupported Depth Buffer Format");
				break;
			}
			GL_pix_format =  GL_DEPTH_COMPONENT;
		}

		if(miscFlags & HAGE::TEXTURE_CUBE)
		{
			for(int face = 0;face < 6; ++ face)
			{
				if(GL_pix_format == 0) // compressed
					glCompressedTexImage2DARB(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, i, GL_format, w, h, 0, Buffer._DestChannel, pData?(&((HAGE::u8*)Buffer._pData)[psize*face]):nullptr);
				else
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, i, GL_format, w, h, 0, GL_pix_format, Buffer._DestChannel, pData?(&((HAGE::u8*)Buffer._pData)[psize*face]):nullptr);
				glError();
			}
		}
		else
		{	
			if(GL_pix_format == 0) // compressed
				glCompressedTexImage2DARB(GL_TEXTURE_2D, i, GL_format, w, h, 0, Buffer._DestChannel, Buffer._pData);
			else
				glTexImage2D(GL_TEXTURE_2D, i, GL_format, w, h, 0, GL_pix_format,	Buffer._DestChannel, Buffer._pData);
		}
		glError();

		w>>=1;
		h>>=1;
		if(w==0)
			w=1;
		if(h==0)
			h=1;

		offset += psize;
	}
	
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, mipLevels-1);

	// set glTexParameter
	/*glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glError();
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glError();
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glError();
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glError();
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glError();
	if(miscFlags&HAGE::TEXTURE_GPU_DEPTH_STENCIL)
	{	
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		//glTexParameteri(target, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE); 
	}*/

	glBindTexture(target,0);

	glError();
}

void OGL3Texture::Clear(HAGE::Vector4<> Color)
{
	_bClearColor= true;
	_ClearColor = Color;
}

void OGL3Texture::Clear(bool bDepth,float depth,bool bStencil,HAGE::u32 stencil )
{
	if(bDepth)
	{
		_bClearDepth = true;
		_ClearDepth = depth;
	}
	if(bStencil)
	{
		_bClearStencil = true;
		_ClearStencil = stencil;
	}
}

OGL3Texture::~OGL3Texture()
{
}