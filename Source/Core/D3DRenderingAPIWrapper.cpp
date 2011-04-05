#include <HAGE.h>
#define INITGUID

#include "D3D11APIWrapper.h"
#include "ResourceDomain.h"

#ifndef NO_D3D

HWND GetHwnd();
HINSTANCE GetHInstance();
extern void SetWindowSize(bool bFullscreen,HAGE::u32 width,HAGE::u32 height);
extern void ChangeDisplayMode(HAGE::u32 xRes,HAGE::u32 yRes);
#define WM_THREADS_SHUTDOWN			WM_USER
#define WM_REMOTE_FUNCTION_CALL		WM_USER+1

class RemoteFunctionCall
{
public:
	virtual void Call() = 0;
};

inline D3D11_SAMPLER_DESC HAGESamplerToD3DSamplerState(const HAGE::APIWSamplerState& SamplerState);

class ChangeResolutionCall : public RemoteFunctionCall
{
public:
	ChangeResolutionCall(D3D11APIWrapper* pWrapper) : _pWrapper(pWrapper) {}
	void Call(){_pWrapper->_UpdateDisplaySettings_RemoteCall();}
private:
	D3D11APIWrapper* _pWrapper;
};

class InitCall : public RemoteFunctionCall
{
public:
	InitCall(D3D11APIWrapper* pWrapper) : _pWrapper(pWrapper) {}
	void Call(){_pWrapper->_Initialize_RemoteCall();}
private:
	D3D11APIWrapper* _pWrapper;
};

/*
bool D3D11APIWrapper::checkForCgError(const char *situation)
{
  CGerror error;
  bool bError = false;
  const char *string;
  while((string = cgGetLastErrorString(&error)) && error != CG_NO_ERROR)
	{
		error = cgGetError();
		if(situation)
			printf("%s: %s: %s\n","HAGE", situation, string);
		if (error == CG_COMPILER_ERROR) {
			if(situation)
				printf("%s\n", cgGetLastListing(myCgContext));
		}
		bError= true;
	}
  return bError;
}*/

HAGE::RenderingAPIWrapper* D3D11APIWrapper::CreateD3D11Wrapper(const HAGE::APIWDisplaySettings* displaySettings)
{
	RenderingAPIWrapper* pResult = new D3D11APIWrapper(displaySettings);
	_pAllocator= pResult;
	HAGE::domain_access<HAGE::ResourceDomain>::Get()->_RegisterResourceType(HAGE::guid_of<HAGE::IDrawableMesh>::Get(),&HAGE::CDrawableMeshLoader::Initialize);
	HAGE::domain_access<HAGE::ResourceDomain>::Get()->_RegisterResourceType(HAGE::guid_of<HAGE::ITextureImage>::Get(),&HAGE::CTextureImageLoader::Initialize);
	return pResult;
}

D3D11APIWrapper::D3D11APIWrapper(const HAGE::APIWDisplaySettings* pSettings) :
	m_hInst(GetHInstance()),
	m_hWnd(GetHwnd()),
	m_pSwapChain(nullptr),
	m_pDevice(nullptr),
	m_pContext(nullptr),
	m_pRenderTargetView(nullptr),
	m_pDepthStencilView(nullptr),
	m_NextVertexFormatEntry(0),
	_newDisplaySettings(*pSettings),
	_currentEffect(nullptr)
{
	InitCall initCall(this);
	SendMessage(m_hWnd,WM_REMOTE_FUNCTION_CALL,0,(LPARAM)&initCall);

	// adjust display settings
	_UpdateDisplaySettings();

	m_DebugUIRenderer = new HAGE::RenderDebugUI(this);
	D3D11_SAMPLER_DESC sampler= HAGESamplerToD3DSamplerState(HAGE::DefaultSamplerState);
	m_pDevice->CreateSamplerState(&sampler,&_defaultSampler);
}

