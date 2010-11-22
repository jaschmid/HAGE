#include <HAGE.h>
#include "OpenGL3APIWrapper.h"

#ifndef NO_OGL

OGL3Effect::OGL3Effect(OpenGL3APIWrapper* pWrapper,const char* pVertexProgram,const char* pFragmentProgram,HAGE::u16 rasterizer,HAGE::u16 blend) :m_pWrapper(pWrapper),m_BlendState(blend),m_RastState(rasterizer)
{
	m_CgVertexProgram =
	cgCreateProgram(
		m_pWrapper->GetCGC(),              /* Cg runtime context */
		CG_SOURCE,                /* Program in human-readable form */
		pVertexProgram,				/* Name of file containing program */
		m_pWrapper->GetVertexProfile(),        /* Profile: OpenGL ARB vertex program */
		"vertex",			/* Entry function name */
		NULL);                    /* No extra compiler options */
	m_pWrapper->checkForCgError("creating vertex program from source");
	cgGLLoadProgram(m_CgVertexProgram);
	m_pWrapper->checkForCgError("loading vertex program");
	
	m_CgFragmentProgram =
	cgCreateProgram(
		m_pWrapper->GetCGC(),                /* Cg runtime context */
		CG_SOURCE,                  /* Program in human-readable form */
		pFragmentProgram,			/* Name of file containing program */
		m_pWrapper->GetFragmentProfile(),        /* Profile: OpenGL ARB vertex program */
		"fragment",		/* Entry function name */
		NULL);                      /* No extra compiler options */
	m_pWrapper->checkForCgError("creating fragment program from source");
	cgGLLoadProgram(m_CgFragmentProgram);
	m_pWrapper->checkForCgError("loading fragment program");
}

OGL3Effect::~OGL3Effect()
{
	cgDestroyProgram( m_CgVertexProgram );
    m_pWrapper->checkForCgError( "destroying vertex program" );
    cgDestroyProgram( m_CgFragmentProgram );
    m_pWrapper->checkForCgError( "destroying fragment program" );
}

void OGL3Effect::Draw(HAGE::APIWVertexArray* pArrayPre,HAGE::APIWConstantBuffer** _pConstants,HAGE::u32 nConstants)
{
	OGL3VertexArray* pArray = (OGL3VertexArray*)pArrayPre;
	OGL3ConstantBuffer** ppConstants = (OGL3ConstantBuffer**)_pConstants;
	
	assert(nConstants<=N_CBUFFERS);
	
	cgGLBindProgram(m_CgVertexProgram);
	
	cgGLBindProgram(m_CgFragmentProgram);

	// set cBuffers

	for(HAGE::u32 i =0; i<nConstants;++i)
	{
		cgSetProgramBuffer(m_CgVertexProgram,i,ppConstants[i]->m_Buffer);
	}
	//m_pWrapper->SetCBuffer(nBuffer,this);

	// set states

	m_pWrapper->SetBlendState(m_BlendState);
	m_pWrapper->SetRasterizerState(m_RastState);
	
	glBindVertexArray(pArray->m_vaoID);	

	HAGE::u32 nItems;
	GLenum PrimitiveType;

	switch(pArray->m_PrimitiveType)
	{
	case HAGE::PRIMITIVE_POINTLIST:
		PrimitiveType = GL_POINTS;
		nItems = pArray->m_nPrimitives;
		break;
	case HAGE::PRIMITIVE_LINELIST:
		PrimitiveType = GL_LINES;
		nItems = pArray->m_nPrimitives*2;
		break;
	case HAGE::PRIMITIVE_LINESTRIP:
		PrimitiveType = GL_LINE_STRIP;
		nItems = pArray->m_nPrimitives-1;
		break;
	case HAGE::PRIMITIVE_TRIANGLELIST:
		PrimitiveType = GL_TRIANGLES;
		nItems = pArray->m_nPrimitives*3;
		break;
	case HAGE::PRIMITIVE_TRIANGLESTRIP:
		PrimitiveType = GL_TRIANGLE_STRIP;
		nItems = pArray->m_nPrimitives-2;
		break;
	}

	if(pArray->m_vboIndexID)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pArray->m_vboIndexID);
		glDrawElements(PrimitiveType, nItems , GL_UNSIGNED_INT,0);

	}
	else
	{
		glDrawArrays(PrimitiveType, 0, nItems);	
	}

}

#endif NO_OGL