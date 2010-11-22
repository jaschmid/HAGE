#include "header.h"
#include "GenericActor.h"
#include "RActor.h"

namespace HAGE {

	IObject* Actor<RenderingDomain>::CreateInstance(guid objectId)
	{
		return (IObject*) new Actor<RenderingDomain>(objectId);
	}

	Actor<RenderingDomain>::Actor(guid ObjectId) : 
		GenericActor(ObjectId),
		ColorIn(GetInputPin<GraphicsDomain>()),
		PositionIn(GetInputPin<LogicDomain>())
	{
	}

	Actor<RenderingDomain>::~Actor()
	{
	}

	result Actor<RenderingDomain>::Step(u64 step)
	{
		return S_OK;
	}

	bool Actor<RenderingDomain>::MessageProc(const MessageObjectUnknown* pMessage)
	{
		if(pMessage->GetMessageCode() == MESSAGE_OBJECT_OUTPUT_INIT)
		{
			MessageObjectOutputInit* pDetailed = (MessageObjectOutputInit*)pMessage;
			if(pDetailed->GetSource() == guidLogicDomain)
				PositionIn.Open(pDetailed->GetHandle());
			else if(pDetailed->GetSource() == guidGraphicsDomain)
				ColorIn.Open(pDetailed->GetHandle());		
			return true;
		}
		return false;
	}
}