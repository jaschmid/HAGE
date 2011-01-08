#ifndef LIGHT__GENERIC__INCLUDED
#define LIGHT__GENERIC__INCLUDED

#include "header.h"
#include <boost/random/mersenne_twister.hpp>

namespace HAGE {

class GenericLight
{
public:
	GenericLight() {}
	~GenericLight() {}
};

class LogicLight;
class GraphicsLight;
class RenderingLight;

struct LightInit
{
	Vector3<>	Position;
	Vector3<>	Color;
	f32			Range;
};

struct LLightOut
{
	Vector3<>	Position;
	Vector3<>	Color;
	f32			Range;
};

struct GLightOut
{
	Vector3<>	Position;
	Vector3<>	Color;
	f32			Range;
	bool		bVisible;
};


DECLARE_CLASS_GUID(RenderingLight,			0xd5cd9332,0x703f,0x43b4,0x8b5e,0x2e4db6371a8f);
DECLARE_CLASS_GUID(LogicLight,				0xb2428990,0xbdc9,0x4734,0x8f3b,0x16ac5fa93f3f);
DECLARE_CLASS_GUID(GraphicsLight,			0x6353c6b2,0x1018,0x436f,0xb67b,0x6c2e69f7a931);

template<> class get_traits<LogicLight> : public ObjectTraits<LogicLight,LogicDomain,LightInit,LLightOut> {};
template<> class get_traits<GraphicsLight> : public ObjectTraits<GraphicsLight,GraphicsDomain,NoDirectInstantiation,GLightOut,LogicLight> {};
template<> class get_traits<RenderingLight> : public ObjectTraits<RenderingLight,RenderingDomain,NoDirectInstantiation,void,GraphicsLight> {};

}

#endif
