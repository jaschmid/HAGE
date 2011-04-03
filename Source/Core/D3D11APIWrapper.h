#ifndef D3D11__API__WRAPPER
#define D3D11__API__WRAPPER

#include <HAGE.h>
#include "RenderDebugUI.h"

//#define NO_D3D

#ifndef TARGET_WINDOWS
#define NO_D3D
#elif defined( COMPILER_GCC )
# include <specstrings.h> // define the MSVC++ extensions. Note: Possible conflicts with libstdc++
# define UINT8 unsigned char
#define NO_D3D // doesn't work anyway
#endif

#ifndef NO_D3D

#include <Windows.h>
#include <D3DCommon.h>
#include <D3D11.h>
#include <D3Dcompiler.h>
#include <boost/intrusive/list.hpp>
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <array>
#include <algorithm>

class D3D11APIWrapper : public HAGE::RenderingAPIWrapper
{
public:
	friend class D3D11Effect;
	typedef std::vector<HAGE::u8,HAGE::global_allocator<HAGE::u8> > VertexFormatKey;
	struct ArrayFormatEntry;

	D3D11APIWrapper(const HAGE::APIWDisplaySettings* pSettings);
	~D3D11APIWrapper();

	void SetRenderTarget(HAGE::APIWTexture* pTextureRenderTarget,HAGE::APIWTexture* pTextureDepthStencil,const HAGE::APIWViewport& viewport);
	void GetCurrentViewport(HAGE::APIWViewport& vpOut){vpOut=_currentViewport;}
	void UpdateDisplaySettings(const HAGE::APIWDisplaySettings* pSettings);
	void BeginFrame();
	void PresentFrame();
	void BeginAllocation();
	void EndAllocation();

	static HAGE::RenderingAPIWrapper* CreateD3D11Wrapper(const HAGE::APIWDisplaySettings* displaySettings);

	HAGE::Matrix4<> GenerateProjectionMatrix(HAGE::f32 _near,HAGE::f32 _far,HAGE::f32 fovX,HAGE::f32 fovY)
	{
		HAGE::f32 h,w,Q;
		w = 1.0f/tan(fovX*0.5f);
		h = 1.0f/tan(fovY*0.5f);
		Q = _far/(_far-_near);
		return HAGE::Matrix4<>(
			HAGE::Vector4<>(w,		0.0f,	0.0f,			0.0f),
			HAGE::Vector4<>(0.0f,		h,		0.0f,			0.0f),
			HAGE::Vector4<>(0.0f,		0.0f,	Q,				-Q*_near),
			HAGE::Vector4<>(0.0f,		0.0f,	1,				0.0f)
		);
	}
	HAGE::Matrix4<> GenerateRenderTargetProjection(HAGE::f32 _near,HAGE::f32 _far,HAGE::f32 fovX,HAGE::f32 fovY)
	{
		return GenerateProjectionMatrix(_near,_far,fovX,fovY);
	}
	HAGE::Matrix4<> GenerateProjectionMatrixBase()
	{
		return HAGE::Matrix4<>::One();
	}
	HAGE::Matrix4<> GenerateRenderTargetProjectionBase()
	{
		return HAGE::Matrix4<>::One();
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
	virtual HAGE::APIWTexture* CreateTexture(HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels, HAGE::APIWFormat format,HAGE::u32 miscFlags,const void* pData);

	ID3D11Device*				GetDevice(){return m_pDevice;}
	ID3D11DeviceContext*		GetContext(){return m_pContext;}

	const ArrayFormatEntry*		GetArrayFormat(const VertexFormatKey& code);
	HAGE::u8					GetVertexFormatCode(const char* name);
	HAGE::u32					GetVertexSize(HAGE::u8 code);

	struct VertexFormatEntry
	{
		HAGE::u32						uVertexSize;
		D3D11_INPUT_ELEMENT_DESC*		pD3DDescription;
		HAGE::VertexDescriptionEntry*	pOriginalDescription;
		HAGE::u32						nElements;
	};

	struct ArrayFormatEntry
	{
		D3D11_INPUT_ELEMENT_DESC*		pD3DDescription;
		HAGE::u32						nElements;
	};

	struct D3DSampler
	{
		char				Name[32];
		ID3D11SamplerState*	pSampler;
	};
	
	//should be called from main UI thread
	void _UpdateDisplaySettings_RemoteCall();
	void _Initialize_RemoteCall();

	inline ID3D11SamplerState*	GetDefaultSampler(){_defaultSampler->AddRef();return _defaultSampler;}
	inline D3D11Effect* GetCurrentEffect(){return _currentEffect;}
	inline void SetCurrentEffect(D3D11Effect* pNew){_currentEffect=pNew;}
	inline void	SetViewport(const HAGE::APIWViewport& vp){_currentViewport = vp;}
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
	
	struct VertexFormatHash
	{
		size_t operator() (const VertexFormatKey& key) const
		{
			size_t seed = 0xbadf00d;
			for(int i= 0;i<key.size();++i)
				boost::hash_combine<HAGE::u8>(seed,key[i]);
			return seed;
		}
	};

	typedef std::unordered_map<VertexFormatKey,ArrayFormatEntry,VertexFormatHash,std::equal_to<VertexFormatKey>,HAGE::global_allocator<std::pair<VertexFormatKey,ArrayFormatEntry>>> ArrayFormatListType;
	ArrayFormatListType			m_ArrayFormatList;
	
	void _UpdateDisplaySettings();


	HAGE::APIWDisplaySettings	_currentDisplaySettings;
	HAGE::APIWDisplaySettings	_newDisplaySettings;

	HINSTANCE                   m_hInst;
	HWND                        m_hWnd;
	HAGE::u32					m_NextVertexFormatEntry;

	IDXGISwapChain*             m_pSwapChain;
	ID3D11Device*               m_pDevice;
	ID3D11DeviceContext*        m_pContext;
	ID3D11RenderTargetView*     m_pRenderTargetView;
	ID3D11DepthStencilView *    m_pDepthStencilView;
	ID3D11SamplerState*			_defaultSampler;
	D3D11Effect*				_currentEffect;
	HAGE::APIWViewport				_currentViewport;
	HAGE::APIWViewport				_backBufferViewport;
    D3D11_VIEWPORT				_vp;

	HAGE::RenderDebugUI*		m_DebugUIRenderer;


};

class D3D11Texture : public HAGE::APIWTexture
{
public:
	D3D11Texture(D3D11APIWrapper* pWrapper,HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels, HAGE::APIWFormat format,HAGE::u32 miscFlags,const void* pData);
	void Clear(HAGE::Vector4<> Color);
	void Clear(bool bDepth,float depth,bool bStencil = false,HAGE::u32 stencil = 0);
	virtual ~D3D11Texture();
private:
	HAGE::u32			_xSize;
	HAGE::u32			_ySize;
	HAGE::u32			_mipLevels;
	HAGE::APIWFormat	_format;
	HAGE::u32			_miscFlags;
	ID3D11Texture2D*                    _texture;
	ID3D11ShaderResourceView*           _shaderResourceView;
	ID3D11RenderTargetView*				_renderTargetView;
	ID3D11DepthStencilView*				_depthStencilView;
	D3D11APIWrapper*					_pWrapper;

