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

DECLARE_CLASS_GUID(RenderingActor,	0x9d008bfa,0x26d3,0x461c,0x917a,0xbd9f4acc446d);
DECLARE_CLASS_GUID(LogicActor,		0xcc98d92a,0xc597,0x47cd,0xaaa8,0xd42c0dde5723);
DECLARE_CLASS_GUID(GraphicsActor,	0x9ecbcb76,0xa470,0x43d0,0xb1b5,0xfc57c3d0051b);

template<> class get_traits<LogicActor> : public ObjectTraits<LogicActor,LogicDomain,Vector3<>,Vector3<>> {};
template<> class get_traits<GraphicsActor> : public ObjectTraits<GraphicsActor,GraphicsDomain,NoDirectInstantiation,Vector3<u8>,LogicActor> {};
template<> class get_traits<RenderingActor> : public ObjectTraits<RenderingActor,RenderingDomain,NoDirectInstantiation,void,LogicActor,GraphicsActor> {};

}

#endif
