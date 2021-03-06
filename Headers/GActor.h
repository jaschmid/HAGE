#ifndef ACTOR__GRAPHICS__INCLUDED
#define ACTOR__GRAPHICS__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "GraphicsDomain.h"

namespace HAGE {

class GraphicsActor  : public GenericActor, public ObjectBase<GraphicsActor>
{
public:
	static GraphicsActor* CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source,const ActorGInit* pInit);
	
	int Step();

private:
	GraphicsActor(const guid& ObjectId,const MemHandle& h,const guid& source,const ActorGInit* pInit);
	virtual ~GraphicsActor();

	ActorOut data;
};


}
#endif