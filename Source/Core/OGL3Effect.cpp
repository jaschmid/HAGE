#include <HAGE.h>
#include "OpenGL3APIWrapper.h"

#ifndef NO_OGL

CGprogram OGL3Effect::testProgram = 0;

void CheckShader(GLuint id, GLuint type, GLint *ret, const char *onfail)
{
 //Check if something is wrong with the shader
 switch(type) {
 case(GL_COMPILE_STATUS):
   glGetShaderiv(id, type, ret);
   if(*ret == false){
    int infologLength =  0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infologLength);
    GLchar* buffer= new char[infologLength];
    GLsizei charsWritten = 0;
    std::cout << onfail << std::endl;
    glGetShaderInfoLog(id, infologLength, &charsWritten, buffer);
    std::cout << buffer << std::endl;
   }
   break;
 case(GL_LINK_STATUS):
   glGetProgramiv(id, type, ret);
   if(*ret == false){
    int infologLength =  0;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infologLength);
    GLchar* buffer= new char[infologLength];
    GLsizei charsWritten = 0;
    std::cout << onfail << std::endl;
    glGetProgramInfoLog(id, infologLength, &charsWritten, buffer);
    std::cout << buffer << std::endl;
   }
   break;
 default:
   break;
 };
}

static const char* extStr = "#extension GL_ARB_uniform_buffer_object : enable\n";

OGL3Effect::OGL3Effect(OpenGL3APIWrapper* pWrapper,const char* pVertexProgram,const char* pFragmentProgram,HAGE::u16 rasterizer,HAGE::u16 blend) 
	:m_pWrapper(pWrapper),m_BlendState(blend),m_RastState(rasterizer),_cVertexShader(new char[strlen(pVertexProgram)+1]),_cPixelShader(new char[strlen(pFragmentProgram)+1]),
	_bInit(false)
{
/*	strcpy(_cVertexShader,extStr);
	strcpy(_cPixelShader,extStr);*/
	strcpy(_cVertexShader,pVertexProgram);
	strcpy(_cPixelShader,pFragmentProgram);

	const char* argsV[] = {NULL};
	const char* argsF[] = {NULL};

	m_CgVertexProgram =
	cgCreateProgram(
		m_pWrapper->GetCGC(),              /* Cg runtime context */
		CG_SOURCE,                /* Program in human-readable form */
		_cVertexShader,				/* Name of file containing program */
		pWrapper->GetVertexProfile(),        /* Profile: OpenGL ARB vertex program */
		"vertex",			/* Entry function name */
		argsV);                    /* No extra compiler options */
	m_pWrapper->checkForCgError("creating vertex program from source");
	
	m_CgFragmentProgram =
	cgCreateProgram(
		m_pWrapper->GetCGC(),                /* Cg runtime context */
		CG_SOURCE,                  /* Program in human-readable form */
		_cPixelShader,			/* Name of file containing program */
		pWrapper->GetFragmentProfile(),        /* Profile: OpenGL ARB vertex program */
		"fragment",		/* Entry function name */
		argsF);                      /* No extra compiler options */
	m_pWrapper->checkForCgError("creating fragment program from source");
	
	cgGLLoadProgram(m_CgVertexProgram);
	m_pWrapper->checkForCgError("loading vertex program");
	cgGLLoadProgram(m_CgFragmentProgram);
	m_pWrapper->checkForCgError("loading fragment program");

	if(testProgram == 0)
	{
		testProgram = m_CgVertexProgram;
	}

	/*
	cgCompileProgram(m_CgVertexProgram);
    m_pWrapper->checkForCgError("compiling vertex program");
	cgCompileProgram(m_CgFragmentProgram);
    m_pWrapper->checkForCgError("compiling fragment program");
	
	const char* hlsl_source_v[] = {cgGetProgramString( m_CgVertexProgram, CG_COMPILED_PROGRAM)};
	const char* hlsl_source_f[] = {cgGetProgramString( m_CgFragmentProgram, CG_COMPILED_PROGRAM)};


	_glVShader =  glCreateShader(GL_VERTEX_SHADER);
	glError();
	_glFShader =  glCreateShader(GL_FRAGMENT_SHADER);
	glError();
	glShaderSource(	_glVShader,1,hlsl_source_v,NULL);
	glError();
	glShaderSource(	_glFShader,1,hlsl_source_f,NULL);
	glError();

	_glProgram = glCreateProgram();
	glError();
	glAttachShader(_glProgram,_glVShader);
	glError();
	glAttachShader(_glProgram,_glFShader);
	glError();
	glCompileShader(_glVShader);
	int bCompile;
	CheckShader(_glVShader,GL_COMPILE_STATUS,&bCompile,"compile VS");
	glError();
	glCompileShader(_glFShader);
	CheckShader(_glFShader,GL_COMPILE_STATUS,&bCompile,"compile FS");
	glError();
	glLinkProgram(_glProgram);
	CheckShader(_glProgram,GL_LINK_STATUS,&bCompile,"link prog");
	glError();
	
	glValidateProgram(_glProgram);
	int bValid,nLength,bLink;
	glGetProgramiv(_glProgram,GL_LINK_STATUS,&bLink);
	glGetProgramiv(_glProgram,GL_VALIDATE_STATUS,&bValid);
	glGetProgramiv(_glProgram,GL_INFO_LOG_LENGTH,&nLength);
	char* info=new char[nLength];
	glGetProgramInfoLog(_glProgram,nLength,&nLength,info);
	glUseProgram(_glProgram);
	glError();
	glUseProgram(0);
	glError();
	bool program = glIsProgram(_glProgram);
	bool vertex = glIsShader(_glVShader);
	bool fragment = glIsShader(_glFShader);

	int nAttribs;
	int nAttribsLength;
	glGetProgramiv(_glProgram,GL_ACTIVE_ATTRIBUTES,&nAttribs);
	glGetProgramiv(_glProgram,GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,&nAttribsLength);
	char* cAttribName = new char[nAttribsLength];
	for(int i =0;i<nAttribs;++i)
	{
		int len;
		int size;
		GLenum type;
		glGetActiveAttrib(_glProgram,i,nAttribsLength,&len,&size,&type,cAttribName);
		printf("Attrib %i  \@ %i= %s\n",i,glGetAttribLocation(_glProgram,cAttribName),cAttribName);
	}
	glGetProgramiv(_glProgram,GL_ACTIVE_UNIFORM_BLOCKS,&nAttribs);
	glGetProgramiv(_glProgram,GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH,&nAttribsLength);
	cAttribName = new char[nAttribsLength];
	for(int i =0;i<nAttribs;++i)
	{
		int len;
		int buflen;
		glGetActiveUniformBlockiv(_glProgram,i,GL_UNIFORM_BLOCK_BINDING,&len);
		glGetActiveUniformBlockName(_glProgram,i,nAttribsLength,&buflen,cAttribName);
		printf("Uniform Block %i \@ %i= %s\n",i,len,	cAttribName);
	}
	glGetProgramiv(_glProgram,GL_ACTIVE_UNIFORMS,&nAttribs);
	glGetProgramiv(_glProgram,GL_ACTIVE_UNIFORM_MAX_LENGTH,&nAttribsLength);
	cAttribName = new char[nAttribsLength];
	for(int i =0;i<nAttribs;++i)
	{
		int len;
		int size;
		GLenum type;
		glGetActiveUniform(_glProgram,i,nAttribsLength,&len,&size,&type,cAttribName);
		printf("Uniform %i \@ %i= %s, size %i, type %i\n",i,glGetUniformLocation(_glProgram,cAttribName),	cAttribName,size,type);
	}

	printf("Created %08x in context %08x\n",this,wglGetCurrentContext());*/
}

