#ifndef ACTOR__GRAPHICS__INCLUDED
#define ACTOR__GRAPHICS__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "GraphicsDomain.h"

namespace HAGE {

class GraphicsActor  : public GenericActor, public GraphicsActorBase
{
public:
	static IObject* CreateInstance(guid ObjectId);
	
	int Step()
	{
		position = Input1::Get();
		color = Vector3<u8>(255,255,255);
		Output::Set(color);
		return 1;
	}

	static const bool isImplemented = true;
private:
	GraphicsActor(guid ObjectId);
	virtual ~GraphicsActor();

	Vector3<>		position;
	Vector3<u8>		color;
};

DECLARE_CLASS_GUID(GraphicsActor,	0x9ecbcb76,0xa470,0x43d0,0xb1b5,0xfc57c3d0051b);

}
#endif