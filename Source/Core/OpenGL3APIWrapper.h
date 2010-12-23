#ifndef OPENGL3__API__WRAPPER
#define OPENGL3__API__WRAPPER

#ifdef DEBUG_GL
#include "SDK/Hage.h"
#else
#include <HAGE.h>
#endif

#include "RenderDebugUI.h"

#ifndef NO_OGL

#define N_CBUFFERS				16
#define MAX_VERTEX_FORMATS		((HAGE::u8)256)
#define MAX_RASTERIZER_STATES	((HAGE::u16)4096)
#define MAX_BLEND_STATES		((HAGE::u16)4096)

#include <boost/intrusive/list.hpp>
#include <boost/functional/hash.hpp>
#include "FixedSizeKeyStorage.h"
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
#include <boost/functional/hash.hpp>

#if _DEBUG
#define glError() { \
	GLenum err = glGetError(); \
	if (err != GL_NO_ERROR) { \
		printf("glError: %08x caught at %s:%u\n", err, __FILE__, __LINE__); \
		err = glGetError(); \
	} \
}
#else
#define glError() 
#endif

class OGL3ConstantBuffer;

struct BlendStateEX
{
	bool					bAlphaToCoverage;
	HAGE::u32				nBlendStates;
	HAGE::APIWBlendState	BlendStates[8];
};

inline bool operator ==(const BlendStateEX& _l,const BlendStateEX& _r)
{
	return memcmp(&_l,&_r,sizeof(BlendStateEX))==0;
}

namespace std {

template<> class hash<HAGE::APIWRasterizerState>
{
    public:
        size_t operator ()(const HAGE::APIWRasterizerState & state) const
        {
             std::size_t seed = 0;
             boost::hash_combine(seed,state.bDepthClipEnable);
             boost::hash_combine(seed,state.bMultisampleEnable);
             boost::hash_combine(seed,state.bScissorEnable);
             boost::hash_combine(seed,state.bWireframe);
             boost::hash_combine(seed,state.CullMode);
             boost::hash_combine(seed,state.fDepthBiasClamp);
             boost::hash_combine(seed,state.fSlopeScaledDepthBias);
             boost::hash_combine(seed,state.iDepthBias);
             return seed;
        }
};

template<> class hash<BlendStateEX>
{
    public:
        size_t operator ()(const BlendStateEX & state) const
        {
             std::size_t seed = 0;
             boost::hash_combine(seed,state.bAlphaToCoverage);
             boost::hash_combine(seed,state.nBlendStates);
             for(HAGE::u32 i =0;i<state.nBlendStates;++i)
             {
                 boost::hash_combine(seed,state.BlendStates[i].bBlendEnable);
                 if(state.BlendStates[i].bBlendEnable)
                 {
                    boost::hash_combine(seed,state.BlendStates[i].BlendOp);
                    boost::hash_combine(seed,state.BlendStates[i].BlendOpAlpha);
                    boost::hash_combine(seed,state.BlendStates[i].bWriteA);
                    boost::hash_combine(seed,state.BlendStates[i].bWriteB);
                    boost::hash_combine(seed,state.BlendStates[i].bWriteG);
                    boost::hash_combine(seed,state.BlendStates[i].bWriteR);
                    boost::hash_combine(seed,state.BlendStates[i].DestBlend);
                    boost::hash_combine(seed,state.BlendStates[i].DestBlendAlpha);
                    boost::hash_combine(seed,state.BlendStates[i].SrcBlend);
                    boost::hash_combine(seed,state.BlendStates[i].SrcBlendAlpha);
                 }
             }
             return seed;
        }
};

}

class OpenGL3APIWrapper : public HAGE::RenderingAPIWrapper
{
public:
	OpenGL3APIWrapper();
	~OpenGL3APIWrapper();

	void BeginFrame();
	void PresentFrame();
	void BeginAllocation();
	void EndAllocation();

	void RegisterVertexFormat(const char* szName,const HAGE::VertexDescriptionEntry* pDescription,HAGE::u32 nNumEntries);
	HAGE::APIWVertexArray* CreateVertexArray(
		HAGE::u32 nPrimitives,
		HAGE::APIWPrimitiveType PrimitiveType,
		HAGE::APIWVertexBuffer** pBuffers,
		HAGE::u32 nBuffers,
		const HAGE::u32* pIndexBufferData);
	HAGE::APIWVertexBuffer* CreateVertexBuffer(const char* szVertexFormat,const void* pData,HAGE::u32 nElements,bool bDynamic, bool bInstanceData);
	HAGE::APIWConstantBuffer* CreateConstantBuffer(HAGE::u32 nSize);
	virtual HAGE::APIWEffect* CreateEffect(const char* pVertexProgram,const char* pFragmentProgram,
		const HAGE::APIWRasterizerState* pRasterizerState, const HAGE::APIWBlendState* pBlendState,
		const HAGE::u32 nBlendStates, bool AlphaToCoverage);

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
	void checkForCgError(const char *situation);

