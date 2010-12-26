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

/*
POSITION, ATTR0 Input Vertex, Generic Attribute 0

BLENDWEIGHT, ATTR1 Input vertex weight, Generic Attribute 1

NORMAL, ATTR2 Input normal, Generic Attribute 2

DIFFUSE, COLOR0, ATTR3 Input primary color, Generic Attribute 3

SPECULAR, COLOR1, ATTR4 Input secondary color, Generic Attribute 4

TESSFACTOR, FOGCOORD, ATTR5 Input fog coordinate, Generic Attribute 5

PSIZE, ATTR6 Input point size, Generic Attribute 6

BLENDINDICES, ATTR7 Generic Attribute 7

TEXCOORD0-TEXCOORD7, Input texture coordinates (texcoord0-texcoord7)
ATTR8-ATTR15 Generic Attributes 8-15

TANGENT, ATTR14 Generic Attribute 14

BINORMAL, ATTR15 Generic Attribute 15
*/

// LOL HACKY SHIT
const char* vertex_slot_names[] =
{
	"POSITION",
	//"BLENDWEIGHT",
	"NORMAL",
	"COLOR",
	"COLOR1",
	"FOGCOORD",
	"PSIZE",
	"BLENDINDICES",
	"TEXCOORD",
	"TEXCOORD1",
	"TEXCOORD2",
	"TEXCOORD3",
	"TEXCOORD4",
	"TEXCOORD5",
	"TEXCOORD6",
	"TEXCOORD7",
	"TANGENT",
	"BINORMAL"
};

OGL3VertexArray::OGL3VertexArray(OpenGL3APIWrapper* pWrapper,HAGE::u32 nPrimitives,HAGE::APIWPrimitiveType PrimitiveType,HAGE::APIWVertexBuffer** pBuffers,HAGE::u32 nBuffers, const HAGE::u32* pIndexBufferData)
	: m_pWrapper(pWrapper),m_pBuffers(new OGL3VertexBuffer*[nBuffers]),m_nBuffers(nBuffers),m_PrimitiveType(PrimitiveType),m_nPrimitives(nPrimitives),_bInit(false)
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
}

void OGL3VertexArray::Init()
{
	if(!_bInit)
	{
		_bInit=true;
		glGenVertexArrays(1, &m_vaoID);
		glError();

		// VAO setup
		glBindVertexArray(m_vaoID);
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
				for(int k=0;k<16;++k)
					if(stricmp(pFormat->pOriginalDescription[j].pName,vertex_slot_names[k])==0)
					{
						nProperty = k;
						break;
					}
				assert(nProperty!=-1);
				glVertexAttribPointer((GLuint)nProperty, nAttributes, Type, GL_FALSE, pFormat->uVertexSize, (const GLvoid*)nOffset);
				glEnableVertexAttribArray(nProperty);
				nOffset += size;
			}
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		glBindVertexArray(0);
	}



}

OGL3VertexArray::~OGL3VertexArray()
{
	if(m_pBuffers)delete [] m_pBuffers;
	if(m_vboIndexID)glDeleteBuffers(1, &m_vboIndexID);
}

#endif
