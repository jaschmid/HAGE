#include "header.h"
#include "GenericActor.h"
#include "GraphicsDomain.h"

namespace HAGE {

	IObject* Actor<GraphicsDomain>::CreateInstance(guid objectId)
	{
		return (IObject*) new Actor<GraphicsDomain>(objectId);
	}

	Actor<GraphicsDomain>::Actor(guid ObjectId) : 
		GenericActor(ObjectId),
		ColorOut(GetOutputPin()),
		PositionIn(GetInputPin<LogicDomain>())
	{
		PostMessage(MessageObjectOutputInit(ObjectId,ColorOut.GetHandle()));
	}

	Actor<GraphicsDomain>::~Actor()
	{
	}

	result Actor<GraphicsDomain>::Step(u64 step)
	{
		return S_OK;
	}

	bool Actor<GraphicsDomain>::MessageProc(const MessageObjectUnknown* pMessage)
	{
		if(pMessage->GetMessageCode() == MESSAGE_OBJECT_OUTPUT_INIT)
		{
			MessageObjectOutputInit* pDetailed = (MessageObjectOutputInit*)pMessage;
			if(pDetailed->GetSource() == guidLogicDomain)
				PositionIn.Open(pDetailed->GetHandle());
			return true;
		}
		return false;
	}

}