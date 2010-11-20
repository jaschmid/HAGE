#include <HAGE.h>
#include "OpenGL3APIWrapper.h"

#ifndef NO_OGL

OGL3VertexBuffer::OGL3VertexBuffer(OpenGL3APIWrapper* pWrapper,const char* szVertexFormat,void* pData,HAGE::u32 nElements,bool bInstanceData) : m_pWrapper(pWrapper)
{
	m_code = pWrapper->GetVertexFormatCode(szVertexFormat);

	glGenBuffers(1, &m_vboID);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
	glBufferData(GL_ARRAY_BUFFER, nElements*pWrapper->GetVertexSize(m_code), pData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

OGL3VertexBuffer::~OGL3VertexBuffer()
{
	glDeleteBuffers(1, &m_vboID);
}

#endif