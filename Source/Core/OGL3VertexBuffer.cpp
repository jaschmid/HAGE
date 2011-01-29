#include <HAGE.h>
#include "OpenGL3APIWrapper.h"

#ifndef NO_OGL

OGL3VertexBuffer::OGL3VertexBuffer(OpenGL3APIWrapper* pWrapper,const char* szVertexFormat,const void* pData,HAGE::u32 nElements,bool bDynamic,bool bInstanceData) : m_pWrapper(pWrapper)
{
	m_code = pWrapper->GetVertexFormatCode(szVertexFormat);
	m_BufferSize=nElements*pWrapper->GetVertexSize(m_code);

	glGenBuffers(1, &m_vboID);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
	glBufferData(GL_ARRAY_BUFFER, m_BufferSize, pData, bDynamic?GL_DYNAMIC_DRAW:GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glError();
}

void OGL3VertexBuffer::UpdateContent(const void* pData)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
	glBufferData(GL_ARRAY_BUFFER, m_BufferSize, pData, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glError();
}

OGL3VertexBuffer::~OGL3VertexBuffer()
{
	glDeleteBuffers(1, &m_vboID);
}

#endif