#include "Hage.h"
#include "D3D11APIWrapper.h"

D3D11_TEXTURE2D_DESC APIWToD3D11Texture2DDesc(HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels,HAGE::APIWFormat format,HAGE::u32 miscFlags)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.ArraySize = 1;
	desc.Format = APIWFormatToD3DFormat(format);
	desc.Height = ySize;
	desc.Width = xSize;
	desc.MipLevels = mipLevels;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	if(!(miscFlags & HAGE::TEXTURE_GPU_WRITE || miscFlags & HAGE::TEXTURE_GPU_DEPTH_STENCIL || HAGE::TEXTURE_GPU_COPY|| HAGE::TEXTURE_CPU_WRITE|| HAGE::TEXTURE_CPU_READ))
	{
		desc.Usage = D3D11_USAGE_IMMUTABLE ;
		desc.CPUAccessFlags = 0;
	}
	else
	{
		desc.Usage = D3D11_USAGE_DEFAULT ;
		desc.CPUAccessFlags = 0;
	}

	desc.BindFlags = 0;

	if(!(miscFlags & HAGE::TEXTURE_GPU_NO_READ))
	{
		desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	}
	if(miscFlags & HAGE::TEXTURE_GPU_WRITE)
	{
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	if(miscFlags & HAGE::TEXTURE_GPU_DEPTH_STENCIL)
	{
		switch(desc.Format)
		{
		case DXGI_FORMAT_R32_FLOAT:
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			break;
		case DXGI_FORMAT_R16_UNORM:
			desc.Format = DXGI_FORMAT_R16_TYPELESS;
			break;
		default:
			assert(!"Unsupported format for depth stencil");
		}
		desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	}

	desc.MiscFlags = 0;

	if(miscFlags & HAGE::TEXTURE_CUBE )
	{
		desc.ArraySize = 6;
		desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE ;
	}
	return desc;
}


