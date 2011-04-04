/********************************************************/
/* FILE: RenderingAPIWrapper.h                          */
/* DESCRIPTION: Defines the Rendering API Wrapper class.*/
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef RENDERING__API__WRAPPER
#define RENDERING__API__WRAPPER

#include "HAGE.h"

namespace HAGE {

class APIWVertexBuffer;
class APIWEffect;
class APIWConstantBuffer;
struct VertexDescriptionEntry;
class APIWVertexArray;
class APIWTexture;

typedef enum _APIWPrimitiveType
{
	PRIMITIVE_POINTLIST	= 0,
	PRIMITIVE_LINELIST,
	PRIMITIVE_LINESTRIP,
	PRIMITIVE_TRIANGLELIST,
	PRIMITIVE_TRIANGLESTRIP
} APIWPrimitiveType;

typedef enum _APIWFormat
{
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
	DXTC1_UNORM			= 12,
	DXTC1_UNORM_SRGB	= 13,
	DXTC3_UNORM			= 14,
	DXTC3_UNORM_SRGB	= 15,
	DXTC5_UNORM			= 16,
	DXTC5_UNORM_SRGB	= 17
} APIWFormat;

typedef enum _APIWCullMode
{
	CULL_NONE			= 0,
	CULL_CCW			= 1,
	CULL_CW				= 2
} APIWCullMode;

typedef struct _APIWRasterizerState
{
	APIWCullMode		CullMode;
	bool				bWireframe;
	i32					iDepthBias;
	f32					fDepthBiasClamp;
	f32					fSlopeScaledDepthBias;
	bool				bDepthClipEnable;
	bool				bScissorEnable;
	bool				bMultisampleEnable;
} APIWRasterizerState;

inline bool operator ==(const HAGE::APIWRasterizerState& _l,const HAGE::APIWRasterizerState& _r)
{
	return memcmp(&_l,&_r,sizeof(HAGE::APIWRasterizerState))==0;
}

typedef enum _APIWBlendMode
{
	BLEND_ZERO				= 1,
	BLEND_ONE				= 2,
	BLEND_SRC_COLOR			= 3,
	BLEND_INV_SRC_COLOR		= 4,
	BLEND_SRC_ALPHA			= 5,
	BLEND_INV_SRC_ALPHA		= 6,
	BLEND_DEST_ALPHA		= 7,
	BLEND_INV_DEST_ALPHA	= 8,
	BLEND_DEST_COLOR		= 9,
	BLEND_INV_DEST_COLOR	= 10,
	BLEND_SRC_ALPHA_SAT		= 11,
	BLEND_BLEND_FACTOR		= 12,
	BLEND_INV_BLEND_FACTOR	= 13,
	BLEND_SRC1_COLOR		= 14,
	BLEND_INV_SRC_1_COLOR	= 15,
	BLEND_SRC1_ALPHA		= 16,
	BLEND_INV_SRC1_ALPHA	= 17
} APIWBlendMode;

typedef enum _APIWBlendOp
{
	BLEND_OP_ADD			= 1,
	BLEND_OP_SUBTRACT		= 2,
	BLEND_OP_REV_SUBTRACT	= 3,
	BLEND_OP_MIN			= 4,
	BLEND_OP_MAX			= 5
} APIWBlendOp;

typedef enum _APIWTextureFlags
{
	TEXTURE_CUBE				= 0x00000001,
	TEXTURE_CPU_WRITE			= 0x00000002,
	TEXTURE_CPU_READ			= 0x00000004,
	TEXTURE_GPU_WRITE			= 0x00000008,
	TEXTURE_GPU_NO_READ			= 0x00000010,
	TEXTURE_GPU_DEPTH_STENCIL	= 0x00000020
} APIWTextureFlags;
typedef enum _APIWFilterFlags
{
	FILTER_MIN_POINT			= 0x00000000,
	FILTER_MIN_LINEAR			= 0x00000001,
	FILTER_MAG_POINT			= 0x00000000,
	FILTER_MAG_LINEAR			= 0x00000002,
	FILTER_MIP_POINT			= 0x00000000,
	FILTER_MIP_LINEAR			= 0x00000004,
	FILTER_COMPARISON			= 0x00000008,
	FILTER_ANISOTROPIC			= 0x00000040
} APIWFilterFlags;
typedef enum _APIWComparison
{
	COMPARISON_NEVER			= 0x000000001,
	COMPARISON_LESS				= 0x000000002,
	COMPARISON_EQUAL			= 0x000000003,
	COMPARISON_LESS_EQUAL		= 0x000000004,
	COMPARISON_GREATER			= 0x000000005,
	COMPARISON_NOT_EQUAL		= 0x000000006,
	COMPARISON_GREATER_EQUAL	= 0x000000007,
	COMPARISON_ALWAYS			= 0x000000008
} APIWComparison;
typedef enum _APIWAddressModes
{
	ADDRESS_WRAP			= 0x00000001,
	ADDRESS_MIRROR			= 0x00000002,
	ADDRESS_CLAMP			= 0x00000003,
	ADDRESS_BORDER			= 0x00000004,
	ADDRESS_MIRROR_ONCE		= 0x00000005,
} APIWAddressModes;

typedef struct _APIWBlendState
{
	bool				bBlendEnable;
	APIWBlendMode		SrcBlend;
	APIWBlendMode		DestBlend;
	APIWBlendOp			BlendOp;
	APIWBlendMode		SrcBlendAlpha;
	APIWBlendMode		DestBlendAlpha;
	APIWBlendOp			BlendOpAlpha;
	bool				bWriteR,bWriteG,bWriteB,bWriteA;
} APIWBlendState;

typedef struct _APIWSamplerState
{
	u32					FilterFlags;
	APIWComparison		ComparisonFunction;
	APIWAddressModes	AddressModeU;
	APIWAddressModes	AddressModeV;
	APIWAddressModes	AddressModeW;
	float				BorderColor[4];
	f32					MaxAnisotropy;
	f32					MipLODBias;
	f32					MipLODMin;
	f32					MipLODMax;
} APIWSamplerState;

typedef struct _APIWSampler
{
	char				SamplerName[32];
	APIWSamplerState	State;
} APIWSampler;

extern const APIWBlendState DefaultBlendState;
extern const APIWRasterizerState DefaultRasterizerState;
extern const APIWSamplerState DefaultSamplerState;

typedef struct _APIWDisplaySettings
{
	bool	bFullscreen;
	u32		xRes;
	u32		yRes;
} APIWDisplaySettings;

typedef enum _APIWRendererType
{
	APIW_D3DWRAPPER,
	APIW_OGLWRAPPER,
	APIW_DEFAULT
} APIWRendererType;


static HAGE::u32 APIWFormatPixelSize(const HAGE::APIWFormat& format)
{
	switch(format)
	{
	case HAGE::R16_UNORM			:
		return sizeof(HAGE::u16)*2;
	case HAGE::R32_FLOAT			:
		return sizeof(HAGE::f32);
	case HAGE::R32G32_FLOAT			:
		return sizeof(HAGE::f32)*2;
	case HAGE::R32G32B32_FLOAT		:
		return sizeof(HAGE::f32)*3;
	case HAGE::R32G32B32A32_FLOAT	:
		return sizeof(HAGE::f32)*4;
	case HAGE::R8G8B8A8_UNORM		:
	case HAGE::R8G8B8A8_UNORM_SRGB	:
	case HAGE::R8G8B8A8_SNORM		:
	case HAGE::R8G8B8A8_UINT		:
	case HAGE::R8G8B8A8_SINT		:
	case HAGE::R8G8B8A8_TYPELESS	:
		return sizeof(HAGE::u8)*4;
	case HAGE::DXTC1_UNORM:
	case HAGE::DXTC1_UNORM_SRGB:
	case HAGE::DXTC3_UNORM:
	case HAGE::DXTC3_UNORM_SRGB:
	case HAGE::DXTC5_UNORM:
	case HAGE::DXTC5_UNORM_SRGB:
		assert(!"Compressed Texture does not have pixel Size!");
		return 0;
	default:
		assert(!"Unknown Format!");
		return 0;
	}
}

static const u32 DXTC1_BLOCKSIZE = 8;
static const u32 DXTC3_BLOCKSIZE = 16;
static const u32 DXTC5_BLOCKSIZE = 16;

static HAGE::u32 APIWFormatImagePhysicalSize(const HAGE::APIWFormat& format,u32 virtualWidth,u32 virtualHeight)
{
	switch(format)
	{
	case HAGE::R16_UNORM			:
	case HAGE::R32_FLOAT			:
	case HAGE::R32G32_FLOAT			:
	case HAGE::R32G32B32_FLOAT		:
	case HAGE::R32G32B32A32_FLOAT	:
	case HAGE::R8G8B8A8_UNORM		:
	case HAGE::R8G8B8A8_UNORM_SRGB	:
	case HAGE::R8G8B8A8_SNORM		:
	case HAGE::R8G8B8A8_UINT		:
	case HAGE::R8G8B8A8_SINT		:
	case HAGE::R8G8B8A8_TYPELESS	:
		return APIWFormatPixelSize(format)*virtualWidth*virtualHeight;
	case HAGE::DXTC1_UNORM:
	case HAGE::DXTC1_UNORM_SRGB:
		return (virtualWidth/4+((virtualWidth%4)?1:0))*(virtualHeight/4+((virtualHeight%4)?1:0))*DXTC1_BLOCKSIZE;
	case HAGE::DXTC3_UNORM:
	case HAGE::DXTC3_UNORM_SRGB:
		return (virtualWidth/4+((virtualWidth%4)?1:0))*(virtualHeight/4+((virtualHeight%4)?1:0))*DXTC3_BLOCKSIZE;
	case HAGE::DXTC5_UNORM:
	case HAGE::DXTC5_UNORM_SRGB:
		return (virtualWidth/4+((virtualWidth%4)?1:0))*(virtualHeight/4+((virtualHeight%4)?1:0))*DXTC5_BLOCKSIZE;
	default:
		assert(!"Unknown Format!");
		return 0;
	}
}

static HAGE::u32 APIWFormatImagePhysicalPitch(const HAGE::APIWFormat& format,u32 virtualWidth)
{
	switch(format)
	{
	case HAGE::R16_UNORM			:
	case HAGE::R32_FLOAT			:
	case HAGE::R32G32_FLOAT			:
	case HAGE::R32G32B32_FLOAT		:
	case HAGE::R32G32B32A32_FLOAT	:
	case HAGE::R8G8B8A8_UNORM		:
	case HAGE::R8G8B8A8_UNORM_SRGB	:
	case HAGE::R8G8B8A8_SNORM		:
	case HAGE::R8G8B8A8_UINT		:
	case HAGE::R8G8B8A8_SINT		:
	case HAGE::R8G8B8A8_TYPELESS	:
		return APIWFormatPixelSize(format)*virtualWidth;
	case HAGE::DXTC1_UNORM:
	case HAGE::DXTC1_UNORM_SRGB:
		return (virtualWidth/4+((virtualWidth%4)?1:0))*DXTC1_BLOCKSIZE;
	case HAGE::DXTC3_UNORM:
	case HAGE::DXTC3_UNORM_SRGB:
		return (virtualWidth/4+((virtualWidth%4)?1:0))*DXTC3_BLOCKSIZE;
	case HAGE::DXTC5_UNORM:
	case HAGE::DXTC5_UNORM_SRGB:
		return (virtualWidth/4+((virtualWidth%4)?1:0))*DXTC5_BLOCKSIZE;
	default:
		assert(!"Unknown Format!");
		return 0;
	}
}

typedef struct _APIWViewport
{
	f32 XMin,XSize;
	f32 YMin,YSize;
	f32 ZMin,ZMax;
} APIWViewport;

extern const APIWViewport NullViewport;

extern APIWTexture * const RENDER_TARGET_DEFAULT;
extern APIWTexture * const RENDER_TARGET_NONE;

class RenderingAPIAllocator
{
public:
	virtual void RegisterVertexFormat(const char* szName,const VertexDescriptionEntry* pDescription,u32 nNumEntries) = 0;
	virtual APIWVertexArray* CreateVertexArray(HAGE::u32 nPrimitives,
		HAGE::APIWPrimitiveType PrimitiveType,
		HAGE::APIWVertexBuffer** pBuffers,
		HAGE::u32 nBuffers = 1,
		const HAGE::u32* pIndexBufferData = nullptr) = 0;
	virtual APIWVertexBuffer* CreateVertexBuffer(const char* szVertexFormat,const void* pData,u32 nElements,bool bDynamic = false, bool bInstanceData = false) = 0;
	virtual APIWConstantBuffer* CreateConstantBuffer(u32 nSize) = 0;
	virtual APIWEffect* CreateEffect(const char* pProgram,
		const APIWRasterizerState* pRasterizerState = &DefaultRasterizerState, 
		const APIWBlendState* pBlendState = &DefaultBlendState,const u32 nBlendStates = 1, bool AlphaToCoverage = false,
		const APIWSampler* pSamplers= nullptr,u32 nSamplers = 0) = 0;
	virtual APIWTexture* CreateTexture(u32 xSize, u32 ySize, u32 mipLevels, APIWFormat format,u32 miscFlags,const void* pData,u32 nDataSize) = 0;
	//virtual APIWSampler* CreateSampler(APIWSamplerState* pSamplerState) = 0;