void D3D11APIWrapper::_Initialize_RemoteCall()
{
	// Device and Swap Chain

	/*SetWindowSize(_currentDisplaySettings.bFullscreen,_currentDisplaySettings.xRes,_currentDisplaySettings.yRes);
	if(_currentDisplaySettings.bFullscreen)
		ChangeDisplayMode(_currentDisplaySettings.xRes,_currentDisplaySettings.yRes);*/

	HRESULT hr = S_OK;
	
    UINT createDeviceFlags = 0;//D3D11_CREATE_DEVICE_SINGLETHREADED;
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

	RECT window;
	GetClientRect(m_hWnd,&window);
	
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 2;
    sd.BufferDesc.Width = window.right-window.left;
    sd.BufferDesc.Height = window.bottom-window.top;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
	sd.Windowed = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    D3D_DRIVER_TYPE driverType;

	_currentDisplaySettings.xRes = sd.BufferDesc.Width;
	_currentDisplaySettings.yRes = sd.BufferDesc.Height;
	_currentDisplaySettings.bFullscreen = !sd.Windowed;

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
}

void D3D11APIWrapper::UpdateDisplaySettings(const HAGE::APIWDisplaySettings* pSettings)
{
	_newDisplaySettings = *pSettings;
}

extern void OSRequestMessageQueuePause();
extern void OSRequestMessageQueueResume();

void D3D11APIWrapper::_UpdateDisplaySettings()
{
	if((_newDisplaySettings.xRes != _currentDisplaySettings.xRes) || 
		(_newDisplaySettings.yRes != _currentDisplaySettings.yRes) ||
		(_newDisplaySettings.bFullscreen != _currentDisplaySettings.bFullscreen) )
	{
		
		ChangeResolutionCall updateCall(this);
		SendMessage(m_hWnd,WM_REMOTE_FUNCTION_CALL,0,(LPARAM)&updateCall);
	}
}

void D3D11APIWrapper::_UpdateDisplaySettings_RemoteCall()
{
	_currentDisplaySettings=_newDisplaySettings;
	if(m_pDepthStencilView)
		m_pDepthStencilView->Release();
	if(m_pRenderTargetView)
		m_pRenderTargetView->Release();
	ID3D11RenderTargetView* pTarget[1] = {nullptr};
	m_pContext->OMSetRenderTargets( 1, pTarget, nullptr );
	m_pContext->ClearState();

	IDXGIOutput* pOut = nullptr;
	m_pSwapChain->GetContainingOutput(&pOut);
	DXGI_MODE_DESC goal;	
	ZeroMemory( &goal, sizeof( goal ) );
	goal.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	goal.Width = _currentDisplaySettings.xRes;
	goal.Height = _currentDisplaySettings.yRes;
	DXGI_MODE_DESC enums[128];
	UINT numModes=128;
	INT goodMode=-1;
	pOut->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,0,&numModes,enums);
	for(int i =0;i<numModes;++i)
	{
		if(enums[i].Width == _currentDisplaySettings.xRes && enums[i].Height == _currentDisplaySettings.yRes)
			if(goodMode == -1 || (
				((float)enums[goodMode].RefreshRate.Numerator/(float)enums[goodMode].RefreshRate.Denominator) 
					< 
				(float)enums[i].RefreshRate.Numerator/(float)enums[i].RefreshRate.Denominator
				))
				goodMode = i;
	}
	assert(goodMode != -1);
	HRESULT hr;
	if(_currentDisplaySettings.bFullscreen)
		hr = m_pSwapChain->SetFullscreenState(true,pOut);
	else
		hr = m_pSwapChain->SetFullscreenState(false,nullptr);
	assert(SUCCEEDED(hr)); //causes deadlock in present
	//SetWindowSize(_currentDisplaySettings.bFullscreen,_currentDisplaySettings.xRes,_currentDisplaySettings.yRes);
	hr = m_pSwapChain->ResizeTarget(&enums[goodMode]);
	assert(SUCCEEDED(hr));
				
	hr = m_pSwapChain->ResizeBuffers(2,_currentDisplaySettings.xRes, _currentDisplaySettings.yRes,enums[goodMode].Format,DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	assert(SUCCEEDED(hr));
	pOut->Release();

		
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
	DSDesc.Width              = _currentDisplaySettings.xRes;
	DSDesc.Height             = _currentDisplaySettings.yRes;
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
	_vp.Width = (FLOAT)(_currentDisplaySettings.xRes);
	_vp.Height = (FLOAT)(_currentDisplaySettings.yRes);
	_vp.MinDepth = 0.0f;
	_vp.MaxDepth = 1.0f;
	_vp.TopLeftX = 0;
	_vp.TopLeftY = 0;

	_backBufferViewport.XMin = 0.0f;
	_backBufferViewport.XSize = (HAGE::f32)_currentDisplaySettings.xRes;
	_backBufferViewport.YMin = 0.0f;
	_backBufferViewport.YSize = (HAGE::f32)_currentDisplaySettings.yRes;
	_backBufferViewport.ZMin = 0.0f;
	_backBufferViewport.ZMax = 1.0f;

	_currentViewport = _backBufferViewport;
	
	m_pContext->RSSetViewports( 1, &_vp );
}

