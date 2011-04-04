#include "Hage.h"
#include "D3D11APIWrapper.h"

D3D11Texture::D3D11Texture(D3D11APIWrapper* pWrapper,HAGE::u32 xSize, HAGE::u32 ySize, HAGE::u32 mipLevels, HAGE::APIWFormat format,HAGE::u32 miscFlags,const void* pData)
	: _pWrapper(pWrapper),_xSize(xSize),_ySize(ySize),_mipLevels(mipLevels),_format(format),_miscFlags(miscFlags),_texture(nullptr),_shaderResourceView(nullptr),_renderTargetView(nullptr),_depthStencilView(nullptr)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.ArraySize = 1;
	desc.Format = APIWFormatToD3DFormat(_format);
	desc.Height = ySize;
	desc.Width = xSize;
	desc.MipLevels = mipLevels;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	if(_miscFlags & HAGE::TEXTURE_CPU_WRITE)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC ;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if(_miscFlags & HAGE::TEXTURE_CPU_READ)
	{
		assert(!"Currently don't support staging resources");
	}
	else if(!(_miscFlags & HAGE::TEXTURE_GPU_WRITE || _miscFlags & HAGE::TEXTURE_GPU_DEPTH_STENCIL))
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

	if(!(_miscFlags & HAGE::TEXTURE_GPU_NO_READ))
	{
		desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	}
	if(_miscFlags & HAGE::TEXTURE_GPU_WRITE)
	{
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	if(_miscFlags & HAGE::TEXTURE_GPU_DEPTH_STENCIL)
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

	if(_miscFlags & HAGE::TEXTURE_CUBE )
	{
		desc.ArraySize = 6;
		desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE ;
	}

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
			data[i].SysMemPitch = APIWFormatImagePhysicalPitch(format,w);
			data[i].SysMemSlicePitch = APIWFormatImagePhysicalSize(format,w,h);

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
	HRESULT hres = pWrapper->GetDevice()->CreateTexture2D(&desc,pLocalData,&_texture);
	assert(SUCCEEDED(hres));

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
}

void D3D11Texture::GenerateMips()
{
}