	friend class D3D11APIWrapper;
	friend class D3D11Effect;
};

class D3D11VertexBuffer : public HAGE::APIWVertexBuffer
{
public:
	D3D11VertexBuffer(D3D11APIWrapper* pWrapper,const char* szVertexFormat,const void* pData,HAGE::u32 nElements,bool bDynamic, bool bInstanceData);
	void UpdateContent(const void* pData);
	~D3D11VertexBuffer();

	HAGE::u8					GetCode() const{return m_nCode;}

private:
	ID3D11Buffer*               m_pVertexBuffer;
	D3D11APIWrapper*			m_pWrapper;
	HAGE::u32					m_nElements;
	bool						m_bInstance;
	HAGE::u8					m_nCode;
	friend class D3D11Effect;
	friend class D3D11VertexArray;
};

class D3D11VertexArray : public HAGE::APIWVertexArray
{
public:
	D3D11VertexArray(D3D11APIWrapper* pWrapper,HAGE::u32 nPrimitives,HAGE::APIWPrimitiveType PrimitiveType,HAGE::APIWVertexBuffer** pBuffers,HAGE::u32 nBuffers, const HAGE::u32* pIndexBufferData);
	~D3D11VertexArray();

private:
	std::vector<D3D11VertexBuffer*>	m_VertexBuffers;
	D3D11APIWrapper::VertexFormatKey				m_ArrayCode;
	HAGE::u32					m_nBuffers;
	HAGE::APIWPrimitiveType		m_PrimitiveType;
	HAGE::u32					m_nPrimitives;
	D3D11APIWrapper*			m_pWrapper;
	ID3D11Buffer*				m_pIndexBuffer;
	friend class D3D11Effect;
};

class D3D11ConstantBuffer : public HAGE::APIWConstantBuffer
{
public:
	D3D11ConstantBuffer(D3D11APIWrapper* pWrapper,HAGE::u32 nSize);
	virtual void UpdateContent(const void* pData);
	~D3D11ConstantBuffer();

private:
	D3D11APIWrapper*			m_pWrapper;
	ID3D11Buffer*				m_pBuffer;
	HAGE::u32					m_nSize;

	friend class D3D11Effect;
};


class D3D11Effect : public HAGE::APIWEffect, public boost::intrusive::list_base_hook<>
{
public:
	D3D11Effect(D3D11APIWrapper* pWrapper,const char* pProgram,ID3D11RasterizerState* pRasterizerState, ID3D11BlendState* pBlendState, ID3D11DepthStencilState* pDepthState, D3D11APIWrapper::D3DSampler* pSamplers,HAGE::u32 nSamplers);
	~D3D11Effect();
	
