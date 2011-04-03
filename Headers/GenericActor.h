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

enum
{
	ACTOR_BEHAVIOR_PLANET,
	ACTOR_BEHAVIOR_SUN
};

struct ActorInit
{
	int				behavior;
	bool			bCastShadow;
	char			mesh[200];
	char			texture[200];
	float			orbit_speed;
	float			rotation_speed;
	Vector3<>		orbit_axis;
	Vector3<>		rotation_axis;
	Vector3<>		initial_position;
	float			initial_distance;
	guid			master_object;
	Vector3<>		scale;
};

struct ActorOut
{
	Matrix4<>		worldMatrix;
	Matrix4<>		orbitWorldMatrix;
};

typedef ActorInit ActorGInit;
typedef ActorInit ActorRInit;

DECLARE_CLASS_GUID(RenderingActor,	0x9d008bfa,0x26d3,0x461c,0x917a,0xbd9f4acc446d);
DECLARE_CLASS_GUID(LogicActor,		0xcc98d92a,0xc597,0x47cd,0xaaa8,0xd42c0dde5723);
DECLARE_CLASS_GUID(GraphicsActor,	0x9ecbcb76,0xa470,0x43d0,0xb1b5,0xfc57c3d0051b);

template<> class get_traits<LogicActor> : public ObjectTraits<LogicActor,LogicDomain,InOutInit<ActorInit,ActorGInit>,ActorOut> {};
template<> class get_traits<GraphicsActor> : public ObjectTraits<GraphicsActor,GraphicsDomain,NoDirectInstantiation<InOutInit<ActorGInit,ActorRInit>>,ActorOut,LogicActor> {};
template<> class get_traits<RenderingActor> : public ObjectTraits<RenderingActor,RenderingDomain,NoDirectInstantiation<ActorRInit>,void,GraphicsActor> {};

}

#endif