	virtual void BeginAllocation() = 0;
	virtual void EndAllocation() = 0;

	virtual HAGE::Matrix4<> GenerateProjectionMatrix(f32 near,f32 far,f32 fovX,f32 fovY)	= 0;
	virtual	HAGE::Matrix4<> GenerateRenderTargetProjection(f32 near,f32 far,f32 fovX,f32 fovY) = 0;
	virtual HAGE::Matrix4<> GenerateProjectionMatrixBase()	= 0;
	virtual	HAGE::Matrix4<> GenerateRenderTargetProjectionBase() = 0;

	static RenderingAPIAllocator* QueryAPIAllocator(){return _pAllocator;}
protected:
	static RenderingAPIAllocator* _pAllocator;
};

class RenderingAPIWrapper : public RenderingAPIAllocator
{
public:
	static RenderingAPIWrapper* CreateRenderingWrapper(APIWRendererType type,const APIWDisplaySettings* pSettings);

	virtual ~RenderingAPIWrapper(){};

	virtual void BeginFrame() = 0;
	virtual void PresentFrame() = 0;

	virtual void SetRenderTarget(HAGE::APIWTexture* pTextureRenderTarget,HAGE::APIWTexture* pTextureDepthStencil,const HAGE::APIWViewport& viewport = NullViewport) = 0;
	virtual void GetCurrentViewport(HAGE::APIWViewport& vpOut) = 0;
	virtual void UpdateDisplaySettings(const APIWDisplaySettings* pSettings) = 0;
};

class APIWVertexBuffer
{
public:
	virtual ~APIWVertexBuffer(){}
	virtual void UpdateContent(const void* pData)=0;
};

class APIWVertexArray
{
public:
	virtual ~APIWVertexArray(){}
};

class APIWTexture
{
public:
	virtual void Clear(Vector4<> Color) = 0;
	virtual void Clear(bool bDepth,float depth,bool bStencil = false,u32 stencil = 0) = 0;
	virtual void GenerateMips() = 0;
	virtual ~APIWTexture(){}
};

class APIWConstantBuffer
{
public:
	virtual ~APIWConstantBuffer(){}
	virtual void UpdateContent(const void* pData)=0;
};

class APIWEffect
{
public:
	virtual ~APIWEffect(){}
	virtual void SetConstant(const char* pName,const APIWConstantBuffer* constant)=0;
	virtual void SetTexture(const char* pName,const APIWTexture* texture)=0;
	virtual void Draw(HAGE::APIWVertexArray* pArray)=0;
};

struct VertexDescriptionEntry
{
	const char*				pName;
	u32						nSubElements;
	APIWFormat				fFormat;
};

}

#endif