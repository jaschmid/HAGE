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

struct ActorInit
{
	int				behavior;
	char			mesh[32];
	float			mass;
	Vector3<>		location;
	Vector3<>		scale;
};

struct ActorGInit
{
	char			mesh[32];
	Vector3<>		scale;
};

struct ActorRInit
{
	char			mesh[32];
	Vector3<>		scale;
};

DECLARE_CLASS_GUID(RenderingActor,	0x9d008bfa,0x26d3,0x461c,0x917a,0xbd9f4acc446d);
DECLARE_CLASS_GUID(LogicActor,		0xcc98d92a,0xc597,0x47cd,0xaaa8,0xd42c0dde5723);
DECLARE_CLASS_GUID(GraphicsActor,	0x9ecbcb76,0xa470,0x43d0,0xb1b5,0xfc57c3d0051b);

template<> class get_traits<LogicActor> : public ObjectTraits<LogicActor,LogicDomain,InOutInit<ActorInit,ActorGInit>,Vector3<>> {};
template<> class get_traits<GraphicsActor> : public ObjectTraits<GraphicsActor,GraphicsDomain,NoDirectInstantiation<InOutInit<ActorGInit,ActorRInit>>,Vector3<>,LogicActor> {};
template<> class get_traits<RenderingActor> : public ObjectTraits<RenderingActor,RenderingDomain,NoDirectInstantiation<ActorRInit>,void,GraphicsActor> {};

}

#endif