D3D11APIWrapper::~D3D11APIWrapper()
{
	delete m_DebugUIRenderer;

	if(m_pSwapChain)m_pSwapChain->Release();
	if(m_pRenderTargetView)m_pRenderTargetView->Release();
	if(m_pDepthStencilView)m_pDepthStencilView->Release();
	m_pContext->ClearState();
	if(m_pContext)m_pContext->Release();
	if(m_pDevice)m_pDevice->Release();
}

void D3D11APIWrapper::BeginAllocation()
{
    
}
void D3D11APIWrapper::EndAllocation()
{
    
}

void D3D11APIWrapper::BeginFrame()
{
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; 
    m_pContext->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
	m_pContext->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0 );
    m_pContext->OMSetRenderTargets( 1, &m_pRenderTargetView, m_pDepthStencilView );
}


void D3D11APIWrapper::PresentFrame()
{
	/*
	m_pContext->OMSetBlendState( myBlendState_NoBlend, 0, 0xffffffff );
    m_pContext->RSSetState( myRasterizerState_NoCull );
	*/

	// Debug UI
//	AttachThreadInput();
//	AttachThreadInput();
	m_DebugUIRenderer->Draw();

    // Show the rendered frame on the screen
	/*wm_magic_present magic;
	magic.pSwapChain = m_pSwapChain;
	SendMessage(m_hWnd,WM_MAGIC_PRESENT,0,(LPARAM)&magic);*/
	/*if(_currentDisplaySettings.bFullscreen)
		SetWindowPos(m_hWnd,HWND_TOPMOST,0,0,_currentDisplaySettings.xRes,_currentDisplaySettings.yRes,SWP_SHOWWINDOW|SWP_FRAMECHANGED);*/
	//printf("About to present on %08x\n",GetCurrentThreadId());
    m_pSwapChain->Present( 0, 0 );
	//printf("Presented on %08x\n",GetCurrentThreadId());
	/*
	static int i = 0;
	++i;
	if(i%10==0)
		_newDisplaySettings.bFullscreen = !_currentDisplaySettings.bFullscreen;*/

	_UpdateDisplaySettings();
	
	//framerate hack yay
    static HAGE::u64 last = 0;
	static HAGE::u64 freq = 0;
    static float sum = 0;
    static int nSum = 0;
    HAGE::u64 current;
    QueryPerformanceCounter((LARGE_INTEGER*)&current);

    HAGE::u64 diff = current - last;
    last = current;

	if(freq == 0)
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

    sum +=1.0f/(float)diff*(float)freq;
    nSum++;

    if(nSum%100==0)
        printf("Average %.02f - current %.02f\n",sum/(float)nSum,1.0f/(float)diff*(float)freq);
}

HAGE::APIWVertexBuffer* D3D11APIWrapper::CreateVertexBuffer(const char* szVertexFormat,const void* pData,HAGE::u32 nElements,bool bDynamic,bool bInstanceData)
{
	return new D3D11VertexBuffer(this,szVertexFormat,pData,nElements,bDynamic,bInstanceData);
}

HAGE::APIWConstantBuffer* D3D11APIWrapper::CreateConstantBuffer(HAGE::u32 nSize)
{
	return new D3D11ConstantBuffer(this,nSize);
}

HAGE::APIWTexture* D3D11APIWrapper::CreateTexture(HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels, HAGE::APIWFormat format,HAGE::u32 miscFlags,const void* pData,HAGE::u32 nDataSize)
{
	return new D3D11Texture(this,xSize,ySize,mipLevels,format, miscFlags,pData,nDataSize);
}

