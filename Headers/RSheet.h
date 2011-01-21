#ifndef SHEET__RENDERING__INCLUDED
#define SHEET__RENDERING__INCLUDED

#include "header.h"
#include "GenericSheet.h"
#include "RenderingDomain.h"

namespace HAGE {

	struct SheetVertexFormat
	{
	public:
		Vector3<>	position;
		Vector3<>	normal;
		Vector2<>	texcoord;
		Vector3<>	color;

		static const char* name;
	};

class RenderingSheet  : public GenericSheet, public ObjectBase<RenderingSheet>
{
public:
	static RenderingSheet* CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source);
	
	int Draw(EffectContainer* pEffect,const position_constants& c,APIWConstantBuffer* pBuffer);

private:
	RenderingSheet(const guid& ObjectId,const MemHandle& h,const guid& source);
	virtual ~RenderingSheet();

	const static u32 nTriangles = (SheetSize-1)*(SheetSize-1)*2*2;
	const static u32 nVertices = SheetSize*SheetSize*2;

	TResourceAccess<ITextureImage>				_texture;
	std::array<SheetVertexFormat,nVertices>*	_pVertexData;
	
	APIWVertexBuffer*							_pVertexBuffer;
	APIWVertexArray*							_pVertexArray;
};

}
#endif