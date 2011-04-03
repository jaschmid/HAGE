#include <HAGE.h>
#include "OpenGL3APIWrapper.h"

#ifdef TARGET_LINUX
#include <strings.h>
#define stricmp strcasecmp
#elif defined(TARGET_WINDOWS)
#include <string.h>
#ifdef COMPILER_GCC
extern "C" {
_CRTIMP int __cdecl __MINGW_NOTHROW	stricmp (const char*, const char*);
}
#endif
#endif

#ifndef NO_OGL


OGL3VertexArray::OGL3VertexArray(OpenGL3APIWrapper* pWrapper,HAGE::u32 nPrimitives,HAGE::APIWPrimitiveType PrimitiveType,HAGE::APIWVertexBuffer** pBuffers,HAGE::u32 nBuffers, const HAGE::u32* pIndexBufferData)
	: m_pWrapper(pWrapper),m_pBuffers(new OGL3VertexBuffer*[nBuffers]),m_nBuffers(nBuffers),m_PrimitiveType(PrimitiveType),m_nPrimitives(nPrimitives)
{

	for(HAGE::u32 i = 0;i<nBuffers;++i)
		m_pBuffers[i]=(OGL3VertexBuffer*)pBuffers[i];

	if(pIndexBufferData)
	{
		HAGE::u32 indexBufferSize = 0;
		switch(PrimitiveType)
		{
		case HAGE::PRIMITIVE_POINTLIST:
			indexBufferSize = nPrimitives*sizeof(HAGE::u32);
			break;
		case HAGE::PRIMITIVE_LINELIST:
			indexBufferSize = nPrimitives*2*sizeof(HAGE::u32);
			break;
		case HAGE::PRIMITIVE_LINESTRIP:
			indexBufferSize = (nPrimitives+1)*sizeof(HAGE::u32);
			break;
		case HAGE::PRIMITIVE_TRIANGLELIST:
			indexBufferSize = (nPrimitives*3)*sizeof(HAGE::u32);
			break;
		case HAGE::PRIMITIVE_TRIANGLESTRIP:
			indexBufferSize = (nPrimitives+2)*sizeof(HAGE::u32);
			break;
		}

		glGenBuffers(1, &m_vboIndexID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboIndexID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, pIndexBufferData, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	else
		m_vboIndexID = 0;
	printf("Created %08x in context %08x\n",this->m_vaoID,wglGetCurrentContext());
	
	glError();
}

unsigned int OGL3VertexArray::GetVA(GLuint program)
{
	auto it = m_vaoID.find(program);
	if(it!=m_vaoID.end())
		return it->second;
	else
	{
		unsigned int id;
		glGenVertexArrays(1, &id);
		glError();

		// VAO setup
		glBindVertexArray(id);
		glError();

		for(HAGE::u32 i = 0;i<m_nBuffers;++i)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_pBuffers[i]->m_vboID);

			int nOffset = 0;

			const OpenGL3APIWrapper::VertexFormatEntry* pFormat=m_pWrapper->GetVertexFormat(m_pBuffers[i]->m_code);
			for(HAGE::u32 j=0;j<pFormat->nElements;++j)
			{
				GLint nAttributes;
				GLenum Type;
				int size = 0;
				switch(pFormat->pOriginalDescription[j].fFormat)
				{
				case HAGE::R32G32B32A32_FLOAT:
					nAttributes = 4;
					Type = GL_FLOAT;
					size += 4*sizeof(HAGE::f32);
					break;
				case HAGE::R32G32B32_FLOAT:
					nAttributes = 3;
					Type = GL_FLOAT;
					size += 3*sizeof(HAGE::f32);
					break;
				case HAGE::R32G32_FLOAT:
					nAttributes = 2;
					Type = GL_FLOAT;
					size += 2*sizeof(HAGE::f32);
					break;
				case HAGE::R32_FLOAT:
					nAttributes = 1;
					Type = GL_FLOAT;
					size += 1*sizeof(HAGE::f32);
					break;
				}
				int nProperty = -1;

				int nAttribs;
				int nAttribsLength;
				
				glGetProgramiv(program,GL_ACTIVE_ATTRIBUTES,&nAttribs);
				glGetProgramiv(program,GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,&nAttribsLength);
				char* cAttribName = new char[nAttribsLength];
				for(int i =0;i<nAttribs;++i)
				{
					int len;
					int size;
					GLenum type;
					glGetActiveAttrib(program,i,nAttribsLength,&len,&size,&type,cAttribName);
					if(stricmp(pFormat->pOriginalDescription[j].pName,&cAttribName[1])==0)
						nProperty = glGetAttribLocation(program,cAttribName);
				}

				delete [] cAttribName;

				if(nProperty!=-1);
				{
					glVertexAttribPointer((GLuint)nProperty, nAttributes, Type, GL_FALSE, pFormat->uVertexSize, (const GLvoid*)nOffset);
					glEnableVertexAttribArray(nProperty);
				}
				nOffset += size;
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_vaoID.insert(std::pair<GLuint,unsigned int>(program,id));
		}

		glBindVertexArray(0);

		return id;
	}



}

OGL3VertexArray::~OGL3VertexArray()
{
	if(m_pBuffers)delete [] m_pBuffers;
	if(m_vboIndexID)glDeleteBuffers(1, &m_vboIndexID);
}

#endif
