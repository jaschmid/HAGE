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

template<> class _OGLPixelTransferBufferInfo<HAGE::R16_FLOAT>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R16_FLOAT;
	const static GLenum				DestFormat = GL_R16;
	const static GLenum				DestChannel = GL_UNSIGNED_SHORT;
	const static int				nSrcChannels = 1;
	const static int				nDestChannels = 1;
	
	typedef unsigned short DestChannelType;
	typedef unsigned short SrcChannelType;
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
template<> class _OGLPixelTransferBufferInfo<HAGE::R16G16B16A16_UNORM>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R16G16B16A16_UNORM;
	const static GLenum				DestFormat = GL_RGBA;
	const static GLenum				DestChannel = GL_UNSIGNED_SHORT;
	const static int				nSrcChannels = 4;
	const static int				nDestChannels = 4;
	
	typedef float DestChannelType;
	typedef float SrcChannelType;
};

template<> class _OGLPixelTransferBufferInfo<HAGE::R10G10B10A2_UNORM>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R10G10B10A2_UINT;
	const static GLenum				DestFormat = GL_UNSIGNED_INT_2_10_10_10_REV ;
	const static GLenum				DestChannel = GL_UNSIGNED_INT;
	const static int				nSrcChannels = 4;
	const static int				nDestChannels = 4;
	
	typedef float DestChannelType;
	typedef float SrcChannelType;
};
template<> class _OGLPixelTransferBufferInfo<HAGE::R10G10B10A2_UINT>
{
public:
	const static HAGE::APIWFormat	SourceFormat = HAGE::R10G10B10A2_UINT;
	const static GLenum				DestFormat = GL_UNSIGNED_INT_2_10_10_10_REV ;
	const static GLenum				DestChannel = GL_UNSIGNED_INT;
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
		case HAGE::R16_FLOAT:
			SetValues<HAGE::R16_FLOAT>(width,height,pData);
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
		case HAGE::R16G16B16A16_UNORM:
			SetValues<HAGE::R16G16B16A16_UNORM>(width,height,pData);
			break;
		case HAGE::R10G10B10A2_UNORM:
			SetValues<HAGE::R10G10B10A2_UNORM>(width,height,pData);
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

OGL3Texture::OGL3Texture(OpenGL3APIWrapper* pWrapper,HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels, HAGE::APIWFormat format,HAGE::u32 miscFlags,const void* pData,HAGE::u32 nDataSize)
{
	glError();
	_xSize = xSize;
	_ySize = ySize;
	_mipLevels = mipLevels;
	_format = format;
	_miscFlags = miscFlags;
	_streamSync=nullptr;

	_bClearColor= false;
	_bClearDepth = false;
	_bClearStencil = false;


	GLenum GL_format = APIWFormatToOGLFormat(format);

	//create texture
	if(_miscFlags & HAGE::TEXTURE_CPU_WRITE || _miscFlags & HAGE::TEXTURE_CPU_READ)
	{
		glError();
		glGenBuffers(1,&_tbo);

		assert(!(_miscFlags & HAGE::TEXTURE_GPU_WRITE || _miscFlags & HAGE::TEXTURE_GPU_DEPTH_STENCIL));
		assert(_miscFlags & HAGE::TEXTURE_GPU_NO_READ);
		glError();
	}
	else
	{
		glGenTextures(1,&_tbo);
		glError();
		GLenum target= GL_TEXTURE_2D;
		if(miscFlags & HAGE::TEXTURE_CUBE)
			target = GL_TEXTURE_CUBE_MAP;
		glBindTexture(target,_tbo);
		glError();

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
}

void OGL3Texture::Clear(HAGE::Vector4<> Color)
{
	assert(!(_miscFlags & HAGE::TEXTURE_CPU_WRITE || _miscFlags & HAGE::TEXTURE_CPU_READ));

	_bClearColor= true;
	_ClearColor = Color;
}

void OGL3Texture::Clear(bool bDepth,float depth,bool bStencil,HAGE::u32 stencil )
{
	assert(!(_miscFlags & HAGE::TEXTURE_CPU_WRITE || _miscFlags & HAGE::TEXTURE_CPU_READ));

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
	if(_streamSync)
	{
		glDeleteSync(_streamSync);
		_streamSync = nullptr;
	}

	
	if(! (_miscFlags & HAGE::TEXTURE_CPU_WRITE || _miscFlags & HAGE::TEXTURE_CPU_READ) )
	{
		//free texture
		glDeleteTextures(1,&_tbo);
	}
	else
	{
		//free buffer
		glDeleteBuffers(1,&_tbo);
	}
}

void OGL3Texture::GenerateMips()
{

	GLenum target= GL_TEXTURE_2D;
	if(_miscFlags & HAGE::TEXTURE_CUBE)
		target = GL_TEXTURE_CUBE_MAP;
	glBindTexture(target,_tbo);

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(target,_tbo);
}

/*
void OGL3Texture::StreamToTexture(HAGE::u32 xOff,HAGE::u32 yOff,HAGE::u32 xSize,HAGE::u32 ySize,HAGE::APIWTexture* pBuffer) const
{	
	if(! (_miscFlags & HAGE::TEXTURE_CPU_WRITE || _miscFlags & HAGE::TEXTURE_CPU_READ) )
	{
		glError();
		assert(!"Not supported yet");

		// copy texture to pbo	
		assert(_miscFlags & HAGE::TEXTURE_GPU_COPY); 
		OGL3Texture* target = (OGL3Texture*)pBuffer;

		assert(target->_miscFlags & HAGE::TEXTURE_CPU_READ); 

		//settings have to match
		assert(target->_xSize == _xSize);
		assert(target->_ySize == _ySize);
		assert(target->_mipLevels == _mipLevels);
		assert(target->_format == _format);


		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, target->_tbo);
		glReadPixels(0, 0, _xSize, _ySize, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

		if(_streamSync)
		{
			glDeleteSync(_streamSync);
			_streamSync = nullptr;
		}
		if(target->_streamSync)
		{
			glDeleteSync(target->_streamSync);
			target->_streamSync = nullptr;
		}

		_streamSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
		target->_streamSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
		glError();
	}
	else
	{
		glError();
		//copy pbo to texture
		glError();
		OGL3Texture* target = (OGL3Texture*)pBuffer;

		assert(target->_miscFlags & HAGE::TEXTURE_GPU_COPY); 

		assert(_miscFlags & HAGE::TEXTURE_CPU_WRITE); 

		//settings have to match
		assert(target->_xSize == _xSize);
		assert(target->_ySize == _ySize);
		assert(target->_mipLevels == _mipLevels);
		assert(target->_format == _format);

		glBindTexture(GL_TEXTURE_2D, target->_tbo);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _tbo);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, xSize, ySize,
                GL_BGRA, GL_UNSIGNED_BYTE, 0);

		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glError();

		
		if(_streamSync)
		{
			glDeleteSync(_streamSync);
			_streamSync = nullptr;
		}
		if(target->_streamSync)
		{
			glDeleteSync(target->_streamSync);
			target->_streamSync = nullptr;
		}

		_streamSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
		target->_streamSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
		glError();
	}

}

bool OGL3Texture::IsStreamComplete()
{
	assert(_streamSync);
	GLenum result =glClientWaitSync(_streamSync,GL_SYNC_FLUSH_COMMANDS_BIT,0);

	if(result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
		return true;
	else
		return false;
}

void OGL3Texture::WaitForStream()
{
	assert(_streamSync);
	GLenum result = glClientWaitSync(_streamSync,GL_SYNC_FLUSH_COMMANDS_BIT,1000000000);
	assert(result != GL_TIMEOUT_EXPIRED);
}

HAGE::u32 OGL3Texture::ReadTexture(const HAGE::u8** ppBufferOut) const
{
	assert(_miscFlags & HAGE::TEXTURE_CPU_READ); 
	return 0;
}

HAGE::u32 OGL3Texture::LockTexture(HAGE::u8** ppBufferOut,HAGE::u32 flags)
{	
	assert(_miscFlags & HAGE::TEXTURE_CPU_WRITE); 
	return 0;
}

void OGL3Texture::UnlockTexture()
{
	assert(_miscFlags & HAGE::TEXTURE_CPU_WRITE); 
}*/

void OGL3Texture::StreamForReading(HAGE::u32 xOff,HAGE::u32 yOff,HAGE::u32 xSize,HAGE::u32 ySize)
{
}
void OpenGL3APIWrapper::UpdateTexture(HAGE::APIWTexture* pTexture,HAGE::u32 xOff,HAGE::u32 yOff,HAGE::u32 xSize,HAGE::u32 ySize,HAGE::u32 Level, const void* pData)
{
}