OGL3Effect::~OGL3Effect()
{
	cgDestroyProgram( m_CgVertexProgram );
    m_pWrapper->checkForCgError( "destroying vertex program" );
    cgDestroyProgram( m_CgFragmentProgram );
    m_pWrapper->checkForCgError( "destroying fragment program" );
	delete _cVertexShader;
	delete _cPixelShader;
}

void OGL3Effect::Draw(HAGE::APIWVertexArray* pArrayPre,HAGE::APIWConstantBuffer* const * _pConstants,HAGE::u32 nConstants)
{
	//printf("Rendering %08x in context %08x\n",this,wglGetCurrentContext());
	OGL3VertexArray* pArray = (OGL3VertexArray*)pArrayPre;
	OGL3ConstantBuffer** ppConstants = (OGL3ConstantBuffer**)_pConstants;

	assert(nConstants<=N_CBUFFERS);
	
	if(!_bInit)
	{
		_bInit=true;
		cgGLLoadProgram(m_CgVertexProgram);
		m_pWrapper->checkForCgError("loading vertex program");
		cgGLLoadProgram(m_CgFragmentProgram);
		m_pWrapper->checkForCgError("loading fragment program");
	}

	cgGLBindProgram(m_CgVertexProgram);
    m_pWrapper->checkForCgError( "binding vertex program" );
	
	cgGLBindProgram(m_CgFragmentProgram);
    m_pWrapper->checkForCgError( "binding fragment program" );
	/*
	bool program = glIsProgram(_glProgram);
	bool vertex = glIsShader(_glVShader);
	bool fragment = glIsShader(_glFShader);
	glUseProgram(_glProgram);
	glError();

	// set cBuffers
	*/
	for(HAGE::u32 i =0; i<nConstants;++i)
	{
		/*glBindBufferBase(GL_UNIFORM_BUFFER,i,cgGLGetBufferObject(ppConstants[i]->m_Buffer));
		glError();*/
		cgSetProgramBuffer(m_CgVertexProgram,i,ppConstants[i]->m_Buffer);
		m_pWrapper->checkForCgError( "setting program buffer" );
	}
	//m_pWrapper->SetCBuffer(nBuffer,this);

	// set states

	m_pWrapper->SetBlendState(m_BlendState);
	m_pWrapper->SetRasterizerState(m_RastState);

	pArray->Init();
	glBindVertexArray(pArray->m_vaoID);	
	glError();

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
		glError();
		glDrawElements(PrimitiveType, nItems , GL_UNSIGNED_INT,0);
		glError();
	}
	else
	{
		glDrawArrays(PrimitiveType, 0, nItems);	
		glError();
	}

}

#endif NO_OGL