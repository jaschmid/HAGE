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