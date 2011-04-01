#ifndef ACTOR__LOGIC__INCLUDED
#define ACTOR__LOGIC__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "LogicDomain.h"

namespace HAGE {
	
class LogicActor : public GenericActor, public ObjectBase<LogicActor>
{
public:
	static LogicActor* CreateInstance(const guid& ObjectId,const ActorInit& init);

	Matrix4<> GetWorldMatrix() const {return	Output::GetOld().worldMatrix;}

	bool Step(guid& out);

private:
	LogicActor(guid ObjectId,const ActorInit& vpos);
	virtual ~LogicActor();

	ActorInit		_init;

	Matrix4<>		worldMatrix;
	float			rotation;
	float			orbit_rotation;
};

}
#endif