#include <HAGE.h>
#include "D3D11APIWrapper.h"

#ifndef NO_D3D


D3D11VertexBuffer::D3D11VertexBuffer(D3D11APIWrapper* pWrapper,const char* szVertexFormat,void* pData,HAGE::u32 nElements,bool bInstanceData) :
	m_pWrapper(pWrapper),
	m_pVertexBuffer(nullptr),
	m_nElements(nElements),
	m_bInstance(bInstanceData),
	m_nCode(pWrapper->GetVertexFormatCode(szVertexFormat))
{
	// Create the vertex buffer

    D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = m_pWrapper->GetVertexSize(m_nCode)*nElements;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = pData;
    HRESULT hr = m_pWrapper->GetDevice()->CreateBuffer( &bd, &initData, &m_pVertexBuffer );
	assert(SUCCEEDED(hr));
}

D3D11VertexBuffer::~D3D11VertexBuffer()
{
	if(m_pVertexBuffer)m_pVertexBuffer->Release();
}

#endif
