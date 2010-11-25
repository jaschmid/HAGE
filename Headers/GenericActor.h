#ifndef ACTOR__GENERIC__INCLUDED
#define ACTOR__GENERIC__INCLUDED

#include "header.h"
#include <boost/random/mersenne_twister.hpp>

namespace HAGE {

class GenericActor 
{
public:
	GenericActor() {}
	~GenericActor() {}
};

class LogicActor;
class GraphicsActor;
class RenderingActor;

typedef ObjectBase<LogicActor,LogicDomain,Vector3<>>										LogicActorBase;
typedef ObjectBase<GraphicsActor,GraphicsDomain,Vector3<u8>,LogicActorBase>					GraphicsActorBase;
typedef ObjectBase<RenderingActor,RenderingDomain,void,LogicActorBase,GraphicsActorBase>	RenderingActorBase;

}

#endif
