#include <HAGE.h>
#include "D3D11APIWrapper.h"

#ifndef NO_D3D

D3D11ConstantBuffer::D3D11ConstantBuffer(D3D11APIWrapper* pWrapper,HAGE::u32 nSize) : m_pWrapper(pWrapper),m_nSize(nSize),m_pBuffer(nullptr)
{
	D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = m_nSize;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    HRESULT hr = m_pWrapper->GetDevice()->CreateBuffer( &bd, NULL, &m_pBuffer );
}

D3D11ConstantBuffer::~D3D11ConstantBuffer()
{
	if(m_pBuffer)
		m_pBuffer->Release();
}

void D3D11ConstantBuffer::UpdateContent(const void* pData)
{
	m_pWrapper->GetContext()->UpdateSubresource(m_pBuffer,0,NULL,pData,0,0);
}

void D3D11ConstantBuffer::Set(HAGE::u32 nSlot)
{
	m_pWrapper->GetContext()->VSSetConstantBuffers(nSlot,1,&m_pBuffer);
}
#endif