D3D11Texture::D3D11Texture(D3D11APIWrapper* pWrapper,HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels, HAGE::APIWFormat format,HAGE::u32 miscFlags,const void* pData,HAGE::u32 nDataSize)
	: _pWrapper(pWrapper),_xSize(xSize),_ySize(ySize),_mipLevels(mipLevels),_format(format),_miscFlags(miscFlags),_texture(nullptr),_shaderResourceView(nullptr),_renderTargetView(nullptr),_depthStencilView(nullptr)
{

	D3D11_TEXTURE2D_DESC desc = APIWToD3D11Texture2DDesc(_xSize,_ySize,_mipLevels,_format,_miscFlags);

	D3D11_SUBRESOURCE_DATA data[16];
	D3D11_SUBRESOURCE_DATA* pLocalData = nullptr;

	if(pData)
	{		
		HAGE::u8* pMipData = (HAGE::u8*)pData;
		int w = xSize;
		int h = ySize;
		for(int i =0;i<mipLevels;++i)
		{
			data[i].pSysMem = pMipData;
			data[i].SysMemPitch = HAGE::APIWFormatImagePhysicalPitch(format,w);
			data[i].SysMemSlicePitch = HAGE::APIWFormatImagePhysicalSize(format,w,h);

			pMipData+=APIWFormatImagePhysicalSize(format,w,h);
			w>>=1;
			h>>=1;
			if(w==0)
				w=1;
			if(h==0)
				h=1;
		}
		pLocalData = data;
	}

	pWrapper->EnterDeviceCritical();
	HRESULT hres = pWrapper->GetDevice()->CreateTexture2D(&desc,pLocalData,&_texture);
	pWrapper->LeaveDeviceCritical();
	assert(SUCCEEDED(hres));
	
	
	if(_miscFlags & HAGE::TEXTURE_CPU_READ)
	{
		//create reading texture
		_pReadData = new _ReadData;

		D3D11_TEXTURE2D_DESC descCPURead = desc;
		descCPURead.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
		descCPURead.Usage = D3D11_USAGE_STAGING;
		descCPURead.BindFlags = 0;
		
		D3D11_QUERY_DESC queryDesc;
		queryDesc.MiscFlags = 0;
		queryDesc.Query = D3D11_QUERY_EVENT;

		pWrapper->EnterDeviceCritical();
		HRESULT hres = pWrapper->GetDevice()->CreateTexture2D(&descCPURead,nullptr,&_pReadData->textureStagingRead);
		pWrapper->GetDevice()->CreateQuery(&queryDesc,&_pReadData->pReadOpQuery);
		pWrapper->LeaveDeviceCritical();
		assert(SUCCEEDED(hres));

		_pReadData->pixelSize= APIWFormatPixelSize(format);
		_pReadData->bReadOpPending = false;
		_pReadData->nReadBufferSize = APIWFormatImagePhysicalSize(format,xSize,ySize);
		_pReadData->pReadBuffer = new HAGE::u8[_pReadData->nReadBufferSize];

	}
	else
		_pReadData = nullptr;


	if(!(_miscFlags & HAGE::TEXTURE_GPU_NO_READ))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		switch(desc.Format)
		{
		case DXGI_FORMAT_R32_TYPELESS:
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			break;
		case DXGI_FORMAT_R16_TYPELESS:
			srvDesc.Format = DXGI_FORMAT_R16_UNORM;
			break;
		default:
			srvDesc.Format = desc.Format;
		}
		if(desc.ArraySize == 1)
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
		}
		else if(_miscFlags & HAGE::TEXTURE_CUBE )
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = desc.MipLevels;
			srvDesc.TextureCube.MostDetailedMip = 0;
		}
		hres = pWrapper->GetDevice()->CreateShaderResourceView( _texture, &srvDesc, &_shaderResourceView );
		assert(SUCCEEDED(hres));
	}
	if(_miscFlags & HAGE::TEXTURE_GPU_DEPTH_STENCIL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
		switch(desc.Format)
		{
		case DXGI_FORMAT_R32_TYPELESS:
			depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
			break;
		case DXGI_FORMAT_R16_TYPELESS:
			depthDesc.Format = DXGI_FORMAT_D16_UNORM;
			break;
		default:
			assert(!"Unrecognized Depth Stencil Format");
		}
		if(desc.ArraySize == 1)
		{
			depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthDesc.Texture2D.MipSlice = 0;
		}
		else
		{
			depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			depthDesc.Texture2DArray.ArraySize = desc.ArraySize;
			depthDesc.Texture2DArray.FirstArraySlice = 0;
			depthDesc.Texture2DArray.MipSlice = 0;
		}
		depthDesc.Flags = 0;
		hres = pWrapper->GetDevice()->CreateDepthStencilView( _texture, &depthDesc, &_depthStencilView );
		assert(SUCCEEDED(hres));
	}
	if(_miscFlags & HAGE::TEXTURE_GPU_WRITE)
	{
		D3D11_RENDER_TARGET_VIEW_DESC renderDesc;
		switch(desc.Format)
		{
		case DXGI_FORMAT_R32_TYPELESS:
			renderDesc.Format = DXGI_FORMAT_R32_FLOAT;
			break;
		case DXGI_FORMAT_R16_TYPELESS:
			renderDesc.Format = DXGI_FORMAT_R16_UNORM;
			break;
		default:
			renderDesc.Format = DXGI_FORMAT_UNKNOWN;
		}
		if(desc.ArraySize == 1)
		{
			renderDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderDesc.Texture2D.MipSlice = 0;
		}
		else
		{
			renderDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			renderDesc.Texture2DArray.ArraySize = desc.ArraySize;
			renderDesc.Texture2DArray.FirstArraySlice = 0;
			renderDesc.Texture2DArray.MipSlice = 0;
		}

		hres = pWrapper->GetDevice()->CreateRenderTargetView( _texture, &renderDesc, &_renderTargetView );
		assert(SUCCEEDED(hres));	
	}
}

void D3D11Texture::Clear(bool bDepth,float fDepth,bool bStencil,HAGE::u32 uStencil)
{
	assert(_depthStencilView);
	if(!bDepth && !bStencil)
		return;
	_pWrapper->GetContext()->ClearDepthStencilView( _depthStencilView, (bDepth?D3D11_CLEAR_DEPTH:0) | (bStencil?D3D11_CLEAR_STENCIL:0) , fDepth, uStencil );
}

void D3D11Texture::Clear(HAGE::Vector4<> vColor)
{
	assert(_renderTargetView);
    _pWrapper->GetContext()->ClearRenderTargetView( _renderTargetView, vColor.c );
}

D3D11Texture::~D3D11Texture()
{
	if(_pReadData)
	{
		delete [] _pReadData->pReadBuffer;
		_pReadData->textureStagingRead->Release();
		_pReadData->pReadOpQuery->Release();
		delete _pReadData;
	}
	if(_shaderResourceView)
		_shaderResourceView->Release();
	if(_renderTargetView)
		_renderTargetView->Release();
	if(_depthStencilView)
		_depthStencilView->Release();
	if(_texture)
		_texture->Release();
}

void D3D11Texture::GenerateMips()
{
	assert(_shaderResourceView);
	_pWrapper->GetContext()->GenerateMips(_shaderResourceView);
}

