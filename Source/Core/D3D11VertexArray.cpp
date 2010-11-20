#include <HAGE.h>
#include "D3D11APIWrapper.h"

#ifndef NO_D3D


bool cmp (const D3D11VertexBuffer* p1,const D3D11VertexBuffer* p2)
{
	return p1->GetCode() < p2->GetCode();
}

D3D11VertexArray::D3D11VertexArray(D3D11APIWrapper* pWrapper,HAGE::u32 nPrimitives,HAGE::APIWPrimitiveType PrimitiveType,HAGE::APIWVertexBuffer** pBuffers,HAGE::u32 nBuffers,const HAGE::u32* pIndexBufferData) :
	m_pWrapper(pWrapper),
	m_nBuffers(nBuffers),
	m_PrimitiveType(PrimitiveType),
	m_nPrimitives(nPrimitives),
	m_pIndexBuffer(nullptr)
{
	// Create the vertex buffer
	for(unsigned int i=0;i<nBuffers;++i)
		m_VertexBuffers.push_back((D3D11VertexBuffer*)pBuffers[i]);

	std::sort(m_VertexBuffers.begin(),m_VertexBuffers.end(),cmp);

	for(auto i=m_VertexBuffers.begin();i!=m_VertexBuffers.end();++i)
		m_ArrayCode.push_back((*i)->GetCode());

	// make sure format exists
	m_pWrapper->GetArrayFormat(m_ArrayCode);

	if(pIndexBufferData)
	{
		D3D11_BUFFER_DESC bd;
		bd.Usage = D3D11_USAGE_DEFAULT;
		switch(PrimitiveType)
		{
		case HAGE::PRIMITIVE_POINTLIST:
			bd.ByteWidth = nPrimitives*sizeof(HAGE::u32);
			break;
		case HAGE::PRIMITIVE_LINELIST:
			bd.ByteWidth = nPrimitives*2*sizeof(HAGE::u32);
			break;
		case HAGE::PRIMITIVE_LINESTRIP:
			bd.ByteWidth = (nPrimitives+1)*sizeof(HAGE::u32);
			break;
		case HAGE::PRIMITIVE_TRIANGLELIST:
			bd.ByteWidth = (nPrimitives*3)*sizeof(HAGE::u32);
			break;
		case HAGE::PRIMITIVE_TRIANGLESTRIP:
			bd.ByteWidth = (nPrimitives+2)*sizeof(HAGE::u32);
			break;
		}
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = pIndexBufferData;
		HRESULT hr = m_pWrapper->GetDevice()->CreateBuffer( &bd, &initData, &m_pIndexBuffer );
		assert(SUCCEEDED(hr));
	}
	else
		m_pIndexBuffer = 0;
}

D3D11VertexArray::~D3D11VertexArray()
{
	if(m_pIndexBuffer)
		m_pIndexBuffer->Release();
}

#endif
