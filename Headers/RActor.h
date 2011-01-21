#ifndef ACTOR__RENDERING__INCLUDED
#define ACTOR__RENDERING__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "RenderingDomain.h"

namespace HAGE {
	
class RenderingActor  : public GenericActor, public ObjectBase<RenderingActor>
{
public:
	static RenderingActor* CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source,const ActorRInit* pInit);
	
	int Draw(EffectContainer* pEffect,const position_constants& c,APIWConstantBuffer* pBuffer);

private:
	RenderingActor(const guid& ObjectId,const MemHandle& h,const guid& source,const ActorRInit* pInit);
	virtual ~RenderingActor();

	
	TResourceAccess<ITextureImage>						_texture;
	TResourceAccess<IDrawableMesh>						_mesh;

	Vector3<>		scale;
	Vector3<>		position;
	Vector3<u8>		color;
};

}
#endif