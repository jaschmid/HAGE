#include "header.h"
#include "LActor.h"

namespace HAGE {

	LogicActor* LogicActor::CreateInstance(const guid& objectId,const ActorInit& init)
	{
		return new LogicActor(objectId,init);
	}

	bool LogicActor::Step(guid& out)
	{
		if(!_init.gravity_behavior)
		{
			float fd2 = !position;
			position = (Matrix4<>::AngleRotation(axis,0.01f)*Vector4<>(position,1.0f)).xyz();
			Output::Set(position);
		}
		else
		{
			speed = speed + acceleration * 0.005f;
			position = position + speed *0.005f;
		
			Output::Set(position);

			acceleration = Vector3<>(0.0f,0.0f,0.0f);
			auto end = GetFactory()->end(guid_of<LogicActor>::Get());
			for(auto i= GetFactory()->begin(guid_of<LogicActor>::Get());i!=end;++i)
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
		}
	
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

	LogicActor::LogicActor(guid ObjectId,const ActorInit& init) : GenericActor(),ObjectBase<LogicActor>(ObjectId),_init(init)
	{
		position=_init.location;
		speed=Vector3<>(0.0f,0.0f,0.0f);
		acceleration=Vector3<>(0.0f,0.0f,0.0f);
		axis = (position % Vector3<>(0.0,1.0,0.0))/sqrtf(!position);
		Output::Set(position);
	}

	LogicActor::~LogicActor()
	{
	}

	DEFINE_CLASS_GUID(LogicActor);

}
