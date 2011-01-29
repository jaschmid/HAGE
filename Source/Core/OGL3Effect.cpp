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

static const char* extStr = "#version 400\nlayout(std140) uniform;\n#define _H_GLSL\n";
static const char* vsStr = "#define _V_SHADER\n";
static const char* gsStr = "#define _G_SHADER\n";
static const char* fsStr = "#define _F_SHADER\n";
static const char* preproc = 
	"#ifdef _H_GLSL\n"
	"	#define H_CONSTANT_BUFFER_BEGIN(x) layout(std140) uniform _##x {\n"
	"	#define H_CONSTANT_BUFFER_END };\n"
	"	#ifdef _V_SHADER\n"
	"		#define VERTEX_SHADER void main() \n"
	"		#define GEOMETRY_SHADER(x,y,z) void _geometry_unused_vertex() \n"
	"		#define FRAGMENT_SHADER void _fragment_unused_vertex() \n"
	"	#elif defined(_G_SHADER)\n"
	"		#define VERTEX_SHADER void _vertex_unused_geometry() \n"
	"		#define GEOMETRY_SHADER(x,y,z) layout(x) in; layout(y,max_vertices = z) out; void main()\n"
	"		#define FRAGMENT_SHADER void _fragment_unused_geometry() \n"
	"	#else // _F_SHADER\n"
	"		#define VERTEX_SHADER void _vertex_unused_fragment() \n"
	"		#define GEOMETRY_SHADER(x,y,z) void _geometry_unused_fragment() \n"
	"		#define FRAGMENT_SHADER void main() \n"
	"	#endif\n"
	"	#ifdef _V_SHADER\n"
	"		#define DECL_VS_IN(x,y) in x _##y\n"
	"		#define DECL_VS_OUT(x,y) out x y\n"
	"		#define VS_OUT(x) x\n"
	"		#define VS_IN(x) _##x\n"
	"		#define VS_OUT_POSITION (gl_Position)\n"
	"	#else\n"
	"		#define DECL_VS_IN(x,y) x _VS_IN##y\n"
	"		#define DECL_VS_OUT(x,y) x _VS_OUT##y\n"
	"		#define VS_OUT(x) _VS_OUT##x\n"
	"		#define VS_IN(x) _VS_IN##x\n"
	"		#define VS_OUT_POSITION (_VS_OUT##_position)\n"
	"	#endif\n"
	"	#ifdef _F_SHADER\n"
	"		#define DECL_FS_IN(x,y) in x y\n"
	"		#define FS_IN(x) x\n"
	"		#define FS_OUT_COLOR (gl_FragColor)\n"
	"		#define DECL_FS_COLOR float4 gl_FragColor\n"
	"		#define H_TEXTURE_2D(x) uniform sampler2D x\n"
	"		#define H_TEXTURE_CUBE(x) uniform samplerCube x\n"
	"		#define H_SAMPLE_2D(sampler,coord) (texture(sampler,float2((coord).x,1.0-(coord).y)))\n"
	"		#define H_SAMPLE_CUBE(sampler,coord) (texture(sampler,float3((coord).x,(coord).y,(coord).z)))\n"
	"	#else\n"
	"		#define DECL_FS_IN(x,y) x _FS_IN##y\n"
	"		#define FS_IN(x) _FS_IN##x\n"
	"		#define FS_OUT_COLOR (_FS_OUT##_target)\n"
	"		#define DECL_FS_COLOR float4 _FS_OUT##_target\n"
	"		#define H_TEXTURE_2D(x) \n"
	"		#define H_TEXTURE_CUBE(x) \n"
	"		#define H_SAMPLE_2D(x,y) (vec4(0.0,0.0,0.0,0.0))\n"
	"		#define H_SAMPLE_CUBE(x,y) (vec4(0.0,0.0,0.0,0.0))\n"
	"	#endif\n"
	"	#ifdef _G_SHADER\n"
	"		#define GS_IN_POSITION(i) (gl_in[(i)].gl_Position)\n"
	"		#define GS_IN(x,i) (x[(i)])\n"
	"		#define GS_OUT(x) (x)\n"
	"		#define GS_OUT_POSITION (gl_Position)\n"
	"		#define GS_OUT_LAYER (gl_Layer)\n"
	"		#define DECL_GS_IN(x,y) in x y[]\n"
	"		#define DECL_GS_OUT(x,y) out x y\n"
	"		#define DECL_GS_IN_POSITION\n"
	"		#define DECL_GS_OUT_POSITION\n"
	"		#define DECL_GS_OUT_LAYER\n"
	"		#define GS_END_VERTEX EmitVertex()\n"
	"		#define GS_END_PRIMITIVE EndPrimitive()\n"
	"	#else\n"
	"		#define GS_IN_POSITION(i) (vec4(0.0,0.0,0.0,0.0))\n"
	"		#define GS_IN(x,i) (_GS_IN##x)\n"
	"		#define GS_OUT(x) (_GS_OUT##x)\n"
	"		#define GS_OUT_POSITION (_GS_OUT##_target)\n"
	"		#define GS_OUT_LAYER (_GS_OUT##_layer)\n"
	"		#define DECL_GS_IN(x,y) x _GS_IN##y\n"
	"		#define DECL_GS_OUT(x,y) x _GS_OUT##y\n"
	"		#define DECL_GS_IN_POSITION\n"
	"		#define DECL_GS_OUT_POSITION float4 _GS_OUT##_target\n"
	"		#define DECL_GS_OUT_LAYER int _GS_OUT##_layer\n"
	"		#define GS_END_VERTEX\n"
	"		#define GS_END_PRIMITIVE\n"
	"	#endif\n"
	"	#define GS_INIT_OUT float gs_out= 0.0\n"
	"	#define VS_IN_BEGIN\n"
	"	#define VS_IN_END\n"
	"	#define VS_OUT_BEGIN\n"
	"	#define VS_OUT_END\n"
	"	#define FS_IN_BEGIN\n"
	"	#define FS_IN_END\n"
	"	#define FS_OUT_BEGIN\n"
	"	#define FS_OUT_END\n"
	"	#define GS_IN_BEGIN\n"
	"	#define GS_IN_END\n"
	"	#define GS_OUT_BEGIN\n"
	"	#define GS_OUT_END\n"
	"	#define GS_STREAM_OUT(x) float _dummy\n"
	"	#define GS_PASS_STREAM_OUT (0.0)\n"
	"	#define DECL_VS_POSITION float4 _VS_OUT##_position\n"
	"	#define TRIANGLE_IN triangles\n"
	"	#define TRIANGLE_ADJ_IN triangles_adjacency\n"
	"	#define LINE_IN lines\n"
	"	#define LINE_ADJ_IN lines_adjacency\n"
	"	#define POINT_IN points\n"
	"	#define TRIANGLE_OUT triangle_strip\n"
	"	#define LINE_OUT line_strip\n"
	"	#define POINT_OUT points\n"
	"	#define float4 vec4\n"
	"	#define float3 vec3\n"
	"	#define float2 vec2\n"
	"	#define float4x4 mat4\n"
	"	#define float3x3 mat3\n"
	"	#define float2x2 mat2\n"
	"	#define mul(x,y) ((x)*(y))\n"
	"	#define static\n"
	"	#define saturate(x) clamp((x), 0.0,1.0)\n"
	"#endif\n";

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

