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

static GLenum APIWFormatToOGLFormat(const HAGE::APIWFormat& format)
{
	switch(format)
	{
	case HAGE::R16_UNORM			:
		return GL_R16;
		break;
	case HAGE::R32_FLOAT			:
		return GL_R32F;
		break;
	case HAGE::R32G32_FLOAT			:
		return GL_RG32F;
		break;
	case HAGE::R32G32B32_FLOAT		:
		return GL_RGB32F;
		break;
	case HAGE::R32G32B32A32_FLOAT	:
		return GL_RGBA32F;
		break;
	case HAGE::R8G8B8A8_UNORM		:
		return GL_RGBA8;
		break;
	case HAGE::R8G8B8A8_UNORM_SRGB	:
		return GL_SRGB8_ALPHA8;
		break;
	case HAGE::R8G8B8A8_SNORM		:
		return GL_RGBA8_SNORM;
		break;
	case HAGE::R8G8B8A8_UINT		:
		return GL_RGBA8UI;
		break;
	case HAGE::R8G8B8A8_SINT		:
		return GL_RGBA8I;
		break;
	case HAGE::DXTC1_UNORM		:
		return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case HAGE::DXTC3_UNORM		:
		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case HAGE::DXTC5_UNORM		:
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case HAGE::DXTC1_UNORM_SRGB		:
	case HAGE::DXTC3_UNORM_SRGB		:
	case HAGE::DXTC5_UNORM_SRGB		:
	case HAGE::R8G8B8A8_TYPELESS	:
		assert(!"Not supported");
		return 0;
		break;
	default:
		assert(!"Unknown Format!");
		return 0;
	}
}

class OGL3Effect;

class OpenGL3APIWrapper : public HAGE::RenderingAPIWrapper
{
public:
	OpenGL3APIWrapper(const HAGE::APIWDisplaySettings* displaySettings);
	~OpenGL3APIWrapper();

	void SetRenderTarget(HAGE::APIWTexture* pTextureRenderTarget,HAGE::APIWTexture* pTextureDepthStencil,const HAGE::APIWViewport& viewport);
	void GetCurrentViewport(HAGE::APIWViewport& vpOut){vpOut=_currentViewport;}
	void UpdateDisplaySettings(const HAGE::APIWDisplaySettings* pSettings);
	void BeginFrame();
	void PresentFrame();
	void BeginAllocation();
	void EndAllocation();

	const OGL3Effect* GetCurrentEffect() const{return _currentEffect;}
	void SetCurrentEffect(const OGL3Effect* pEffect){_currentEffect=pEffect;}
	
	static HAGE::RenderingAPIWrapper* CreateOGL3Wrapper(const HAGE::APIWDisplaySettings* displaySettings);

	HAGE::Matrix4<> _GenerateProjectionMatrix(HAGE::f32 _near,HAGE::f32 _far,HAGE::f32 fovX,HAGE::f32 fovY)
	{
		HAGE::f32 h,w,Q;
		w = 1.0f/tan(fovX*0.5f);
		h = 1.0f/tan(fovY*0.5f);
		Q = _far/(_far-_near);
		return HAGE::Matrix4<>(
			HAGE::Vector4<>(w,			0.0f,	0.0f,			0.0f),
			HAGE::Vector4<>(0.0f,		h,		0.0f,			0.0f),
			HAGE::Vector4<>(0.0f,		0.0f,	(_far+_near)/(_far-_near),		-2.0f*Q*_near),
			HAGE::Vector4<>(0.0f,		0.0f,	1,				0.0f)
		);
	}
	
