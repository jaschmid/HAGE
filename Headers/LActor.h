#ifndef ACTOR__LOGIC__INCLUDED
#define ACTOR__LOGIC__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "LogicDomain.h"

namespace HAGE {

class LogicActor : public GenericActor, public LogicActorBase
{
public:
	static IObject* CreateInstance(guid ObjectId);

	float getFRand()
	{
		static boost::mt19937 rgen;
		return ((float)rgen()/(float)rgen.max())*2.0f-1.0f;
	}

	Vector3<> Init()
	{
		return position;
	}
	Vector3<> Step(const std::vector<Vector3<>>& positions)
	{
		speed = speed + acceleration * 0.05f;
		position = position + speed *0.05f;

		acceleration = Vector3<>(0.0f,0.0f,0.0f);
		for(auto i=positions.begin();i!=positions.end();++i)
		{
			Vector3<> vd = *i-position;
			float		fd2 = !(vd);
			if(fd2 >= 0.3)
			{
				float		fd = sqrtf(fd2);
				vd = vd/fd;
				acceleration = acceleration + vd/fd2;	
			}
		}
		
		if(position.x<=-100.0)
			speed.x = -speed.x;
		if(position.y<=-100.0)
			speed.y = -speed.y;
		if(position.z<=-100.0)
			speed.z = speed.z;
		if(position.x>=100.0)
			speed.x = -speed.x;
		if(position.y>=100.0)
			speed.y = -speed.y;
		if(position.z>=100.0)
			speed.z = -speed.z;


		_ObjectBaseOutput<LogicDomain,Vector3<>>::Set(position);
		//*TestOut=position;
		return position;
	}

	static const bool isImplemented = true;
private:
	LogicActor(guid ObjectId);
	virtual ~LogicActor();

	Vector3<>		position;
	Vector3<>		speed;
	Vector3<>		acceleration;
};

DECLARE_CLASS_GUID(LogicActor,		0xcc98d92a,0xc597,0x47cd,0xaaa8,0xd42c0dde5723);
}
#endif