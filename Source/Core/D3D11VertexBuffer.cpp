#include <HAGE.h>
#include "D3D11APIWrapper.h"

#ifndef NO_D3D


D3D11VertexBuffer::D3D11VertexBuffer(D3D11APIWrapper* pWrapper,const char* szVertexFormat,const void* pData,HAGE::u32 nElements,bool bDynamic,bool bInstanceData) :
	m_pWrapper(pWrapper),
	m_pVertexBuffer(nullptr),
	m_nElements(nElements),
	m_bInstance(bInstanceData),
	m_nCode(pWrapper->GetVertexFormatCode(szVertexFormat))
{
	// Create the vertex buffer

    D3D11_BUFFER_DESC bd;
    bd.Usage = bDynamic?D3D11_USAGE_DYNAMIC:D3D11_USAGE_DEFAULT;
    bd.ByteWidth = m_pWrapper->GetVertexSize(m_nCode)*nElements;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = bDynamic?D3D11_CPU_ACCESS_WRITE:0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = pData;
	m_pWrapper->EnterDeviceCritical();
    HRESULT hr = m_pWrapper->GetDevice()->CreateBuffer( &bd, &initData, &m_pVertexBuffer );
	m_pWrapper->LeaveDeviceCritical();
	assert(SUCCEEDED(hr));
}

void D3D11VertexBuffer::UpdateContent(const void* pData)
{
	 D3D11_MAPPED_SUBRESOURCE pMapped;
	m_pWrapper->GetContext()->Map(m_pVertexBuffer,0,D3D11_MAP_WRITE_DISCARD,0,&pMapped);
	memcpy(pMapped.pData,pData,m_pWrapper->GetVertexSize(m_nCode)*m_nElements);
	m_pWrapper->GetContext()->Unmap(m_pVertexBuffer,0);
	
}

D3D11VertexBuffer::~D3D11VertexBuffer()
{
	if(m_pVertexBuffer)m_pVertexBuffer->Release();
}

#endif
