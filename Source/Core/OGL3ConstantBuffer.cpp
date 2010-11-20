#include <HAGE.h>
#include "OpenGL3APIWrapper.h"

#ifndef NO_OGL

OGL3ConstantBuffer::OGL3ConstantBuffer(OpenGL3APIWrapper* pWrapper,HAGE::u32 nSize) : m_pWrapper(pWrapper),m_nSize(nSize)
{
	m_Buffer = cgCreateBuffer(pWrapper->GetCGC(),m_nSize,NULL,CG_BUFFER_USAGE_DYNAMIC_DRAW);
}

OGL3ConstantBuffer::~OGL3ConstantBuffer()
{
	cgDestroyBuffer(m_Buffer);
}

void OGL3ConstantBuffer::Set(HAGE::u32 nBuffer)
{
	assert(nBuffer<=16);
	m_pWrapper->SetCBuffer((HAGE::u8)nBuffer,this);
}

void OGL3ConstantBuffer::UpdateContent(const void* pData)
{
	cgSetBufferData(m_Buffer,m_nSize,pData);
}

#endif