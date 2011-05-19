#ifndef LIGHT__RENDERING__INCLUDED
#define LIGHT__RENDERING__INCLUDED

#include "header.h"
#include "GenericLight.h"
#include "RenderingDomain.h"

namespace HAGE {

class RenderingLight  : public GenericLight, public ObjectBase<RenderingLight>
{
public:
	static RenderingLight* CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source);
	
	GLightOut Step(RenderingDomain* pRendering);
	
	int Draw(EffectContainer* pEffect,const position_constants& c,APIWConstantBuffer* pBuffer,bool bShadow,bool bOrbit);

private:
	RenderingLight(const guid& ObjectId,const MemHandle& h,const guid& source);
	
	TResourceAccess<ITextureImage>						_texture;
	TResourceAccess<IDrawableMesh>						_mesh;

	GLightOut _data;
	virtual ~RenderingLight();
};

}
#endif