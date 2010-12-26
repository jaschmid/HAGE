#include <HAGE.h>
#include "OpenGL3APIWrapper.h"

#ifndef NO_OGL

OGL3ConstantBuffer::OGL3ConstantBuffer(OpenGL3APIWrapper* pWrapper,HAGE::u32 nSize) : m_pWrapper(pWrapper),m_nSize(nSize)
{
	glGenBuffers(1,&m_cbo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_cbo);
    glBufferData(GL_UNIFORM_BUFFER, m_nSize,
                     NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

OGL3ConstantBuffer::~OGL3ConstantBuffer()
{
	glDeleteBuffers(1, &m_cbo);
}

void OGL3ConstantBuffer::UpdateContent(const void* pData)
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_cbo);
	glBufferData(GL_UNIFORM_BUFFER, m_nSize, pData, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

#endif