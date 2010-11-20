#include <HAGE.h>
#include "D3D11APIWrapper.h"

#ifndef NO_D3D

HWND GetHwnd();
HINSTANCE GetHInstance();

void D3D11APIWrapper::checkForCgError(const char *situation)
{
  CGerror error;
  const char *string = cgGetLastErrorString(&error);

  if (error != CG_NO_ERROR) {
    printf("%s: %s: %s\n",
      "HAGE", situation, string);
    if (error == CG_COMPILER_ERROR) {
      printf("%s\n", cgGetLastListing(myCgContext));
    }
    exit(1);
  }
}

HAGE::RenderingAPIWrapper* HAGE::RenderingAPIWrapper::CreateD3D11Wrapper()
{
	return new D3D11APIWrapper();
}

D3D11APIWrapper::D3D11APIWrapper() :
	m_hInst(GetHInstance()),
	m_hWnd(GetHwnd()),
	m_pSwapChain(nullptr),
	m_pDevice(nullptr),
	m_pContext(nullptr),
	m_pRenderTargetView(nullptr),
	m_pDepthStencilView(nullptr),
	m_NextVertexFormatEntry(0)
{
	// Device and Swap Chain

	HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( m_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = 2;

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = _countof(featureLevels);
    D3D_FEATURE_LEVEL featureLevelOut;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    D3D_DRIVER_TYPE driverType;
    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, driverType, NULL, createDeviceFlags,
                                            featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
                                            &sd, &m_pSwapChain, &m_pDevice, &featureLevelOut,
                                            &m_pContext );
        if( SUCCEEDED( hr ) )
            break;
    }

	// Create the render target view
	{
		ID3D11Texture2D* pRenderTargetTexture;
		hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pRenderTargetTexture );

		assert( SUCCEEDED( hr ) );

		hr = m_pDevice->CreateRenderTargetView( pRenderTargetTexture, NULL, &m_pRenderTargetView );
		pRenderTargetTexture->Release();

		assert( SUCCEEDED( hr ) );
	}

	// Create Depth Stencil
	D3D11_TEXTURE2D_DESC DSDesc;
	DSDesc.ArraySize          = 1;
	DSDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
	DSDesc.CPUAccessFlags     = 0;
	DSDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DSDesc.Width              = width;
    DSDesc.Height             = height;
	DSDesc.MipLevels          = 1;
	DSDesc.MiscFlags          = 0;
	DSDesc.SampleDesc.Count   = 1;
	DSDesc.SampleDesc.Quality = 0;
	DSDesc.Usage              = D3D11_USAGE_DEFAULT;

	{
		ID3D11Texture2D * DSBuffer;
		hr = m_pDevice->CreateTexture2D( &DSDesc, NULL, &DSBuffer );

		assert( SUCCEEDED( hr ) );

		hr = m_pDevice->CreateDepthStencilView( DSBuffer, NULL, &m_pDepthStencilView );

		DSBuffer->Release();

		assert( SUCCEEDED( hr ) );
	}

    m_pContext->OMSetRenderTargets( 1, &m_pRenderTargetView, m_pDepthStencilView );


	// set viewport
    GetClientRect( m_hWnd, &rc );
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)(rc.right - rc.left);
    vp.Height = (FLOAT)(rc.bottom - rc.top);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pContext->RSSetViewports( 1, &vp );

	// INIT CG

	myCgContext = cgCreateContext();
    checkForCgError( "creating context" );

	hr = cgD3D11SetDevice( myCgContext, m_pDevice );
    checkForCgError( "setting Direct3D device" );
    assert( hr == S_OK );

	myCgVertexProfile = CG_PROFILE_VS_4_0;
	myCgFragmentProfile = CG_PROFILE_PS_4_0;

	// rasterizing state
	/*
	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_NONE;
	desc.FrontCounterClockwise = false;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0;
	desc.SlopeScaledDepthBias = 0;
	desc.DepthClipEnable = false;
	desc.ScissorEnable = false;
	desc.MultisampleEnable = false;
	desc.AntialiasedLineEnable = false;
	ID3D11RasterizerState* pState;

	m_pDevice->CreateRasterizerState(&desc,&pState);

	m_pContext->RSSetState(pState);*/

}

D3D11APIWrapper::~D3D11APIWrapper()
{
	// CG sucks and forgets to release the device
	m_pContext->Release();
    cgD3D11SetDevice( myCgContext, NULL );

    cgDestroyContext( myCgContext );

	if(m_pSwapChain)m_pSwapChain->Release();
	if(m_pRenderTargetView)m_pRenderTargetView->Release();
	if(m_pDepthStencilView)m_pDepthStencilView->Release();
	m_pContext->ClearState();
	if(m_pContext)m_pContext->Release();
	if(m_pDevice)m_pDevice->Release();
}

void D3D11APIWrapper::BeginFrame()
{
	// Clear the render target to dark blue
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    m_pContext->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
	m_pContext->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0 );
    m_pContext->OMSetRenderTargets( 1, &m_pRenderTargetView, NULL );
}

void D3D11APIWrapper::PresentFrame()
{
	/*
	m_pContext->OMSetBlendState( myBlendState_NoBlend, 0, 0xffffffff );
    m_pContext->RSSetState( myRasterizerState_NoCull );
	*/

    // Show the rendered frame on the screen
    m_pSwapChain->Present( 0, 0 );
}