	HAGE::Matrix4<> GenerateProjectionMatrix(HAGE::f32 _near,HAGE::f32 _far,HAGE::f32 fovX,HAGE::f32 fovY)
	{
		if(_bForceCullFlip)
			return HAGE::Matrix4<>::Scale(HAGE::Vector3<>(1.0f,-1.0f,1.0f))*_GenerateProjectionMatrix(_near,_far,fovX,fovY);
		else
			return _GenerateProjectionMatrix(_near,_far,fovX,fovY);
	}
	HAGE::Matrix4<> GenerateRenderTargetProjection(HAGE::f32 _near,HAGE::f32 _far,HAGE::f32 fovX,HAGE::f32 fovY)
	{
		return HAGE::Matrix4<>::Scale(HAGE::Vector3<>(1.0f,-1.0f,1.0f))*_GenerateProjectionMatrix(_near,_far,fovX,fovY);
	}
	HAGE::Matrix4<> _GenerateProjectionMatrixBase()
	{
		return HAGE::Matrix4<>::One();
	}
	HAGE::Matrix4<> GenerateProjectionMatrixBase()
	{
		if(_bForceCullFlip)
			return HAGE::Matrix4<>::Scale(HAGE::Vector3<>(1.0f,-1.0f,1.0f))*_GenerateProjectionMatrixBase();
		else
			return _GenerateProjectionMatrixBase();
	}
	HAGE::Matrix4<> GenerateRenderTargetProjectionBase()
	{
		return HAGE::Matrix4<>::Scale(HAGE::Vector3<>(1.0f,-1.0f,1.0f))*_GenerateProjectionMatrixBase();
	}

	void RegisterVertexFormat(const char* szName,const HAGE::VertexDescriptionEntry* pDescription,HAGE::u32 nNumEntries);
	HAGE::APIWVertexArray* CreateVertexArray(
		HAGE::u32 nPrimitives,
		HAGE::APIWPrimitiveType PrimitiveType,
		HAGE::APIWVertexBuffer** pBuffers,
		HAGE::u32 nBuffers,
		const HAGE::u32* pIndexBufferData);
	HAGE::APIWVertexBuffer* CreateVertexBuffer(const char* szVertexFormat,const void* pData,HAGE::u32 nElements,bool bDynamic, bool bInstanceData);
	HAGE::APIWConstantBuffer* CreateConstantBuffer(HAGE::u32 nSize);
	virtual HAGE::APIWEffect* CreateEffect(const char* pProgram,
		const HAGE::APIWRasterizerState* pRasterizerState, const HAGE::APIWBlendState* pBlendState,
		const HAGE::u32 nBlendStates, bool AlphaToCoverage,
		const HAGE::APIWSampler* pSamplers,HAGE::u32 nSamplers );
	HAGE::APIWTexture* CreateTexture(HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels, HAGE::APIWFormat format,HAGE::u32 miscFlags,const void* pData,HAGE::u32 nDataSize);
	
	void UpdateTexture(HAGE::APIWTexture* pTexture,HAGE::u32 xOff,HAGE::u32 yOff,HAGE::u32 xSize,HAGE::u32 ySize,HAGE::u32 Level, const void* pData);
	bool ReadTexture(HAGE::APIWTexture* pTexture, const void** ppDataOut){return false;}
	
	HAGE::APIWFence* CreateStreamingFence(){return nullptr;}

	void FreeObject(HAGE::APIWObject* pObject){RenderingAPIWrapper::freeObject(pObject);}
	
	struct VertexFormatEntry
	{
		HAGE::u32						uVertexSize;
		HAGE::VertexDescriptionEntry*	pOriginalDescription;
		HAGE::u32						nElements;
	};

	HAGE::u8					GetVertexFormatCode(const char* name);
	HAGE::u32					GetVertexSize(HAGE::u8 code);
	const VertexFormatEntry*	GetVertexFormat(HAGE::u8 code);

	HAGE::u16					GetRasterizerStateCode(const HAGE::APIWRasterizerState* pState);
	void						SetRasterizerState(HAGE::u16 code);

	HAGE::u16					GetBlendStateCode(const HAGE::APIWBlendState* pState,HAGE::u32 nBlendStates,bool bAlphaToCoverage);
	void						SetBlendState(HAGE::u16 code);

	void						SetViewport(const HAGE::APIWViewport& vp){_currentViewport = vp;}

private:
	
	typedef std::basic_string<char,std::char_traits<char>,HAGE::global_allocator<char>> global_string;
	struct GlobalStringHash
	{
		size_t operator() (const global_string& key) const
		{
			size_t seed = 0xbadf00d;
			for(int i= 0;i<key.length();++i)
				boost::hash_combine<HAGE::u8>(seed,key[i]);
			return seed;
		}
	};


