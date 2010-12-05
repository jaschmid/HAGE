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
	R32G32B32A32_FLOAT	= 4
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

class RenderingAPIAllocator
{
public:
	virtual void RegisterVertexFormat(const char* szName,const VertexDescriptionEntry* pDescription,u32 nNumEntries) = 0;
	virtual APIWVertexArray* CreateVertexArray(HAGE::u32 nPrimitives,
		HAGE::APIWPrimitiveType PrimitiveType,
		HAGE::APIWVertexBuffer** pBuffers,
		HAGE::u32 nBuffers = 1,
		const HAGE::u32* pIndexBufferData = nullptr) = 0;
	virtual APIWVertexBuffer* CreateVertexBuffer(const char* szVertexFormat,const void* pData,u32 nElements,bool bInstanceData = false) = 0;
	virtual APIWConstantBuffer* CreateConstantBuffer(u32 nSize) = 0;
	virtual APIWEffect* CreateEffect(const char* pVertexProgram,const char* pFragmentProgram,
		const APIWRasterizerState* pRasterizerState = &DefaultRasterizerState, const APIWBlendState* pBlendState = &DefaultBlendState,
		const u32 nBlendStates = 1, bool AlphaToCoverage = false) = 0;

	virtual void BeginAllocation() = 0;
	virtual void EndAllocation() = 0;

	static RenderingAPIAllocator* QueryAPIAllocator(){return _pAllocator;}
protected:
	static RenderingAPIAllocator* _pAllocator;
};

class RenderingAPIWrapper : public RenderingAPIAllocator
{
public:
#ifndef NO_D3D
#ifdef TARGET_WINDOWS
	static RenderingAPIWrapper* CreateD3D11Wrapper();
#endif
#endif
#ifndef NO_OGL
	static RenderingAPIWrapper* CreateOpenGL3Wrapper();
#endif

	virtual ~RenderingAPIWrapper(){};

	virtual void BeginFrame() = 0;
	virtual void PresentFrame() = 0;


};

class APIWVertexBuffer
{
public:
	virtual ~APIWVertexBuffer(){}
};

class APIWVertexArray
{
public:
	virtual ~APIWVertexArray(){}
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
	virtual void Draw(HAGE::APIWVertexArray* pArray,HAGE::APIWConstantBuffer* const * pConstants,HAGE::u32 nConstants = 1)=0;
};

struct VertexDescriptionEntry
{
	const char*				pName;
	u32						nSubElements;
	APIWFormat				fFormat;
};

}

#endif