HAGE::APIWVertexBuffer* D3D11APIWrapper::CreateVertexBuffer(const char* szVertexFormat,void* pData,HAGE::u32 nElements,bool bInstanceData)
{
	return new D3D11VertexBuffer(this,szVertexFormat,pData,nElements,bInstanceData);
}

HAGE::APIWConstantBuffer* D3D11APIWrapper::CreateConstantBuffer(HAGE::u32 nSize)
{
	return new D3D11ConstantBuffer(this,nSize);
}

HAGE::APIWEffect* D3D11APIWrapper::CreateEffect(const char* pVertexProgram,const char* pFragmentProgram)
{
	return new D3D11Effect(this,pVertexProgram,pFragmentProgram);
}

HAGE::APIWVertexArray* D3D11APIWrapper::CreateVertexArray(HAGE::u32 nPrimitives,
		HAGE::APIWPrimitiveType PrimitiveType,
		HAGE::APIWVertexBuffer** pBuffers,
		HAGE::u32 nBuffers,
		const HAGE::u32* pIndexBufferData)
{
	return new D3D11VertexArray(this,nPrimitives,PrimitiveType,pBuffers,nBuffers,pIndexBufferData);
}

const D3D11APIWrapper::ArrayFormatEntry*	D3D11APIWrapper::GetArrayFormat(const std::vector<HAGE::u8>& code)
{
	auto item=m_ArrayFormatList.find(code);
	if(item == m_ArrayFormatList.end())
	{
		//create entry
		HAGE::u32 nTotalEntries = 0;
		for(auto i=code.begin();i!=code.end();++i)
			nTotalEntries+=m_VertexFormatList[i-code.begin()].nElements;

		ArrayFormatEntry new_entry;
		new_entry.nElements=nTotalEntries;
		new_entry.pD3DDescription = new D3D11_INPUT_ELEMENT_DESC[nTotalEntries];

		int out = 0;
		for(auto i=code.begin();i!=code.end();++i)
			for(HAGE::u32 j=0;j<m_VertexFormatList[*i].nElements;++j,++out)
			{
				new_entry.pD3DDescription[out] = m_VertexFormatList[*i].pD3DDescription[j];
				new_entry.pD3DDescription[out].InputSlot = (UINT)(i-code.begin());
				new_entry.pD3DDescription[out].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				new_entry.pD3DDescription[out].InstanceDataStepRate = 0;
			}

		m_ArrayFormatList.insert(std::pair<std::vector<HAGE::u8>,ArrayFormatEntry>(code,new_entry));
		//done
		item=m_ArrayFormatList.find(code);
		assert(item != m_ArrayFormatList.end());
	}
	return &item->second;
}

HAGE::u8	D3D11APIWrapper::GetVertexFormatCode(const char* name)
{
	std::string s_name(name);
	auto item=m_VertexStringTable.find(s_name);
	assert(item != m_VertexStringTable.end());
	return item->second;
}

HAGE::u32	D3D11APIWrapper::GetVertexSize(HAGE::u8 code)
{
	return m_VertexFormatList[code].uVertexSize;
}

void D3D11APIWrapper::RegisterVertexFormat(const char* szName,HAGE::VertexDescriptionEntry* pDescription,HAGE::u32 nNumEntries)
{
	std::string s_name(szName);
	assert( m_VertexStringTable.find(s_name) == m_VertexStringTable.end());
	assert( m_NextVertexFormatEntry <= 255 );

	VertexFormatEntry& new_entry = m_VertexFormatList[m_NextVertexFormatEntry];


	new_entry.pOriginalDescription = new HAGE::VertexDescriptionEntry[nNumEntries];
	memcpy(new_entry.pOriginalDescription,pDescription,sizeof(HAGE::VertexDescriptionEntry)*nNumEntries);

	HAGE::u32 nNumD3DEntries = 0;

	for(HAGE::u32 i=0;i<nNumEntries;++i)
		nNumD3DEntries+=pDescription[i].nSubElements;

	new_entry.pD3DDescription = new D3D11_INPUT_ELEMENT_DESC[nNumD3DEntries];
	new_entry.nElements = nNumD3DEntries;

	int out = 0;
	int vertex_size = 0;
	for(HAGE::u32 i=0;i<nNumEntries;++i)
		for(HAGE::u32 j=0;j<pDescription[i].nSubElements;++j,++out)
		{
			memset(&new_entry.pD3DDescription[out],sizeof(D3D11_INPUT_ELEMENT_DESC),0);

			new_entry.pD3DDescription[out].SemanticName = new_entry.pOriginalDescription[i].pName;
			new_entry.pD3DDescription[out].SemanticIndex = j;
			switch(new_entry.pOriginalDescription[i].fFormat)
			{
			case HAGE::R32G32B32_FLOAT:
				new_entry.pD3DDescription[out].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				vertex_size+=sizeof(HAGE::f32)*3;
				break;
			}

			if(i==0)
				new_entry.pD3DDescription[out].AlignedByteOffset = 0;
			else
				new_entry.pD3DDescription[out].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		}

	new_entry.uVertexSize = vertex_size;

	m_VertexStringTable.insert(std::pair<std::string,HAGE::u8>(s_name,(HAGE::u8)m_NextVertexFormatEntry));

	++m_NextVertexFormatEntry;
}

#endif