	virtual void SetConstant(const char* pName,const HAGE::APIWConstantBuffer* constant);
	virtual void SetTexture(const char* pName,const HAGE::APIWTexture* texture);
	virtual void Draw(HAGE::APIWVertexArray* pArray);
private:

	ID3D11VertexShader* CompileVertexShader(const char* shader);
	ID3D11PixelShader* CompilePixelShader(const char* shader);
	ID3D11GeometryShader* CompileGeometryShader(const char* shader);
	void CreateInputLayout(const D3D11APIWrapper::VertexFormatKey& v);

	ID3D11VertexShader*         m_pVertexShader;
	ID3D11PixelShader*          m_pPixelShader;
	ID3D11GeometryShader*       m_pGeometryShader;
    ID3D10Blob*					m_pCompiledShader;
	ID3D11RasterizerState*		m_pRasterizerState;
	ID3D11DepthStencilState*	m_pDepthState;
	ID3D11BlendState*			m_pBlendState;
	typedef D3D11APIWrapper::global_string global_string;
	typedef std::unordered_map<D3D11APIWrapper::VertexFormatKey,ID3D11InputLayout*,D3D11APIWrapper::VertexFormatHash,std::equal_to<D3D11APIWrapper::VertexFormatKey>,HAGE::global_allocator<std::pair<D3D11APIWrapper::VertexFormatKey,ID3D11InputLayout*>>> ArrayLayoutListType;
	ArrayLayoutListType			m_ArrayLayoutList;

	void ParseBindPoints(const D3D11_SHADER_INPUT_BIND_DESC& desc,int bindPoint);

	struct	BindPoints
	{
		union
		{
			struct
			{
				int _VSBindPoint;
				int _GSBindPoint;
				int _PSBindPoint;
			};
			int _ShaderStages[3];
		};
	};
	typedef std::map<D3D11APIWrapper::global_string,BindPoints,std::less<D3D11APIWrapper::global_string>,HAGE::global_allocator<std::pair<D3D11APIWrapper::global_string,BindPoints>>> BindPointMap;
	BindPointMap				_constantBinds;
	BindPointMap				_textureBinds;
	BindPointMap				_samplerBinds;

	std::vector<const D3D11ConstantBuffer*,HAGE::global_allocator<const D3D11ConstantBuffer*>>	_boundConstants[3];
	std::vector<const D3D11Texture*,HAGE::global_allocator<const D3D11Texture*>>				_boundTextures[3];
	std::vector<ID3D11SamplerState*,HAGE::global_allocator<ID3D11SamplerState*>>				_boundSamplers[3];

	D3D11APIWrapper*			m_pWrapper;
};

static DXGI_FORMAT APIWFormatToD3DFormat(const HAGE::APIWFormat& format)
{
	switch(format)
	{
	case HAGE::R16_UNORM			:
		return DXGI_FORMAT_R16_UNORM;
		break;
	case HAGE::R32_FLOAT			:
		return DXGI_FORMAT_R32_FLOAT;
		break;
	case HAGE::R32G32_FLOAT			:
		return DXGI_FORMAT_R32G32_FLOAT;
		break;
	case HAGE::R32G32B32_FLOAT		:
		return DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case HAGE::R32G32B32A32_FLOAT	:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case HAGE::R8G8B8A8_UNORM		:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case HAGE::R8G8B8A8_UNORM_SRGB	:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		break;
	case HAGE::R8G8B8A8_SNORM		:
		return DXGI_FORMAT_R8G8B8A8_SNORM;
		break;
	case HAGE::R8G8B8A8_UINT		:
		return DXGI_FORMAT_R8G8B8A8_UINT;
		break;
	case HAGE::R8G8B8A8_SINT		:
		return DXGI_FORMAT_R8G8B8A8_SINT;
		break;
	case HAGE::R8G8B8A8_TYPELESS	:
		return DXGI_FORMAT_R8G8B8A8_TYPELESS;
		break;
	case HAGE::DXTC1_UNORM:
		return DXGI_FORMAT_BC1_UNORM;
		break;
	case HAGE::DXTC1_UNORM_SRGB:
		return DXGI_FORMAT_BC1_UNORM_SRGB;
		break;
	case HAGE::DXTC3_UNORM:
		return DXGI_FORMAT_BC2_UNORM;
		break;
	case HAGE::DXTC3_UNORM_SRGB:
		return DXGI_FORMAT_BC2_UNORM_SRGB;
		break;
	case HAGE::DXTC5_UNORM:
		return DXGI_FORMAT_BC3_UNORM;
		break;
	case HAGE::DXTC5_UNORM_SRGB:
		return DXGI_FORMAT_BC3_UNORM_SRGB;
		break;
	default:
		assert(!"Unknown Format!");
		return DXGI_FORMAT_UNKNOWN;
	}
}


#endif
#endif
