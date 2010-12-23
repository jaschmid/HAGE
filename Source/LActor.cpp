#include "header.h"
#include "LActor.h"

namespace HAGE {

	LogicActor* LogicActor::CreateInstance(const guid& objectId,const Vector3<>& vpos)
	{
		return new LogicActor(objectId,vpos);
	}

	bool LogicActor::Step(guid& out)
	{
#ifdef CIRCULAR_MOTION
		float fd2 = !position;
		position = (Matrix4<>::AngleRotation(axis,0.01f)*Vector4<>(position,1.0f)).xyz();
		Output::Set(position);
#else
		speed = speed + acceleration * 0.05f;
		position = position + speed *0.05f;
		
		Output::Set(position);

		acceleration = Vector3<>(0.0f,0.0f,0.0f);
		auto end = GetFactory()->end();
		for(auto i= GetFactory()->begin();i!=end;++i)
		{
			if(i.objectId() != m_guidObjectId && i.isOutValid())
			{
				Vector3<> v_out = *(const Vector3<>*)i.lastOut(sizeof(Vector3<>));
				Vector3<> vd = v_out-position;
				float		fd2 = !(vd);
				if(fd2!=0.0f)
				{
					float		fd = sqrtf(fd2);
					vd = vd/fd;
					acceleration = acceleration + vd/fd2;	
				}
			}
		}
#endif
	
		out = m_guidObjectId;

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


		return false;
	}

	LogicActor::LogicActor(guid ObjectId,const Vector3<>& vpos) : GenericActor(),ObjectBase<LogicActor>(ObjectId)
	{
		position=Vector3<>(getFRand()*20.0f,getFRand()*20.0f,getFRand()*20.0f);
		speed=Vector3<>(getFRand()*4.0f,getFRand()*4.0f,getFRand()*4.0f);
		acceleration=Vector3<>(0.0f,0.0f,0.0f);
		axis = (position % Vector3<>(0.0,1.0,0.0))/sqrtf(!position);
		Output::Set(position);
	}

	LogicActor::~LogicActor()
	{
	}

	DEFINE_CLASS_GUID(LogicActor);

}