static bool IsTextureUniform(int type)
{
	switch(type)
	{
	case GL_SAMPLER_1D:
	case GL_SAMPLER_2D:
	case GL_SAMPLER_3D:
	case GL_SAMPLER_CUBE:
	case GL_SAMPLER_1D_SHADOW:
	case GL_SAMPLER_2D_SHADOW:
		return true;
	default:
		return false;
	};
}


OGL3Effect::OGL3Effect(OpenGL3APIWrapper* pWrapper,const char* pProgram,HAGE::u16 rasterizer,HAGE::u16 blend) 
	:m_pWrapper(pWrapper),m_BlendState(blend),m_RastState(rasterizer)
{
	glError();
	const char* hlsl_source_v[] = {extStr,vsStr,preproc,pProgram};
	const char* hlsl_source_f[] = {extStr,fsStr,preproc,pProgram};
	const char* hlsl_source_g[] = {extStr,gsStr,preproc,pProgram};
	if(strstr(pProgram,"VERTEX_SHADER"))
	{
		_glVShader =  glCreateShader(GL_VERTEX_SHADER);
		glError();
		glShaderSource(	_glVShader,4,hlsl_source_v,NULL);
		glError();
	}
	else _glVShader = 0;

	if(strstr(pProgram,"FRAGMENT_SHADER"))
	{
		_glFShader =  glCreateShader(GL_FRAGMENT_SHADER);
		glError();
		glShaderSource(	_glFShader,4,hlsl_source_f,NULL);
		glError();
	}
	else _glFShader = 0;
	
	if(strstr(pProgram,"GEOMETRY_SHADER"))
	{
		_glGShader =  glCreateShader(GL_GEOMETRY_SHADER);
		glError();
		glShaderSource(	_glGShader,4,hlsl_source_g,NULL);
		glError();
	}
	else _glGShader = 0;

	_glProgram = glCreateProgram();
	glError();
	
	int bCompile;
	if(_glVShader)
		glAttachShader(_glProgram,_glVShader);
	glError();
	if(_glFShader)
		glAttachShader(_glProgram,_glFShader);
	glError();
	if(_glGShader)
		glAttachShader(_glProgram,_glGShader);
	glError();

	if(_glVShader)
	{
		glCompileShader(_glVShader);
		CheckShader(_glVShader,GL_COMPILE_STATUS,&bCompile,"compile VS");
		glError();
	}
	if(_glFShader)
	{
		glCompileShader(_glFShader);
		CheckShader(_glFShader,GL_COMPILE_STATUS,&bCompile,"compile FS");
		glError();
	}
	if(_glGShader)
	{
		glCompileShader(_glGShader);
		CheckShader(_glGShader,GL_COMPILE_STATUS,&bCompile,"compile GS");
		glError();
	}

	glBindFragDataLocationEXT(_glProgram,0,"target0");
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
	
	glUseProgram(_glProgram);
	glError();
	bool program = glIsProgram(_glProgram);
	bool vertex = glIsShader(_glVShader);
	bool fragment = glIsShader(_glFShader);

	int nAttribs;
	int nAttribsLength;
	char* cAttribName ;
	
	glGetProgramiv(_glProgram,GL_ACTIVE_ATTRIBUTES,&nAttribs);
	glGetProgramiv(_glProgram,GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,&nAttribsLength);
	cAttribName = new char[nAttribsLength];
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
	_constantBuffers.resize(nAttribs);
	/*
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
	*/
	glGetProgramiv(_glProgram,GL_ACTIVE_UNIFORMS,&nAttribs);
	glGetProgramiv(_glProgram,GL_ACTIVE_UNIFORM_MAX_LENGTH,&nAttribsLength);
	cAttribName = new char[nAttribsLength];
	int nTextures = 0;
	for(int i =0;i<nAttribs;++i)
	{
		int len;
		int size;
		GLenum type;
		glGetActiveUniform(_glProgram,i,nAttribsLength,&len,&size,&type,cAttribName);
		if(IsTextureUniform(type))
		{
			if(i+1 > _textureSlots.size())
			{
				_textureSlots.resize(i+1);
			}
			_textureSlots[i] = nTextures;
			glUniform1i(glGetUniformLocation(_glProgram,cAttribName),nTextures);
			glError();
			nTextures++;
		}
		//printf("Uniform %i \@ %i= %s, size %i, type %i\n",i,glGetUniformLocation(_glProgram,cAttribName),	cAttribName,size,type);
	}
	_textures.resize(nTextures);
	delete cAttribName;

	printf("Created %08x in context %08x\n",this,wglGetCurrentContext());
	
	glError();
	glUseProgram(0);
	glError();
}