inline D3D11_RASTERIZER_DESC HAGERasterizerToD3DRasterizerState(const HAGE::APIWRasterizerState* pRasterizerState)
{
	D3D11_RASTERIZER_DESC res;
	res.FrontCounterClockwise = false;
	res.FillMode = pRasterizerState->bWireframe?D3D11_FILL_WIREFRAME:D3D11_FILL_SOLID;
	switch(pRasterizerState->CullMode)
	{
		case HAGE::CULL_NONE:	res.CullMode = D3D11_CULL_NONE;	break;
		case HAGE::CULL_CCW:	res.CullMode = D3D11_CULL_BACK;	break;
		case HAGE::CULL_CW:	res.CullMode = D3D11_CULL_FRONT;	break;
	}
	res.DepthBias				= pRasterizerState->iDepthBias;
	res.DepthBiasClamp			= pRasterizerState->fDepthBiasClamp;
	res.SlopeScaledDepthBias	= pRasterizerState->fSlopeScaledDepthBias;
	res.DepthClipEnable			= pRasterizerState->bDepthClipEnable;
	res.ScissorEnable			= pRasterizerState->bScissorEnable;
	res.MultisampleEnable		= pRasterizerState->bMultisampleEnable;
	return res;
}

inline D3D11_DEPTH_STENCIL_DESC HAGERasterizerToD3DDepthStencilTest(const HAGE::APIWRasterizerState* pRasterizerState)
{
	D3D11_DEPTH_STENCIL_DESC res;
	res.DepthEnable = pRasterizerState->bDepthClipEnable;
	res.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	res.DepthFunc = D3D11_COMPARISON_LESS;
	res.StencilEnable = false;
	res.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	res.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	res.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	res.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	res.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	res.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	res.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	res.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	res.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	res.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	return res;
}

inline D3D11_BLEND HAGEBlendModeToD3DBlendMode(HAGE::APIWBlendMode op)
{
	switch(op)
	{
		case HAGE::BLEND_ZERO:				return D3D11_BLEND_ZERO;				
		case HAGE::BLEND_ONE:				return D3D11_BLEND_ONE;		
		case HAGE::BLEND_SRC_COLOR:			return D3D11_BLEND_SRC_COLOR;		
		case HAGE::BLEND_INV_SRC_COLOR:		return D3D11_BLEND_INV_SRC_COLOR;				
		case HAGE::BLEND_SRC_ALPHA:			return D3D11_BLEND_SRC_ALPHA;		
		case HAGE::BLEND_INV_SRC_ALPHA:		return D3D11_BLEND_INV_SRC_ALPHA;				
		case HAGE::BLEND_DEST_ALPHA:		return D3D11_BLEND_DEST_ALPHA;		
		case HAGE::BLEND_INV_DEST_ALPHA:	return D3D11_BLEND_INV_DEST_ALPHA;		
		case HAGE::BLEND_DEST_COLOR:		return D3D11_BLEND_DEST_COLOR;				
		case HAGE::BLEND_INV_DEST_COLOR:	return D3D11_BLEND_INV_DEST_COLOR;		
		case HAGE::BLEND_SRC_ALPHA_SAT:		return D3D11_BLEND_SRC_ALPHA_SAT;				
		case HAGE::BLEND_BLEND_FACTOR:		return D3D11_BLEND_BLEND_FACTOR;		
		case HAGE::BLEND_INV_BLEND_FACTOR:	return D3D11_BLEND_INV_BLEND_FACTOR;		
		case HAGE::BLEND_SRC1_COLOR:		return D3D11_BLEND_SRC1_COLOR;				
		case HAGE::BLEND_INV_SRC_1_COLOR:	return D3D11_BLEND_INV_SRC1_COLOR;		
		case HAGE::BLEND_SRC1_ALPHA:		return D3D11_BLEND_SRC1_ALPHA;				
		case HAGE::BLEND_INV_SRC1_ALPHA:	return D3D11_BLEND_INV_SRC1_ALPHA;		
		default:							return D3D11_BLEND_ZERO;
	}
}

