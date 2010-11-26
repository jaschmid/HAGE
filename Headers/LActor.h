#ifndef ACTOR__LOGIC__INCLUDED
#define ACTOR__LOGIC__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "LogicDomain.h"

namespace HAGE {

class LogicActor : public GenericActor, public ObjectBase<LogicActor>
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
	bool Step(const std::vector<Vector3<>>& positions,guid& out)
	{
		speed = speed + acceleration * 0.05f;
		position = position + speed *0.05f;
		
		Output::Set(position);

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
		
		if(position.x<=-50.0)
			return true;
		if(position.y<=-50.0)
			return true;
		if(position.z<=-50.0)
			return true;
		if(position.x>=50.0)
			return true;
		if(position.y>=50.0)
			return true;
		if(position.z>=50.0)
			return true;

		out = m_guidObjectId;

		return false;
	}

	static const bool isImplemented = true;
private:
	LogicActor(guid ObjectId);
	virtual ~LogicActor();

	Vector3<>		position;
	Vector3<>		speed;
	Vector3<>		acceleration;
};

}
#endif