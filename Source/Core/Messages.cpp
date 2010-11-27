#include "HAGE.h"

namespace HAGE {
	
	MessageFactoryObjectCreated::MessageFactoryObjectCreated(const guid& ObjectId,const guid& ObjectTypeId,const MemHandle& h) :
		MessageHelper<MessageFactoryObjectCreated,Package>(id),
		objectId(ObjectId),
		objectTypeId(ObjectTypeId),
		handle(h) 
	{

		if(handle.isValid())
			PinBase::ReferenceMemBlock(handle);
	}
	MessageFactoryObjectCreated::MessageFactoryObjectCreated(const MessageFactoryObjectCreated& m) :
		MessageHelper<MessageFactoryObjectCreated,Package>(id),
		objectId(m.objectId),
		objectTypeId(m.objectTypeId),
		handle(m.handle) 
	{
		if(handle.isValid())
			PinBase::ReferenceMemBlock(handle);
	}
	MessageFactoryObjectCreated::~MessageFactoryObjectCreated()
	{
		if(handle.isValid())
			PinBase::FreeMemBlock(handle);
	}

	 MessageObjectOutputInit::MessageObjectOutputInit(const guid& target,const MemHandle& Handle)
	 : MessageObjectHelper<MessageObjectOutputInit,Package>(id,target),handle(Handle) 
	 {
		 if(handle.isValid())
			PinBase::ReferenceMemBlock(handle);
	 }
	MessageObjectOutputInit::MessageObjectOutputInit(const MessageObjectOutputInit& m)
		: MessageObjectHelper<MessageObjectOutputInit,Package>(m),handle(m.handle) 
	{
		if(handle.isValid())
			PinBase::ReferenceMemBlock(handle);
	}

	 MessageObjectOutputInit::~MessageObjectOutputInit()
	{
		if(handle.isValid())
			PinBase::FreeMemBlock(handle);
	}

}