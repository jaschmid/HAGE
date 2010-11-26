#ifndef ACTOR__RENDERING__INCLUDED
#define ACTOR__RENDERING__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "RenderingDomain.h"

namespace HAGE {
	
class RenderingActor  : public GenericActor, public ObjectBase<RenderingActor>
{
public:
	static IObject* CreateInstance(guid ObjectId);
	
	int Step(RenderingDomain* pRendering)
	{
		position = Input1::Get();
		if(Input2::IsReady())
			color = Input2::Get();
		pRendering->DrawIco(position);
		return 1;
	}

	static const bool isImplemented = true;
private:
	RenderingActor(guid ObjectId);
	virtual ~RenderingActor();


	Vector3<>		position;
	Vector3<u8>		color;
};

}
#endif