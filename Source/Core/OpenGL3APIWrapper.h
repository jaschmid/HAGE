#ifndef OPENGL3__API__WRAPPER
#define OPENGL3__API__WRAPPER

#include <HAGE.h>

#ifndef NO_OGL

#define N_CBUFFERS 16

#include <boost/intrusive/list.hpp>
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <array>
#include <algorithm>

#ifdef TARGET_WINDOWS
#include <Windows.h>
#elif defined(TARGET_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include <GL/glew.h>

#ifdef TARGET_WINDOWS
#include <GL/wglew.h>
#elif defined(TARGET_LINUX)
#include <GL/glxew.h>
#endif

#include <Cg/cg.h>    /* Can't include this?  Is Cg Toolkit installed! */
#include <Cg/cgGL.h>

class OGL3ConstantBuffer;

class OpenGL3APIWrapper : public HAGE::RenderingAPIWrapper
{
public:
	OpenGL3APIWrapper();
	~OpenGL3APIWrapper();

	void BeginFrame();
	void PresentFrame();

	void RegisterVertexFormat(const char* szName,HAGE::VertexDescriptionEntry* pDescription,HAGE::u32 nNumEntries);
	HAGE::APIWVertexArray* CreateVertexArray(
		HAGE::u32 nPrimitives,
		HAGE::APIWPrimitiveType PrimitiveType,
		HAGE::APIWVertexBuffer** pBuffers,
		HAGE::u32 nBuffers,
		const HAGE::u32* pIndexBufferData);
	HAGE::APIWVertexBuffer* CreateVertexBuffer(const char* szVertexFormat,void* pData,HAGE::u32 nElements,bool bInstanceData);
	HAGE::APIWConstantBuffer* CreateConstantBuffer(HAGE::u32 nSize);
	HAGE::APIWEffect* CreateEffect(const char* pVertexProgram,const char* pFragmentProgram);

#ifdef TARGET_WINDOWS
	HGLRC GetRC(){return m_hrc;}
#endif

	CGcontext& GetCGC(){return myCgContext;}
	CGprofile& GetVertexProfile(){return myCgVertexProfile;}
	CGprofile& GetFragmentProfile(){return myCgFragmentProfile;}

	struct VertexFormatEntry
	{
		HAGE::u32						uVertexSize;
		HAGE::VertexDescriptionEntry*	pOriginalDescription;
		HAGE::u32						nElements;
	};

	HAGE::u8					GetVertexFormatCode(const char* name);
	HAGE::u32					GetVertexSize(HAGE::u8 code);
	const VertexFormatEntry*	GetVertexFormat(HAGE::u8 code);
	void						SetCBuffer(HAGE::u8 i,OGL3ConstantBuffer* p)
	{
		m_cBuffers[i]=p;
	}
	OGL3ConstantBuffer*			GetCBuffer(HAGE::u8 i){return m_cBuffers[i];}
	void checkForCgError(const char *situation);

private:
	typedef std::unordered_map<std::string,HAGE::u8> VertexStringTableType;
	VertexStringTableType		m_VertexStringTable;
	typedef std::array<VertexFormatEntry,256> VertexFormatListType;
	VertexFormatListType		m_VertexFormatList;

	std::array<OGL3ConstantBuffer*,N_CBUFFERS> m_cBuffers;

#ifdef TARGET_WINDOWS
	HINSTANCE                   m_hInst;
	HWND                        m_hWnd;
	HDC							m_hDC;
	HGLRC						m_hrc;                 // OpenGL Rendering Context
#elif defined(TARGET_LINUX)
    GLXContext                  m_hrc;
    Window*                     m_pWindow;
    Display*                    m_pDisplay;
#endif
	HAGE::u32					m_NextVertexFormatEntry;
	CGcontext   myCgContext;
	CGprofile   myCgVertexProfile, myCgFragmentProfile;

	friend class D3D11Effect;
};

class OGL3VertexBuffer : public HAGE::APIWVertexBuffer
{
public:
	OGL3VertexBuffer(OpenGL3APIWrapper* pWrapper,const char* szVertexFormat,void* pData,HAGE::u32 nElements,bool bInstanceData);
	~OGL3VertexBuffer();

private:
	unsigned int				m_vboID;		// VBO
	HAGE::u8					m_code;
	OpenGL3APIWrapper*			m_pWrapper;
	friend class OGL3Effect;
	friend class OGL3VertexArray;
};

class OGL3VertexArray : public HAGE::APIWVertexArray
{
public:
	OGL3VertexArray(OpenGL3APIWrapper* pWrapper,HAGE::u32 nPrimitives,HAGE::APIWPrimitiveType PrimitiveType,HAGE::APIWVertexBuffer** pBuffers,HAGE::u32 nBuffers, const HAGE::u32* pIndexBufferData);
	~OGL3VertexArray();

private:
	unsigned int				m_vaoID;
	unsigned int				m_vboIndexID;
	OGL3VertexBuffer**			m_pBuffers;
	HAGE::u32					m_nBuffers;
	HAGE::u32					m_nPrimitives;
	HAGE::APIWPrimitiveType		m_PrimitiveType;
	OpenGL3APIWrapper*			m_pWrapper;
	friend class OGL3Effect;
};

class OGL3ConstantBuffer : public HAGE::APIWConstantBuffer
{
public:
	OGL3ConstantBuffer(OpenGL3APIWrapper* pWrapper,HAGE::u32 nSize);
	virtual void Set(HAGE::u32 nBuffer);
	virtual void UpdateContent(const void* pData);
	~OGL3ConstantBuffer();
private:
	OpenGL3APIWrapper*			m_pWrapper;
	HAGE::u32					m_nSize;
	unsigned int				m_cbo;
	CGbuffer					m_Buffer;
	friend class OGL3Effect;
};

class OGL3Effect : public HAGE::APIWEffect
{
public:
	OGL3Effect(OpenGL3APIWrapper* pWrapper,const char* pVertexProgram,const char* pFragmentProgram);
	~OGL3Effect();

	virtual void Draw(HAGE::APIWVertexArray* pArray);
private:
	CGprogram					m_CgVertexProgram;
	CGprogram					m_CgFragmentProgram;

	OpenGL3APIWrapper*			m_pWrapper;
};

#endif

#endif
