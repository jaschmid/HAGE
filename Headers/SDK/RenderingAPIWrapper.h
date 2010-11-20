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
	UNKNOWN			= 0,
	R32G32B32_FLOAT = 1
} APIWFormat;

class RenderingAPIWrapper
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

	virtual void RegisterVertexFormat(const char* szName,VertexDescriptionEntry* pDescription,u32 nNumEntries) = 0;
	virtual APIWVertexArray* CreateVertexArray(HAGE::u32 nPrimitives,
		HAGE::APIWPrimitiveType PrimitiveType,
		HAGE::APIWVertexBuffer** pBuffers,
		HAGE::u32 nBuffers,
		const HAGE::u32* pIndexBufferData) = 0;
	virtual APIWVertexBuffer* CreateVertexBuffer(const char* szVertexFormat,void* pData,u32 nElements,bool bInstanceData = false) = 0;
	virtual APIWConstantBuffer* CreateConstantBuffer(u32 nSize) = 0;
	virtual APIWEffect* CreateEffect(const char* pVertexProgram,const char* pFragmentProgram) = 0;
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
	virtual void Set(u32 nBuffer)=0;
	virtual void UpdateContent(const void* pData)=0;
};

class APIWEffect
{
public:
	virtual ~APIWEffect(){}
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