#include <HAGE.h>

namespace HAGE {

	SharedDomainBase::SharedDomainBase(boost::function<void()> f,boost::function<void(bool)> f2,const guid& id)
		:staticCallback(f),staticQueue(f2),nInputCallbacks(0),nOutputCallbacks(0),nCallbacksRecieved(0),bInit(false),outputPin(nullptr),
		guidDomain(id),bShutdown(false),
		nDelayedInputCallbacks(0),Factory((PinBase*&)outputPin,&Tasks)
	{
	}
	SharedDomainBase::~SharedDomainBase(){}

	void SharedDomainBase::Callback()
	{
		i32 recieved =_InterlockedIncrement(&nCallbacksRecieved);
		if(recieved == nInputCallbacks + nOutputCallbacks && !bShutdown)
		{
			nCallbacksRecieved=nDelayedInputCallbacks;

			if(bInit)
				staticQueue(false);
			else
			{
				bInit = true;
				staticQueue(true);
			}
		}
	}

	void SharedDomainBase::StepComplete()
	{

		for(u32 i=0;i<inputPins.size();++i)
		{
			if(inputPins[i].first == -2)
			{
				++nInputCallbacks;
				++nDelayedInputCallbacks;
			}
			else if(inputPins[i].first == -1)
				--nDelayedInputCallbacks;
			else if(inputPins[i].first == 1)
					--nDelayedInputCallbacks;
		}

		if(!bShutdown)
		{
			// don't close pins in shutdown
			if(outputPin)
				outputPin->ClosePin();
			for(u32 i=0;i<inputPins.size();++i)
			{
				if(inputPins[i].first == 0)
					inputPins[i].second->ClosePin();
				else if( inputPins[i].first < 0)
					inputPins[i].first ++;
				else if( inputPins[i].first > 0)
					inputPins[i].first --;
			}
		}
	}

	void SharedDomainBase::RegisterOutput(LockedMessageQueue& out)
	{
		outputPin = &out;
		nOutputCallbacks++;
		outputPin->InitializeOutputPin(staticCallback,guidDomain);
	}

	void SharedDomainBase::RegisterInput(LockedMessageQueue& in,i32 delay)
	{
		in.InitializeInputPin(staticCallback);
		inputPins.push_back(inputPin(delay,&in));
		if(delay == -1)
			delay = 1;
		if(delay >= 0)
		{
			nInputCallbacks++;
			if(delay > 0)
			{
				++nDelayedInputCallbacks;
			}
		}
	}

	bool SharedDomainBase::MessageProc(const Message* pMessage)
	{
		if( IsMessageType(pMessage->GetMessageCode(),MESSAGE_FACTORY_UNKNOWN) )
		{
			return Factory.MessageProc((const MessageFactoryUnknown*)pMessage);
		}
		else if(  IsMessageType(pMessage->GetMessageCode(),MESSAGE_OBJECT_UNKNOWN) )
		{
			return Factory.DispatchMessage((const MessageObjectUnknown*)pMessage);
		}

		return false;
	}

	void SharedDomainBase::PostMessage(const Message& message)
	{
		if(outputPin)
		{
			outputPin->PostMessage(message);
		}
	}

	void SharedDomainBase::Init(u64 step)
	{
		ProcessMessages();
	}

	void SharedDomainBase::Step(u64 step)
	{
		ProcessMessages();

		DomainStep(step);
	}

	void SharedDomainBase::Shutdown(u64 step)
	{
		ProcessMessages();

		Factory.Shutdown();

		bShutdown = true;
		if(outputPin)
			outputPin->Shutdown();
		for(u32 i = 0;i < inputPins.size();++i)
			inputPins[i].second->Shutdown();
	}

	void SharedDomainBase::ProcessMessages()
	{
		const Message* m=nullptr;
		for(u32 i = 0;i < inputPins.size();++i)
			if(inputPins[i].first == 0)
				while(m=inputPins[i].second->GetNextMessage(m))
				{
					if(!MessageProc(m) && outputPin)
					{
						outputPin->ForwardMessage(*m);
					}
				}
	}

}
