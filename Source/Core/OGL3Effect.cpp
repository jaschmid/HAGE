#include <HAGE.h>
#include "OpenGL3APIWrapper.h"

#ifndef NO_OGL

OGL3Effect::OGL3Effect(OpenGL3APIWrapper* pWrapper,const char* pVertexProgram,const char* pFragmentProgram) :m_pWrapper(pWrapper)
{
	m_CgVertexProgram =
	cgCreateProgram(
		m_pWrapper->GetCGC(),              /* Cg runtime context */
		CG_SOURCE,                /* Program in human-readable form */
		pVertexProgram,				/* Name of file containing program */
		m_pWrapper->GetVertexProfile(),        /* Profile: OpenGL ARB vertex program */
		"C2E1v_green",			/* Entry function name */
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
		"C2E2f_passthru",		/* Entry function name */
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

void OGL3Effect::Draw(HAGE::APIWVertexArray* pArrayPre)
{
	OGL3VertexArray* pArray = (OGL3VertexArray*)pArrayPre;

	cgGLBindProgram(m_CgVertexProgram);
	m_pWrapper->checkForCgError("binding vertex program");

	cgGLEnableProfile(m_pWrapper->GetVertexProfile());
	m_pWrapper->checkForCgError("enabling vertex profile");

	cgGLBindProgram(m_CgFragmentProgram);
	m_pWrapper->checkForCgError("binding fragment program");

	cgGLEnableProfile(m_pWrapper->GetFragmentProfile());
	m_pWrapper->checkForCgError("enabling fragment profile");

	// set cBuffers

	for(int i =0; i<N_CBUFFERS;++i)
	{
		OGL3ConstantBuffer* c=m_pWrapper->GetCBuffer(i);
		if(c)
			cgSetProgramBuffer(m_CgVertexProgram,i,c->m_Buffer);
	}
	//m_pWrapper->SetCBuffer(nBuffer,this);
	
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
		GLenum error;
		while((error = glGetError()) != GL_NO_ERROR)
		{
			printf("GlError: %08x\n",error);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}
	else
	{
		glDrawArrays(PrimitiveType, 0, nItems);	
	}

	glBindVertexArray(0);


	cgGLDisableProfile(m_pWrapper->GetVertexProfile());
	m_pWrapper->checkForCgError("disabling vertex profile");

	cgGLDisableProfile(m_pWrapper->GetFragmentProfile());
	m_pWrapper->checkForCgError("disabling fragment profile");
}

#endif NO_OGL