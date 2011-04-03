#include "header.h"
#include "LActor.h"

namespace HAGE {

	LogicActor* LogicActor::CreateInstance(const guid& objectId,const ActorInit& init)
	{
		return new LogicActor(objectId,init);
	}

	bool LogicActor::Step(guid& out)
	{
		/*
		if(_init.behavior == 1)
		{
			float fd2 = !position;
			position = (Matrix4<>::AngleRotation(axis,GetElapsedTime().toSeconds())*Vector4<>(position,1.0f)).xyz();
			Output::Set(position);
		}
		else if(_init.behavior == 0)
		{
			speed = speed + acceleration * GetElapsedTime().toSeconds();
			position = position + speed * GetElapsedTime().toSeconds();
		
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
					if(fd2 > 0.0001f)
					{
						float		fd = sqrtf(fd2);
						vd = vd/fd;
						acceleration = acceleration + vd/fd2*_init.mass*((const LogicActor*)i.object())->GetMass();	
					}
				}
			}
		}
		else
			Output::Set(position);
	
		out = m_guidObjectId;

		if(position.x<=-5.0)
			return true;
		if(position.y<=-5.0)
			return true;
		if(position.z<=-5.0)
			return true;
		if(position.x>=5.0)
			return true;
		if(position.y>=5.0)
			return true;
		if(position.z>=5.0)
			return true;
		*/
		
		ActorOut outputData;

		outputData.orbitWorldMatrix = Matrix4<>::One();
		//generate Matrix for rotation
		Vector3<> up = Vector3<>(0.0f,1.0f,0.0f);
		Matrix4<> adj = Matrix4<>::One();

		Vector3<> rotation_axis = _init.rotation_axis.normalize();
		if(!(up-rotation_axis) > 0.01f)
		{
			Vector3<> rot = (rotation_axis % up).normalize();
			float angle = acosf(rotation_axis*up);
			adj = Matrix4<>::AngleRotation(rot,-angle);
		}
		
		rotation += _init.rotation_speed * GetElapsedTime().toSeconds();
		worldMatrix = Matrix4<>::AngleRotation(rotation_axis,rotation) * adj ;


		if(_init.behavior == ACTOR_BEHAVIOR_PLANET)
		{
			LogicActor* l=(LogicActor*)GetFactory()->QueryObject(_init.master_object);
			
			orbit_rotation += _init.orbit_speed * GetElapsedTime().toSeconds();

			
			Vector3<> up = Vector3<>(0.0f,1.0f,0.0f);
			Matrix4<> adj = Matrix4<>::One();
			if(!(up-_init.orbit_axis) > 0.01f)
			{
				Vector3<> rot = (_init.orbit_axis % up).normalize();
				float angle = acosf(_init.orbit_axis*up);
				adj = Matrix4<>::AngleRotation(rot,angle);
			}

			outputData.orbitWorldMatrix = l->GetWorldMatrix();

			Matrix4<> local_rotation = Matrix4<>::AngleRotation(_init.orbit_axis,-orbit_rotation);

			Matrix4<> parentTransform = Matrix4<>::AngleRotation(_init.orbit_axis,orbit_rotation) * ( Matrix4<>::Translate(Vector3<>(_init.initial_distance,0.0f,0.0f))*local_rotation); 

			parentTransform = adj * parentTransform;
			worldMatrix = outputData.orbitWorldMatrix * (parentTransform * worldMatrix);
		}
		else
		{
			worldMatrix = outputData.orbitWorldMatrix * (Matrix4<>::Translate(_init.initial_position) * worldMatrix);
			//sun does nothing
		}
		
		Vector3<> pos = (worldMatrix*Vector4<>(0.0f,0.0f,0.0f,1.0f)).xyz();

		outputData.worldMatrix = worldMatrix;
		Output::Set(outputData);

		return false;
	}

	LogicActor::LogicActor(guid ObjectId,const ActorInit& init) : GenericActor(),ObjectBase<LogicActor>(ObjectId),_init(init)
	{
		worldMatrix = Matrix4<>::Translate(Vector3<>(_init.initial_distance,0.0f,0.0f));
		rotation= 0.0f;
		orbit_rotation = 0.0f;
		//speed=Vector3<>(GetRandFloat()-0.5f,GetRandFloat()-0.5f,GetRandFloat()-0.5f)/10.0f;
		//acceleration=Vector3<>(0.0f,0.0f,0.0f);
		//axis = (position % Vector3<>(0.0,1.0,0.0))/sqrtf(!position);
		ActorOut outputData;
		outputData.worldMatrix = worldMatrix;
		Output::Set(outputData);

		ActorGInit& ginit = Output::InitOut;
		ginit = init;
	}

	LogicActor::~LogicActor()
	{
	}

	DEFINE_CLASS_GUID(LogicActor);

}
