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

extern const APIWBlendState DefaultBlendState;
extern const APIWRasterizerState DefaultRasterizerState;

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
		const APIWRasterizerState* pRasterizerState = &DefaultRasterizerState, const APIWBlendState* pBlendState = &DefaultBlendState,
		const u32 nBlendStates = 1, bool AlphaToCoverage = false) = 0;
	virtual APIWTexture* CreateTexture(u32 xSize, u32 ySize, u32 mipLevels, APIWFormat format,u32 miscFlags,const void* pData) = 0;

	virtual void BeginAllocation() = 0;
	virtual void EndAllocation() = 0;

	virtual HAGE::Matrix4<> GenerateProjectionMatrix(f32 near,f32 far,f32 fovX,f32 fovY)	= 0;
	virtual	HAGE::Matrix4<> GenerateRenderTargetProjection(f32 near,f32 far,f32 fovX,f32 fovY) = 0;

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

	virtual void SetRenderTarget(HAGE::APIWTexture* pTextureRenderTarget,HAGE::APIWTexture* pTextureDepthStencil) = 0;
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