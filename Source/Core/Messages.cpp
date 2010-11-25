#include "HAGE.h"

namespace HAGE {

 MessageObjectOutputInit::MessageObjectOutputInit(const guid& target,const MemHandle& Handle)
	 : MessageObjectHelper<MessageObjectOutputInit,Package>(id,target),handle(Handle) 
 {
	 PinBase::ReferenceMemBlock(handle);
 }
MessageObjectOutputInit::MessageObjectOutputInit(const MessageObjectOutputInit& m)
	: MessageObjectHelper<MessageObjectOutputInit,Package>(m),handle(m.handle) 
{
	 PinBase::ReferenceMemBlock(handle);
}

 MessageObjectOutputInit::~MessageObjectOutputInit()
{
	PinBase::FreeMemBlock(handle);
}

}