void D3D11Texture::StreamForReading(HAGE::u32 xOff,HAGE::u32 yOff,HAGE::u32 xSize,HAGE::u32 ySize)
{
	assert(_pReadData);
	assert(!_pReadData->bReadOpPending);

	D3D11_BOX box;
	box.top = yOff;
	box.bottom = yOff+ySize;
	box.left = xOff;
	box.right = xOff+xSize;
	box.front = 0;
	box.back = 1;

	_pWrapper->GetContext()->CopySubresourceRegion(_pReadData->textureStagingRead,0,xOff,yOff,0,_texture,0,&box);
	
	_pWrapper->GetContext()->End(_pReadData->pReadOpQuery);

	_pReadData->xReadOffset = xOff;
	_pReadData->yReadOffset = yOff;
	_pReadData->xReadSize = xSize;
	_pReadData->yReadSize = ySize;
	_pReadData->bReadOpPending= true;

	_pWrapper->QueueTextureForCompleteRead(this);
}

bool D3D11Texture::_CompleteReadingStream()
{
	assert(_pReadData);
	assert(_pReadData->bReadOpPending);

	BOOL val = FALSE;
	_pWrapper->GetContext()->GetData(_pReadData->pReadOpQuery,&val,sizeof(BOOL),D3D11_ASYNC_GETDATA_DONOTFLUSH);

	if(val)
	{
		_pReadData->bReadOpPending= false;
		D3D11_MAPPED_SUBRESOURCE	sub;
		_pWrapper->GetContext()->Map(_pReadData->textureStagingRead,0,D3D11_MAP_READ,0,&sub);

		for(int iy = _pReadData->yReadOffset; iy < _pReadData->yReadOffset + _pReadData->yReadSize; iy++)
			for(int ix = _pReadData->xReadOffset; ix < _pReadData->xReadOffset + _pReadData->xReadSize; ix++)
				for(int ip = 0; ip < _pReadData->pixelSize; ip++)
					((HAGE::u8*)_pReadData->pReadBuffer)[(iy*_pReadData->xReadSize +ix)*_pReadData->pixelSize + ip] = 
						((HAGE::u8*)sub.pData)[iy*sub.RowPitch + ix*_pReadData->pixelSize + ip];

		_pWrapper->GetContext()->Unmap(_pReadData->textureStagingRead,0);
		return true;
	}
	else
		return false;
}

void D3D11Texture::StreamFromSystem(const D3D11APIWrapper::CPUTextureWriteSettings& settings)
{
	assert(_miscFlags & HAGE::TEXTURE_CPU_WRITE);

	_pWrapper->GetContext()->CopySubresourceRegion(_texture,settings.level,settings.xOffset,settings.yOffset,0,settings.pSource,0,nullptr);
}

#undef PostMessage

//more like texture things
void D3D11APIWrapper::UpdateTexture(HAGE::APIWTexture* pTexture,HAGE::u32 xOff,HAGE::u32 yOff,HAGE::u32 xSize,HAGE::u32 ySize,HAGE::u32 Level, const void* pData)
{
	D3D11APIWrapper::CPUTextureWriteSettings settings;

	D3D11Texture* texture = (D3D11Texture*)pTexture;
	
	assert(texture->_miscFlags & HAGE::TEXTURE_CPU_WRITE);

	//create source texture
	D3D11_TEXTURE2D_DESC descCPUWrite = APIWToD3D11Texture2DDesc(xSize,ySize,1,texture->_format,texture->_miscFlags);;
	descCPUWrite.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	descCPUWrite.Usage = D3D11_USAGE_STAGING;
	descCPUWrite.BindFlags = 0;
	D3D11_SUBRESOURCE_DATA sub;
	sub.pSysMem = pData;
	sub.SysMemPitch = HAGE::APIWFormatImagePhysicalPitch(texture->_format,xSize);
	sub.SysMemSlicePitch = HAGE::APIWFormatImagePhysicalSize(texture->_format,xSize,ySize);
	
	EnterDeviceCritical();
	HRESULT hres = GetDevice()->CreateTexture2D(&descCPUWrite,pData?&sub:nullptr,&settings.pSource);
	LeaveDeviceCritical();
	if(SUCCEEDED(hres))
	{
		settings.pDest = texture;
		settings.level = Level;
		settings.xOffset = xOff;
		settings.yOffset = yOff;
		settings.xSize = xSize;
		settings.ySize = ySize;

		_allocQueue.PostMessage(MessageD3DRenderingCPUWriteRequest(settings));
	}
	else
		assert(!"could not create resource!");
}

bool D3D11APIWrapper::ReadTexture(HAGE::APIWTexture* pTexture, const void** ppDataOut)
{
	D3D11Texture* texture = (D3D11Texture*)pTexture;

	if(!texture->_pReadData)
		return false;

	if(texture->_pReadData->bReadOpPending)
		return false;

	*ppDataOut = texture->_pReadData->pReadBuffer;

	return true;
}
