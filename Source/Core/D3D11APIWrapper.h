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
#include <Cg/cg.h>
#include <Cg/cgD3D11.h>
#include <boost/intrusive/list.hpp>
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <array>
#include <algorithm>

class D3D11APIWrapper : public HAGE::RenderingAPIWrapper
{
public:
	typedef std::vector<HAGE::u8,HAGE::global_allocator<HAGE::u8> > VertexFormatKey;
	struct ArrayFormatEntry;

	D3D11APIWrapper();
	~D3D11APIWrapper();

	void SetRenderTarget(HAGE::APIWTexture* pTextureRenderTarget,HAGE::APIWTexture* pTextureDepthStencil);
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
	virtual HAGE::APIWEffect* CreateEffect(const char* pProgram,
		const HAGE::APIWRasterizerState* pRasterizerState, const HAGE::APIWBlendState* pBlendState,
		const HAGE::u32 nBlendStates, bool AlphaToCoverage);
	virtual HAGE::APIWTexture* CreateTexture(HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels, HAGE::APIWFormat format,HAGE::u32 miscFlags,const void* pData);

	ID3D11Device*				GetDevice(){return m_pDevice;}
	ID3D11DeviceContext*		GetContext(){return m_pContext;}
	CGcontext& GetCGC(){return myCgContext;}
	CGprofile& GetVertexProfile(){return myCgVertexProfile;}
	CGprofile& GetFragmentProfile(){return myCgFragmentProfile;}
	CGprofile& GetGeometryProfile(){return myCgGeometryProfile;}

	const ArrayFormatEntry*		GetArrayFormat(const VertexFormatKey& code);
	HAGE::u8					GetVertexFormatCode(const char* name);
	HAGE::u32					GetVertexSize(HAGE::u8 code);

	bool checkForCgError(const char *situation);


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
	

	HINSTANCE                   m_hInst;
	HWND                        m_hWnd;
	HAGE::u32					m_NextVertexFormatEntry;

	IDXGISwapChain*             m_pSwapChain;
	ID3D11Device*               m_pDevice;
	ID3D11DeviceContext*        m_pContext;
	ID3D11RenderTargetView*     m_pRenderTargetView;
	ID3D11DepthStencilView *    m_pDepthStencilView;
    D3D11_VIEWPORT				_vp;

	CGcontext					myCgContext;
	CGprofile					myCgVertexProfile, myCgFragmentProfile, myCgGeometryProfile;

	HAGE::RenderDebugUI*		m_DebugUIRenderer;

	friend class D3D11Effect;

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
    D3D11_VIEWPORT				_vp;

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
	D3D11Effect(D3D11APIWrapper* pWrapper,const char* pProgram,ID3D11RasterizerState* pRasterizerState, ID3D11BlendState* pBlendState);
	~D3D11Effect();
	
	virtual void Draw(HAGE::APIWVertexArray* pArray,HAGE::APIWConstantBuffer* const * pConstants,HAGE::u32 nConstants = 1,HAGE::APIWTexture* const * pTextures = nullptr,HAGE::u32 nTextures = 0);
private:

	ID3D11VertexShader* CompileVertexShader(const char* shader);
	ID3D11PixelShader* CompilePixelShader(const char* shader);
	ID3D11GeometryShader* CompileGeometryShader(const char* shader);
	void CreateInputLayout(const D3D11APIWrapper::VertexFormatKey& v);

	CGprogram					m_CgVertexProgram;
	CGprogram					m_CgFragmentProgram;
	CGprogram					m_CgGeometryProgram;
	ID3D11VertexShader*         m_pVertexShader;
	ID3D11PixelShader*          m_pPixelShader;
	ID3D11GeometryShader*       m_pGeometryShader;
    ID3D10Blob*					m_pCompiledShader;
	ID3D11RasterizerState*		m_pRasterizerState;
	ID3D11BlendState*			m_pBlendState;

	typedef std::unordered_map<D3D11APIWrapper::VertexFormatKey,ID3D11InputLayout*,D3D11APIWrapper::VertexFormatHash,std::equal_to<D3D11APIWrapper::VertexFormatKey>,HAGE::global_allocator<std::pair<D3D11APIWrapper::VertexFormatKey,ID3D11InputLayout*>>> ArrayLayoutListType;
	ArrayLayoutListType			m_ArrayLayoutList;

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
	default:
		assert(!"Unknown Format!");
		return DXGI_FORMAT_UNKNOWN;
	}
}

#endif
#endif