inline D3D11_BLEND_OP HAGEBlendOpToD3DBlendOp(HAGE::APIWBlendOp op)
{
	switch(op)
	{
		case HAGE::BLEND_OP_ADD:			return D3D11_BLEND_OP_ADD;				
		case HAGE::BLEND_OP_SUBTRACT:		return D3D11_BLEND_OP_SUBTRACT;		
		case HAGE::BLEND_OP_REV_SUBTRACT:	return D3D11_BLEND_OP_REV_SUBTRACT;		
		case HAGE::BLEND_OP_MIN:			return D3D11_BLEND_OP_MIN;				
		case HAGE::BLEND_OP_MAX:			return D3D11_BLEND_OP_MAX;		
		default:					return D3D11_BLEND_OP_ADD;
	}
}

inline D3D11_BLEND_DESC HAGEBlendToD3DBlendState(const HAGE::APIWBlendState* pBlendState,const HAGE::u32 nBlendStates, bool AlphaToCoverage)
{
	D3D11_BLEND_DESC res;
	res.AlphaToCoverageEnable		= AlphaToCoverage;
	res.IndependentBlendEnable		= (nBlendStates > 1);
	HAGE::u32 i;
	for(i = 0;i < nBlendStates; ++i)
	{
		res.RenderTarget[i].BlendEnable = pBlendState[i].bBlendEnable;
		res.RenderTarget[i].BlendOp		= HAGEBlendOpToD3DBlendOp(pBlendState[i].BlendOp);
		res.RenderTarget[i].BlendOpAlpha= HAGEBlendOpToD3DBlendOp(pBlendState[i].BlendOpAlpha);
		res.RenderTarget[i].DestBlend		= HAGEBlendModeToD3DBlendMode(pBlendState[i].DestBlend);
		res.RenderTarget[i].DestBlendAlpha	= HAGEBlendModeToD3DBlendMode(pBlendState[i].DestBlendAlpha);
		res.RenderTarget[i].SrcBlend		= HAGEBlendModeToD3DBlendMode(pBlendState[i].SrcBlend);
		res.RenderTarget[i].SrcBlendAlpha	= HAGEBlendModeToD3DBlendMode(pBlendState[i].SrcBlendAlpha);
		res.RenderTarget[i].RenderTargetWriteMask = 
									(pBlendState[i].bWriteR?D3D11_COLOR_WRITE_ENABLE_RED:0) |
									(pBlendState[i].bWriteG?D3D11_COLOR_WRITE_ENABLE_GREEN:0) |
									(pBlendState[i].bWriteB?D3D11_COLOR_WRITE_ENABLE_BLUE:0) |
									(pBlendState[i].bWriteA?D3D11_COLOR_WRITE_ENABLE_ALPHA:0);
	}
	if(nBlendStates>1)
		for(; i < 8;++i)
		{
			res.RenderTarget[i].BlendEnable = FALSE;
			res.RenderTarget[i].BlendOp		= D3D11_BLEND_OP_ADD;
			res.RenderTarget[i].BlendOpAlpha= D3D11_BLEND_OP_ADD;
			res.RenderTarget[i].DestBlend		= D3D11_BLEND_ZERO;
			res.RenderTarget[i].DestBlendAlpha	= D3D11_BLEND_ZERO;
			res.RenderTarget[i].SrcBlend		= D3D11_BLEND_ONE;
			res.RenderTarget[i].SrcBlendAlpha	= D3D11_BLEND_ONE;
			res.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
	return res;
}

inline D3D11_TEXTURE_ADDRESS_MODE HAGEAddressModeToD3DAddressMode(const HAGE::APIWAddressModes AddressMode)
{
	switch(AddressMode)
	{
	case HAGE::ADDRESS_WRAP:
		return D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	case HAGE::ADDRESS_MIRROR:
		return D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	case HAGE::ADDRESS_CLAMP:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	case HAGE::ADDRESS_BORDER:
		return D3D11_TEXTURE_ADDRESS_BORDER;
		break;
	case HAGE::ADDRESS_MIRROR_ONCE:
		return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		break;
	default:
		assert(!"Unknown Addressing Format!");
		return D3D11_TEXTURE_ADDRESS_WRAP;
	}
}

inline D3D11_COMPARISON_FUNC HAGEComparisonFuncToD3DComparisonFunc(const HAGE::APIWComparison comparison)
{
	switch(comparison)
	{
		case HAGE::COMPARISON_NEVER:
			return D3D11_COMPARISON_NEVER;
			break;
		case HAGE::COMPARISON_LESS:
			return D3D11_COMPARISON_LESS;
			break;
		case HAGE::COMPARISON_EQUAL:
			return D3D11_COMPARISON_EQUAL;
			break;
		case HAGE::COMPARISON_LESS_EQUAL:
			return D3D11_COMPARISON_LESS_EQUAL;
			break;
		case HAGE::COMPARISON_GREATER:
			return D3D11_COMPARISON_GREATER;
			break;
		case HAGE::COMPARISON_NOT_EQUAL:
			return D3D11_COMPARISON_NOT_EQUAL;
			break;
		case HAGE::COMPARISON_GREATER_EQUAL:
			return D3D11_COMPARISON_GREATER_EQUAL;
			break;
		case HAGE::COMPARISON_ALWAYS:
			return D3D11_COMPARISON_ALWAYS;
			break;
		default:
			assert(!"Unknown Comparison!");
			return D3D11_COMPARISON_NEVER;
	}
}

inline D3D11_FILTER HAGEFilterToD3DFilter(const HAGE::u32 FilterFlags)
{
	if(FilterFlags & HAGE::FILTER_COMPARISON)
	{
		if(FilterFlags & HAGE::FILTER_ANISOTROPIC)
			return D3D11_FILTER_COMPARISON_ANISOTROPIC;
		else
		{
			if(FilterFlags & HAGE::FILTER_MIN_LINEAR)
			{
				if(FilterFlags & HAGE::FILTER_MAG_LINEAR)
				{
					if(FilterFlags & HAGE::FILTER_MIP_LINEAR)
						return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
					else
						return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT ;
				}
				else
				{
					if(FilterFlags & HAGE::FILTER_MIP_LINEAR)
						return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
					else
						return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
				}
			}
			else
			{
				if(FilterFlags & HAGE::FILTER_MAG_LINEAR)
				{
					if(FilterFlags & HAGE::FILTER_MIP_LINEAR)
						return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
					else
						return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
				}
				else
				{
					if(FilterFlags & HAGE::FILTER_MIP_LINEAR)
						return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
					else
						return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
				}
			}
		}
	}
	else
	{
		if(FilterFlags & HAGE::FILTER_ANISOTROPIC)
			return D3D11_FILTER_ANISOTROPIC;
		else
		{
			if(FilterFlags & HAGE::FILTER_MIN_LINEAR)
			{
				if(FilterFlags & HAGE::FILTER_MAG_LINEAR)
				{
					if(FilterFlags & HAGE::FILTER_MIP_LINEAR)
						return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
					else
						return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT ;
				}
				else
				{
					if(FilterFlags & HAGE::FILTER_MIP_LINEAR)
						return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
					else
						return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
				}
			}
			else
			{
				if(FilterFlags & HAGE::FILTER_MAG_LINEAR)
				{
					if(FilterFlags & HAGE::FILTER_MIP_LINEAR)
						return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
					else
						return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
				}
				else
				{
					if(FilterFlags & HAGE::FILTER_MIP_LINEAR)
						return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
					else
						return D3D11_FILTER_MIN_MAG_MIP_POINT;
				}
			}
		}
	}
}

inline D3D11_SAMPLER_DESC HAGESamplerToD3DSamplerState(const HAGE::APIWSamplerState& SamplerState)
{
	D3D11_SAMPLER_DESC res;
	res.Filter = HAGEFilterToD3DFilter(SamplerState.FilterFlags);
	res.AddressU=HAGEAddressModeToD3DAddressMode(SamplerState.AddressModeU);
	res.AddressV=HAGEAddressModeToD3DAddressMode(SamplerState.AddressModeV);
	res.AddressW=HAGEAddressModeToD3DAddressMode(SamplerState.AddressModeW);
	res.BorderColor[0] = SamplerState.BorderColor[0];
	res.BorderColor[1] = SamplerState.BorderColor[1];
	res.BorderColor[2] = SamplerState.BorderColor[2];
	res.BorderColor[3] = SamplerState.BorderColor[3];
	res.ComparisonFunc =  HAGEComparisonFuncToD3DComparisonFunc(SamplerState.ComparisonFunction);
	res.MaxAnisotropy = SamplerState.MaxAnisotropy;
	res.MaxLOD = SamplerState.MipLODMax;
	res.MinLOD = SamplerState.MipLODMin;
	res.MipLODBias = SamplerState.MipLODBias;
	return res;
}
HAGE::APIWEffect* D3D11APIWrapper::CreateEffect(const char* pProgram,
		const HAGE::APIWRasterizerState* pRasterizerState, const HAGE::APIWBlendState* pBlendState,
		const HAGE::u32 nBlendStates, bool AlphaToCoverage,
		const HAGE::APIWSampler* pSamplers,HAGE::u32 nSamplers )
{
	ID3D11RasterizerState*	rast;
	ID3D11BlendState*		blend;
	ID3D11DepthStencilState*		depth;
	D3DSampler samplers[16]; // assume 16 samplers max
	assert(nSamplers <= 16); 
	D3D11_RASTERIZER_DESC	rastDesc = HAGERasterizerToD3DRasterizerState(pRasterizerState);
	D3D11_DEPTH_STENCIL_DESC	depthDesc = HAGERasterizerToD3DDepthStencilTest(pRasterizerState);
	D3D11_BLEND_DESC		blendDesc = HAGEBlendToD3DBlendState(pBlendState,nBlendStates,AlphaToCoverage);
	for(int i =0;i<nSamplers;++i)
	{
		D3D11_SAMPLER_DESC sampler= HAGESamplerToD3DSamplerState(pSamplers[i].State);
		memcpy(samplers[i].Name,pSamplers[i].SamplerName,32);
		m_pDevice->CreateSamplerState(&sampler,&samplers[i].pSampler);
	}
	m_pDevice->CreateRasterizerState(&rastDesc,&rast);
	m_pDevice->CreateBlendState(&blendDesc,&blend);
	m_pDevice->CreateDepthStencilState(&depthDesc,&depth);
	return new D3D11Effect(this,pProgram,rast,blend,depth,samplers,nSamplers);
}

HAGE::APIWVertexArray* D3D11APIWrapper::CreateVertexArray(HAGE::u32 nPrimitives,
		HAGE::APIWPrimitiveType PrimitiveType,
		HAGE::APIWVertexBuffer** pBuffers,
		HAGE::u32 nBuffers,
		const HAGE::u32* pIndexBufferData)
{
	return new D3D11VertexArray(this,nPrimitives,PrimitiveType,pBuffers,nBuffers,pIndexBufferData);
}

const D3D11APIWrapper::ArrayFormatEntry*	D3D11APIWrapper::GetArrayFormat(const VertexFormatKey& code)
{
	auto item=m_ArrayFormatList.find(code);
	if(item == m_ArrayFormatList.end())
	{
		//create entry
		HAGE::u32 nTotalEntries = 0;
		for(auto i=code.begin();i!=code.end();++i)
			nTotalEntries+=m_VertexFormatList[*i].nElements;

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

		m_ArrayFormatList.insert(std::pair<VertexFormatKey,ArrayFormatEntry>(code,new_entry));
		//done
		item=m_ArrayFormatList.find(code);
		assert(item != m_ArrayFormatList.end());
	}
	return &item->second;
}

HAGE::u8	D3D11APIWrapper::GetVertexFormatCode(const char* name)
{
	global_string s_name(name);
	auto item=m_VertexStringTable.find(s_name);
	assert(item != m_VertexStringTable.end());
	return item->second;
}

HAGE::u32	D3D11APIWrapper::GetVertexSize(HAGE::u8 code)
{
	return m_VertexFormatList[code].uVertexSize;
}

void D3D11APIWrapper::RegisterVertexFormat(const char* szName,const HAGE::VertexDescriptionEntry* pDescription,HAGE::u32 nNumEntries)
{
	global_string s_name(szName);
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
			case HAGE::R32G32B32A32_FLOAT:
				new_entry.pD3DDescription[out].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				vertex_size+=sizeof(HAGE::f32)*4;
				break;
			case HAGE::R32G32B32_FLOAT:
				new_entry.pD3DDescription[out].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				vertex_size+=sizeof(HAGE::f32)*3;
				break;
			case HAGE::R32G32_FLOAT:
				new_entry.pD3DDescription[out].Format = DXGI_FORMAT_R32G32_FLOAT;
				vertex_size+=sizeof(HAGE::f32)*2;
				break;
			case HAGE::R32_FLOAT:
				new_entry.pD3DDescription[out].Format = DXGI_FORMAT_R32_FLOAT;
				vertex_size+=sizeof(HAGE::f32)*1;
				break;
			}

			if(i==0)
				new_entry.pD3DDescription[out].AlignedByteOffset = 0;
			else
				new_entry.pD3DDescription[out].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		}

	new_entry.uVertexSize = vertex_size;

	m_VertexStringTable.insert(std::pair<global_string,HAGE::u8>(s_name,(HAGE::u8)m_NextVertexFormatEntry));

	++m_NextVertexFormatEntry;
}