	typedef std::unordered_map<global_string,HAGE::u8,GlobalStringHash,std::equal_to<global_string>,HAGE::global_allocator<std::pair<global_string,HAGE::u8>>> VertexStringTableType;
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

	unsigned int				m_fboId;

	bool						_bForceDepthDisable;
	bool						_bForceCullFlip;

	HAGE::APIWDisplaySettings	_currentDisplaySettings;
	const OGL3Effect*			_currentEffect;

	HAGE::APIWViewport				_currentViewport;
	HAGE::APIWViewport				_backBufferViewport;

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

	HAGE::RenderDebugUI*		m_DebugUIRenderer;
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

class OGL3Texture : public HAGE::APIWTexture
{
public:
	OGL3Texture(OpenGL3APIWrapper* pWrapper,HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels, HAGE::APIWFormat format,HAGE::u32 miscFlags,const void* pData,HAGE::u32 nDataSize);
	void Clear(HAGE::Vector4<> Color);
	void Clear(bool bDepth,float depth,bool bStencil = false,HAGE::u32 stencil = 0);
	void GenerateMips();
	
	void StreamForReading(HAGE::u32 xOff,HAGE::u32 yOff,HAGE::u32 xSize,HAGE::u32 ySize);


	virtual ~OGL3Texture();
private:
	HAGE::u32			_xSize;
	HAGE::u32			_ySize;
	HAGE::u32			_mipLevels;
	HAGE::APIWFormat	_format;
	HAGE::u32			_miscFlags;

	bool				_bClearColor;
	HAGE::Vector4<>		_ClearColor;
	bool				_bClearDepth;
	HAGE::f32			_ClearDepth;
	bool				_bClearStencil;
	HAGE::u32			_ClearStencil;

	mutable GLsync		_streamSync;

	unsigned int		_tbo;
	friend class OGL3Effect;
	friend class OpenGL3APIWrapper;
};

class OGL3VertexArray : public HAGE::APIWVertexArray
{
public:
	OGL3VertexArray(OpenGL3APIWrapper* pWrapper,HAGE::u32 nPrimitives,HAGE::APIWPrimitiveType PrimitiveType,HAGE::APIWVertexBuffer** pBuffers,HAGE::u32 nBuffers, const HAGE::u32* pIndexBufferData);
	~OGL3VertexArray();

	unsigned int GetVA(GLuint program);

private:
	std::map<GLuint,unsigned int, std::less<GLuint>,HAGE::global_allocator<std::pair<GLuint,unsigned int> > >				m_vaoID;
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
	virtual void UpdateContent(const void* pData);
	~OGL3ConstantBuffer();
private:
	OpenGL3APIWrapper*			m_pWrapper;
	HAGE::u32					m_nSize;
	unsigned int				m_cbo;
	friend class OGL3Effect;
};

class OGL3Effect : public HAGE::APIWEffect
{
public:
	OGL3Effect(OpenGL3APIWrapper* pWrapper,const char* pProgram,HAGE::u16 rasterizer,HAGE::u16 blend,
		const HAGE::APIWSampler* pSamplers,HAGE::u32 nSamplers );
	~OGL3Effect();
	virtual void SetConstant(const char* pName,const HAGE::APIWConstantBuffer* constant);
	virtual void SetTexture(const char* pName,const HAGE::APIWTexture* texture);
	virtual void Draw(HAGE::APIWVertexArray* pArray);
private:
	OpenGL3APIWrapper*			m_pWrapper;
	HAGE::u16					m_RastState;
	HAGE::u16					m_BlendState;
	
	struct Sampler
	{
		GLuint sampler;
		HAGE::APIWSamplerState state;
	};

	std::vector<const OGL3ConstantBuffer*,HAGE::global_allocator<const OGL3ConstantBuffer*>>	_constantBuffers;	
	std::vector<int,HAGE::global_allocator<int>>												_textureSlots;
	std::vector<const OGL3Texture*,HAGE::global_allocator<const OGL3Texture*>>					_textures;
	std::vector<Sampler,HAGE::global_allocator<Sampler>>										_samplers;

	GLuint						_glVShader,_glFShader,_glGShader,_glProgram;
};

#endif

#endif
