#ifndef ACTOR__LOGIC__INCLUDED
#define ACTOR__LOGIC__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "LogicDomain.h"

namespace HAGE {

#define CIRCULAR_MOTION

class LogicActor : public GenericActor, public ObjectBase<LogicActor>
{
public:
	static LogicActor* CreateInstance(const guid& ObjectId,const Vector3<>& vpos);

	inline static float getFRand()
	{
		static boost::mt19937 rgen;
		return ((float)rgen()/(float)rgen.max())*2.0f-1.0f;
	}

	bool Step(guid& out);

private:
	LogicActor(guid ObjectId,const Vector3<>& vpos);
	virtual ~LogicActor();

	Vector3<>		axis;
	Vector3<>		position;
	Vector3<>		speed;
	Vector3<>		acceleration;
};

}
#endif