OGL3Effect::~OGL3Effect()
{
}

void OGL3Effect::Draw(HAGE::APIWVertexArray* pArrayPre)
{
	//printf("Rendering %08x in context %08x\n",this,wglGetCurrentContext());
	OGL3VertexArray* pArray = (OGL3VertexArray*)pArrayPre;/*
	OGL3ConstantBuffer** ppConstants = (OGL3ConstantBuffer**)_pConstants;

	assert(nConstants<=N_CBUFFERS);
	*/
	glUseProgram(_glProgram);
	glError();

	// set cBuffers
	
	for(HAGE::u32 i =0; i<_constantBuffers.size();++i)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER,i,_constantBuffers[i]->m_cbo);
		glUniformBlockBinding(_glProgram, i, i);
		glError();
	}
	for(HAGE::u32 i =0; i<_textures.size();++i)
	{
		glActiveTexture(GL_TEXTURE0+i);
		if(_textures[i]->_miscFlags & HAGE::TEXTURE_CUBE)
			glBindTexture(GL_TEXTURE_CUBE_MAP, _textures[i]->_tbo);
		else
			glBindTexture(GL_TEXTURE_2D, _textures[i]->_tbo);
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


void OGL3Effect::SetConstant(const char* pName,const HAGE::APIWConstantBuffer* constant)
{
	char temp[256];
	temp[0] = '_';
	strcpy(&temp[1],pName);
	int loc = glGetUniformBlockIndex(_glProgram,temp);
	if(loc >= 0 && loc < _constantBuffers.size())
		_constantBuffers[loc] = (const OGL3ConstantBuffer*)constant;
}

void OGL3Effect::SetTexture(const char* pName,const HAGE::APIWTexture* texture)
{
	int loc = glGetUniformLocation(_glProgram,pName);
	if(loc >= 0 && loc < _textureSlots.size())
		_textures[_textureSlots[loc]] = (const OGL3Texture*)texture;
}

#endif NO_OGL