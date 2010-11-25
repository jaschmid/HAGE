#include "header.h"
#include "LogicDomain.h"
#include "GenericActor.h"

namespace HAGE {

	IObject* Actor<LogicDomain>::CreateInstance(guid objectId)
	{
		return (IObject*) new Actor<LogicDomain>(objectId);
	}

	Actor<LogicDomain>::Actor(guid ObjectId) : GenericActor(ObjectId),TestOut(GetOutputPin())
	{
		position=Vector3<>(getFRand()*20.0f,getFRand()*20.0f,getFRand()*20.0f);
		speed=Vector3<>(getFRand()*.2f,getFRand()*.2f,getFRand()*.2f);
		acceleration=Vector3<>(0.0f,0.0f,0.0f);
		PostMessage(MessageObjectOutputInit(ObjectId,TestOut.GetHandle()));
	}

	Actor<LogicDomain>::~Actor()
	{
	}

	result Actor<LogicDomain>::Step(u64 step)
	{
		return S_OK;
	}

}