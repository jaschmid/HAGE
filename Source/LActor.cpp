#include "header.h"
#include "LActor.h"

namespace HAGE {

	IObject* LogicActor::CreateInstance(guid objectId)
	{
		return (IObject*) new LogicActor(objectId);
	}

	LogicActor::LogicActor(guid ObjectId) : GenericActor(),ObjectBase<LogicActor,LogicDomain,Vector3<>>(ObjectId)
	{
		position=Vector3<>(getFRand()*20.0f,getFRand()*20.0f,getFRand()*20.0f);
		speed=Vector3<>(getFRand()*.2f,getFRand()*.2f,getFRand()*.2f);
		acceleration=Vector3<>(0.0f,0.0f,0.0f);
		//PostMessage(MessageObjectOutputInit(ObjectId,TestOut.GetHandle()));
	}

	LogicActor::~LogicActor()
	{
	}

	DEFINE_CLASS_GUID(LogicActor);

}
