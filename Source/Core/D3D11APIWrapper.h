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
	struct ArrayFormatEntry;

	D3D11APIWrapper();
	~D3D11APIWrapper();

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

	ID3D11Device*				GetDevice(){return m_pDevice;}
	ID3D11DeviceContext*		GetContext(){return m_pContext;}
	CGcontext& GetCGC(){return myCgContext;}
	CGprofile& GetVertexProfile(){return myCgVertexProfile;}
	CGprofile& GetFragmentProfile(){return myCgFragmentProfile;}

	const ArrayFormatEntry*		GetArrayFormat(const std::vector<HAGE::u8>& code);
	HAGE::u8					GetVertexFormatCode(const char* name);
	HAGE::u32					GetVertexSize(HAGE::u8 code);

	void checkForCgError(const char *situation);


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
	typedef std::unordered_map<std::string,HAGE::u8> VertexStringTableType;
	VertexStringTableType		m_VertexStringTable;
	typedef std::array<VertexFormatEntry,256> VertexFormatListType;
	VertexFormatListType		m_VertexFormatList;

	struct VertexFormatHash
	{
		size_t operator() (const std::vector<HAGE::u8>& key) const
		{
			size_t seed = 0xbadf00d;
			for(auto i= key.begin();i!=key.end();++i)
				boost::hash_combine<HAGE::u8>(seed,*i);
			return seed;
		}
	};

	typedef std::unordered_map<std::vector<HAGE::u8>,ArrayFormatEntry,VertexFormatHash> ArrayFormatListType;
	ArrayFormatListType			m_ArrayFormatList;

	HINSTANCE                   m_hInst;
	HWND                        m_hWnd;
	HAGE::u32					m_NextVertexFormatEntry;

	IDXGISwapChain*             m_pSwapChain;
	ID3D11Device*               m_pDevice;
	ID3D11DeviceContext*        m_pContext;
	ID3D11RenderTargetView*     m_pRenderTargetView;
	ID3D11DepthStencilView *    m_pDepthStencilView;

	CGcontext					myCgContext;
	CGprofile					myCgVertexProfile, myCgFragmentProfile;

	HAGE::RenderDebugUI*		m_DebugUIRenderer;

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
	std::vector<HAGE::u8>		m_ArrayCode;
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
	D3D11Effect(D3D11APIWrapper* pWrapper,const char* pVertexProgram,const char* pFragmentProgram,ID3D11RasterizerState* pRasterizerState, ID3D11BlendState* pBlendState);
	~D3D11Effect();

	virtual void Draw(HAGE::APIWVertexArray* pArray,HAGE::APIWConstantBuffer* const * pConstants,HAGE::u32 nConstants = 1);
private:

	ID3D11VertexShader* CompileVertexShader(const char* shader);
	ID3D11PixelShader* CompilePixelShader(const char* shader);
	void CreateInputLayout(std::vector<HAGE::u8> v);

	CGprogram					m_CgVertexProgram;
	CGprogram					m_CgFragmentProgram;
	ID3D11VertexShader*         m_pVertexShader;
	ID3D11PixelShader*          m_pPixelShader;
    ID3D10Blob*					m_pCompiledShader;
	ID3D11RasterizerState*		m_pRasterizerState;
	ID3D11BlendState*			m_pBlendState;

	typedef std::unordered_map<std::vector<HAGE::u8>,ID3D11InputLayout*,D3D11APIWrapper::VertexFormatHash> ArrayLayoutListType;
	ArrayLayoutListType			m_ArrayLayoutList;

	D3D11APIWrapper*			m_pWrapper;
};

#endif
#endif
