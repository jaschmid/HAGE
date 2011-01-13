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
	
	inline static float getFRand()
	{
		static boost::mt19937 rgen;
		return ((float)rgen()/(float)rgen.max())*2.0f-1.0f;
	}

	float GetMass() const { return _init.mass;}

	bool Step(guid& out);

private:
	LogicActor(guid ObjectId,const ActorInit& vpos);
	virtual ~LogicActor();

	ActorInit		_init;

	Vector3<>		axis;
	Vector3<>		position;
	Vector3<>		speed;
	Vector3<>		acceleration;
};

}
#endif