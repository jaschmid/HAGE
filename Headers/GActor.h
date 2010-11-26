#ifndef ACTOR__GRAPHICS__INCLUDED
#define ACTOR__GRAPHICS__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "GraphicsDomain.h"

namespace HAGE {

class GraphicsActor  : public GenericActor, public ObjectBase<GraphicsActor>
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


}
#endif