void D3D11APIWrapper::SetRenderTarget(HAGE::APIWTexture* _pTextureRenderTarget,HAGE::APIWTexture* _pTextureDepthStencil,const HAGE::APIWViewport& viewport)
{
	D3D11Texture* pTextureRenderTarget=(D3D11Texture*)_pTextureRenderTarget;
	D3D11Texture* pTextureDepthStencil=(D3D11Texture*)_pTextureDepthStencil;

	ID3D11RenderTargetView* pRT[1];
	HAGE::u32 nRT = 1;
	ID3D11DepthStencilView* pDS;
	bool bViewportSet = false;
	D3D11_VIEWPORT				vp;

	if(viewport.XSize != 0)
	{
		bViewportSet = true;
		vp.MaxDepth = viewport.ZMax;
		vp.MinDepth = viewport.ZMin;
		vp.TopLeftX = viewport.XMin;
		vp.TopLeftY = viewport.YMin;
		vp.Width = viewport.XSize;
		vp.Height = viewport.YSize;
		_currentViewport = viewport;
	}

	if(_pTextureRenderTarget == HAGE::RENDER_TARGET_DEFAULT)
	{
		pRT[0] = m_pRenderTargetView;
		if(!bViewportSet)
		{
			vp = _vp;
			_currentViewport = _backBufferViewport;
			bViewportSet = true;
		}
		
		assert(_pTextureDepthStencil == HAGE::RENDER_TARGET_DEFAULT);
	}
	else if(_pTextureRenderTarget == HAGE::RENDER_TARGET_NONE)
	{
		pRT[0] = nullptr;
	}
	else
	{
		assert(pTextureRenderTarget->_renderTargetView);
		pRT[0] = pTextureRenderTarget->_renderTargetView;
		if(!bViewportSet)
		{
			vp.Width = (FLOAT)(pTextureRenderTarget->_xSize);
			vp.Height = (FLOAT)(pTextureRenderTarget->_ySize);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			_currentViewport.XMin = 0.0f;
			_currentViewport.XSize = pTextureRenderTarget->_xSize;
			_currentViewport.YMin = 0.0f;
			_currentViewport.YSize = pTextureRenderTarget->_ySize;
			_currentViewport.ZMin = 0.0f;
			_currentViewport.ZMax = 1.0f;
			bViewportSet = true;
		}
	}

	if(_pTextureDepthStencil == HAGE::RENDER_TARGET_DEFAULT)
	{
		pDS = m_pDepthStencilView;
		if(!bViewportSet)
		{
			vp = _vp;
			_currentViewport = _backBufferViewport;
			bViewportSet = true;
		}
		
		assert(_pTextureRenderTarget == HAGE::RENDER_TARGET_DEFAULT);
	}
	else if(_pTextureDepthStencil == HAGE::RENDER_TARGET_NONE)
	{
		pDS = nullptr;
	}
	else
	{
		assert(pTextureDepthStencil->_depthStencilView);
		pDS= pTextureDepthStencil->_depthStencilView;
		if(!bViewportSet)
		{
			vp.Width = (FLOAT)(pTextureDepthStencil->_xSize);
			vp.Height = (FLOAT)(pTextureDepthStencil->_ySize);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			_currentViewport.XMin = 0.0f;
			_currentViewport.XSize = pTextureDepthStencil->_xSize;
			_currentViewport.YMin = 0.0f;
			_currentViewport.YSize = pTextureDepthStencil->_ySize;
			_currentViewport.ZMin = 0.0f;
			_currentViewport.ZMax = 1.0f;
			bViewportSet = true;
		}
	}

	assert(bViewportSet);

	m_pContext->RSSetViewports( 1, &vp );
	m_pContext->OMSetRenderTargets(1,pRT,pDS);
}

#endif