	HAGE::u16					GetRasterizerStateCode(const HAGE::APIWRasterizerState* pState);
	void						SetRasterizerState(HAGE::u16 code);

	HAGE::u16					GetBlendStateCode(const HAGE::APIWBlendState* pState,HAGE::u32 nBlendStates,bool bAlphaToCoverage);
	void						SetBlendState(HAGE::u16 code);

private:
	typedef std::unordered_map<std::string,HAGE::u8> VertexStringTableType;
	VertexStringTableType		m_VertexStringTable;
	typedef std::array<VertexFormatEntry,256> VertexFormatListType;
	VertexFormatListType		m_VertexFormatList;
	HAGE::u32					m_NextVertexFormatEntry;

	FixedSizeKeyStorage<HAGE::u16,MAX_RASTERIZER_STATES,HAGE::APIWRasterizerState>
								m_RasterizerStates;
	HAGE::u16					m_CurrentRS;

	FixedSizeKeyStorage<HAGE::u16,MAX_BLEND_STATES,BlendStateEX>
								m_BlendStates;
	HAGE::u16					m_CurrentBS;
	boost::mutex 				m_mutexAllocation;

#ifdef TARGET_WINDOWS
	HINSTANCE                   m_hInst;
	HWND                        m_hWnd;
	HDC							m_hDC;
	HGLRC						m_hrc;                 // OpenGL Rendering Context
	HGLRC						m_hrcL;                // OpenGL Rendering Context for Loading
	HGLRC						m_backupC;
#elif defined(TARGET_LINUX)
    GLXContext                  m_hrc;
	GLXContext                  m_hrcL;
	GLXContext                  m_backupC;
    Window*                     m_pWindow;
    Display*                    m_pDisplay;
	static boost::thread_specific_ptr<GLXContext> _currentRC;
#endif
	CGcontext					myCgContext;
	CGprofile					myCgVertexProfile, myCgFragmentProfile;

	HAGE::RenderDebugUI*		m_DebugUIRenderer;

	friend class D3D11Effect;
};

class OGL3VertexBuffer : public HAGE::APIWVertexBuffer
{
public:
	OGL3VertexBuffer(OpenGL3APIWrapper* pWrapper,const char* szVertexFormat,const void* pData,HAGE::u32 nElements,bool bDynamic, bool bInstanceData);
	void UpdateContent(const void* pData);
	~OGL3VertexBuffer();

private:
	unsigned int				m_vboID;		// VBO
	HAGE::u32					m_BufferSize;
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

	void Init();

private:
	unsigned int				m_vaoID;
	unsigned int				m_vboIndexID;
	OGL3VertexBuffer**			m_pBuffers;
	HAGE::u32					m_nBuffers;
	HAGE::u32					m_nPrimitives;
	HAGE::APIWPrimitiveType		m_PrimitiveType;
	bool						_bInit;
	OpenGL3APIWrapper*			m_pWrapper;
	friend class OGL3Effect;
};

class OGL3ConstantBuffer : public HAGE::APIWConstantBuffer
{
public:
	OGL3ConstantBuffer(OpenGL3APIWrapper* pWrapper,HAGE::u32 nSize);
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
	OGL3Effect(OpenGL3APIWrapper* pWrapper,const char* pVertexProgram,const char* pFragmentProgram,HAGE::u16 rasterizer,HAGE::u16 blend);
	~OGL3Effect();

	virtual void Draw(HAGE::APIWVertexArray* pArray,HAGE::APIWConstantBuffer* const * pConstants,HAGE::u32 nConstants = 1);
private:
	CGprogram					m_CgVertexProgram;
	static CGprogram			testProgram;
	CGprogram					m_CgFragmentProgram;

	OpenGL3APIWrapper*			m_pWrapper;
	HAGE::u16					m_RastState;
	HAGE::u16					m_BlendState;
	char*						_cVertexShader;
	char*						_cPixelShader;
	GLuint						_glVShader,_glFShader,_glProgram;
	bool						_bInit;
};

#endif

#endif
