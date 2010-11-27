#ifndef ACTOR__RENDERING__INCLUDED
#define ACTOR__RENDERING__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "RenderingDomain.h"

namespace HAGE {
	
class RenderingActor  : public GenericActor, public ObjectBase<RenderingActor>
{
public:
	static RenderingActor* CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source);
	
	int Step(RenderingDomain* pRendering);

private:
	RenderingActor(const guid& ObjectId,const MemHandle& h,const guid& source);
	virtual ~RenderingActor();


	Vector3<>		position;
	Vector3<u8>		color;
};

}
#endif