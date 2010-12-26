#include <HAGE.h>
#include "OpenGL3APIWrapper.h"

#ifndef NO_OGL

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

static const char* extStr = "#version 150\n#extension GL_ARB_uniform_buffer_object : enable\nlayout(std140) uniform;\n";

char *replace_str(const char *str, const char *orig, const char *rep, char* buffer)
{
  const char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return strcpy(buffer,str);

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

OGL3Effect::OGL3Effect(OpenGL3APIWrapper* pWrapper,const char* pVertexProgram,const char* pFragmentProgram,HAGE::u16 rasterizer,HAGE::u16 blend) 
	:m_pWrapper(pWrapper),m_BlendState(blend),m_RastState(rasterizer),_cVertexShader(new char[strlen(pVertexProgram)+1]),_cPixelShader(new char[strlen(pVertexProgram)+1])
{
/*	strcpy(_cVertexShader,extStr);
	strcpy(_cPixelShader,extStr);*/
	strcpy(_cVertexShader,pVertexProgram);
	strcpy(_cPixelShader,pVertexProgram);

	const char* argsV[] = {"version=150",NULL};
	const char* argsF[] = {"version=150",NULL};
	
	CGprogram					m_CgProgramParts[3];

	m_CgProgramParts[0] =
	cgCreateProgram(
		m_pWrapper->GetCGC(),              /* Cg runtime context */
		CG_SOURCE,                /* Program in human-readable form */
		_cVertexShader,				/* program */
		pWrapper->GetVertexProfile(),        /* Profile */
		"vertex",			/* Entry function name */
		argsV);                    /* extra compiler options */
	m_pWrapper->checkForCgError("creating vertex program from source");
	
	m_CgProgramParts[1] =
	cgCreateProgram(
		m_pWrapper->GetCGC(),                /* Cg runtime context */
		CG_SOURCE,                  /* Program in human-readable form */
		_cPixelShader,			/* program */
		pWrapper->GetFragmentProfile(),        /* Profile */
		"fragment",		/* Entry function name */
		argsF);                      /*  extra compiler options */
	m_pWrapper->checkForCgError("creating fragment program from source");
	/*
	cgGLLoadProgram(m_CgVertexProgram);
	m_pWrapper->checkForCgError("loading vertex program");
	cgGLLoadProgram(m_CgFragmentProgram);
	m_pWrapper->checkForCgError("loading fragment program");

	if(testProgram == 0)
	{
		testProgram = m_CgVertexProgram;
	}*/
/*	cgCompileProgram(m_CgProgramParts[0]);
    m_pWrapper->checkForCgError("compiling program");
	cgCompileProgram(m_CgProgramParts[1]);
    m_pWrapper->checkForCgError("compiling program");*/
	CGprogram m_CgMergedProgram = cgCombinePrograms(2,m_CgProgramParts);

	cgDestroyProgram(m_CgProgramParts[0]);
	cgDestroyProgram(m_CgProgramParts[1]);

	cgCompileProgram(m_CgMergedProgram);
	m_pWrapper->checkForCgError("compiling program");

	const char* v_string;
	const char* f_string;

	for(int i =0;i<cgGetNumProgramDomains(m_CgMergedProgram);++i)
	{
		CGprogram subprog = cgGetProgramDomainProgram(m_CgMergedProgram, i);
		CGdomain domain = cgGetProgramDomain(subprog);

		switch(domain)
		{
		case CG_VERTEX_DOMAIN:
			v_string = cgGetProgramString( subprog, CG_COMPILED_PROGRAM);
			break;
		case CG_FRAGMENT_DOMAIN:
			f_string = cgGetProgramString( subprog, CG_COMPILED_PROGRAM);
			break;
		case CG_GEOMETRY_DOMAIN:
			break;
		}
	}
	

	const char* version_string = "#version 150\n";

	int v_skip = strstr(v_string,version_string)+strlen(version_string)-v_string;
	int f_skip = strstr(f_string,version_string)+strlen(version_string)-f_string;

	const char* v_skipped = &v_string[v_skip];
	const char* f_skipped = &f_string[f_skip];

	//replace struct qualifier on uniform structs with block, like they should be!
	_glVertexShader = new char[strlen(v_skipped)*2];
	_glFragmentShader = new char[strlen(f_skipped)*2];

	replace_str(v_skipped,"uniform struct","layout(std140) uniform block",_glVertexShader);
	replace_str(f_skipped,"uniform struct","layout(std140) uniform block",_glFragmentShader);
	
	const char* hlsl_source_v[] = {extStr,_glVertexShader};
	const char* hlsl_source_f[] = {extStr,_glFragmentShader};

	_glVShader =  glCreateShader(GL_VERTEX_SHADER);
	glError();
	_glFShader =  glCreateShader(GL_FRAGMENT_SHADER);
	glError();
	glShaderSource(	_glVShader,2,hlsl_source_v,NULL);
	glError();
	glShaderSource(	_glFShader,2,hlsl_source_f,NULL);
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
	glBindFragDataLocationEXT(_glProgram,0,"COL0");
	glLinkProgram(_glProgram);
	CheckShader(_glProgram,GL_LINK_STATUS,&bCompile,"link prog");
	glError();
	
	glValidateProgram(_glProgram);
	int bValid,nLength,bLink;
	glGetProgramiv(_glProgram,GL_LINK_STATUS,&bLink);
	glGetProgramiv(_glProgram,GL_VALIDATE_STATUS,&bValid);
	glGetProgramiv(_glProgram,GL_INFO_LOG_LENGTH,&nLength);
	if(nLength)
	{
		char* info=new char[nLength];
		glGetProgramInfoLog(_glProgram,nLength,&nLength,info);
	}
	/*
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
		glUniformBlockBinding(_glProgram,glGetUniformBlockIndex(_glProgram,cAttribName),i);
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
	
	cgDestroyProgram(m_CgMergedProgram);
}

OGL3Effect::~OGL3Effect()
{
    m_pWrapper->checkForCgError( "destroying program" );
	delete _cVertexShader;
	delete _cPixelShader;
}

void OGL3Effect::Draw(HAGE::APIWVertexArray* pArrayPre,HAGE::APIWConstantBuffer* const * _pConstants,HAGE::u32 nConstants)
{
	//printf("Rendering %08x in context %08x\n",this,wglGetCurrentContext());
	OGL3VertexArray* pArray = (OGL3VertexArray*)pArrayPre;
	OGL3ConstantBuffer** ppConstants = (OGL3ConstantBuffer**)_pConstants;

	assert(nConstants<=N_CBUFFERS);
	
	glUseProgram(_glProgram);
	glError();

	// set cBuffers
	
	for(HAGE::u32 i =0; i<nConstants;++i)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER,i,ppConstants[i]->m_cbo);
		glUniformBlockBinding(_glProgram, i, i);
		glError();
	}

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