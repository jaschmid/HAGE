#include "HAGE.h"

namespace HAGE {
	
	MessageFactoryObjectCreated::MessageFactoryObjectCreated(const guid& ObjectId,const guid& ObjectTypeId,const MemHandle& h,const void* pData,u32 nInitSize) :
		MessageHelper<MessageFactoryObjectCreated,Package>(id),
		objectId(ObjectId),
		objectTypeId(ObjectTypeId),
		handle(h),
		nInitDataSize(nInitSize),
		pInitData(pData?DomainMemory::GlobalAllocate(nInitSize):nullptr) 
	{
		if(pInitData)
			memcpy(pInitData,pData,nInitDataSize);
		if(handle.isValid())
			PinBase::ReferenceMemBlock(handle);
	}
	MessageFactoryObjectCreated::MessageFactoryObjectCreated(const MessageFactoryObjectCreated& m) :
		MessageHelper<MessageFactoryObjectCreated,Package>(m),
		objectId(m.objectId),
		objectTypeId(m.objectTypeId),
		handle(m.handle),
		nInitDataSize(m.nInitDataSize),
		pInitData(m.pInitData?DomainMemory::GlobalAllocate(m.nInitDataSize):nullptr) 
	{
		if(pInitData)
			memcpy(pInitData,m.pInitData,nInitDataSize);
		if(handle.isValid())
			PinBase::ReferenceMemBlock(handle);
	}
	MessageFactoryObjectCreated::~MessageFactoryObjectCreated()
	{
		if(pInitData)
			DomainMemory::GlobalFree(pInitData);
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