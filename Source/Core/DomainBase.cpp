#include <HAGE.h>

extern HAGE::t64 OSGetTime();

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
			/*
			if(this == (SharedDomainBase*)domain_access<RenderingDomain>::Get())
			{
				printf("Queue Rendering\n");
			}*/

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
	{		/*
		if(this == (SharedDomainBase*)domain_access<RenderingDomain>::Get())
		{
			printf("Complete Rendering\n");
		}*/

		for(u32 i=0;i<inputPins.size();++i)
		{
			if(inputPins[i].inputDelay == -2)
			{
				++nInputCallbacks;
				++nDelayedInputCallbacks;
			}
			else if(inputPins[i].inputDelay == -1)
				--nDelayedInputCallbacks;
			else if(inputPins[i].inputDelay == 1)
					--nDelayedInputCallbacks;
		}

		if(!bShutdown)
		{
			// don't close pins in shutdown

			if(outputPin)
			{
				//printf("%08x closes pin %08x\n",(SharedDomainBase*)this,outputPin);
				outputPin->CloseWritePin();
			}
			for(u32 i=0;i<inputPins.size();++i)
			{
				if(inputPins[i].inputDelay == 0)
				{
					//printf("%08x closes pin %08x\n",(SharedDomainBase*)this,inputPins[i].second);
					inputPins[i].pPin->CloseReadPin();
				}
				else if( inputPins[i].inputDelay < 0)
					inputPins[i].inputDelay ++;
				else if( inputPins[i].inputDelay > 0)
					inputPins[i].inputDelay --;
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
		inputPin p;
		p.pPin = &in;
		p.inputDelay = delay;
		inputPins.push_back(p);
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
		//printf("Domain %08x has %i Inputs and %i delayed inputs now\n",(SharedDomainBase*)this,nInputCallbacks,nDelayedInputCallbacks);
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
		if(pMessage->GetMessageCode() == MESSAGE_RESERVED_INIT_TIME)
			return true;

		return false;
	}

	void SharedDomainBase::PostMessage(const Message& message)
	{
		if(outputPin)
		{
			outputPin->PostMessage(message);
		}
	}
	extern const char* GetDomainName(const guid& guid);

	void SharedDomainBase::Init()
	{
		// find time messages
		
		//printf("%s Base Init\n",GetDomainName(guidDomain));
		const Message* m=nullptr;
		u64 rand_seed = 0;
		for(u32 i = 0;i < inputPins.size();++i)
			if(inputPins[i].inputDelay == 0)
				while(m=inputPins[i].pPin->GetNextMessage(m))
					if(m->GetMessageCode() == MESSAGE_RESERVED_INIT_TIME)
					{
						inputPins[i].hTimeHandle = ((MessageReservedInitTime*)m)->GetHandle();
						rand_seed = ((MessageReservedInitTime*)m)->GetSeed();
						PinBase::ReferenceMemBlock(inputPins[i].hTimeHandle);
					}
		_time.time_utc = 0;
		_timeLast = _time;
		for(auto i = inputPins.begin();i!=inputPins.end();++i)
		{
			const void* values[3];
			t64 this_time = ((const timeData*)(((PinBase*)i->pPin)->GetReadMem(i->hTimeHandle,sizeof(timeData))))->time;
			((const timeData*)(((PinBase*)i->pPin)->GetAll(i->hTimeHandle,values)));
			t64 times[3] = {*(const t64*)values[0],*(const t64*)values[1],*(const t64*)values[2]};
			if(this_time > _time)
				_time = this_time;
			assert(_time.time_utc == 0);
		}

		if(rand_seed == 0)
			rand_seed = OSGetTime().time_utc;

		Tasks.InitHighPrecisionGenerator(rand_seed^guidDomain.ll[0]);
		Factory.SeedInternalNumberGenerator(rand_seed^guidDomain.ll[0]);

		if(inputPins.size() == 0)
			_timeBegin = OSGetTime();

		if(outputPin)
		{
			_timeHandle = ((PinBase*)outputPin)->AllocateMemBlock(sizeof(timeData));
			timeData* mem =((timeData*)(((PinBase*)outputPin)->GetWriteMem(_timeHandle,sizeof(timeData))));
			mem->time = _time;
			//printf("%s wrote %I64x to %I64x\n",GetDomainName(guidDomain),_time.time_utc,mem);
			outputPin->PostMessage(MessageReservedInitTime(_timeHandle,rand_seed));
		}
		//something wrong here
		Factory.Step(_time);
		ProcessMessages();
	}

	void SharedDomainBase::Step()
	{/*
		if(this == (SharedDomainBase*)domain_access<RenderingDomain>::Get())
		{
			printf("Step Rendering\n");
		}*/
		//printf("%s Base Step\n",GetDomainName(guidDomain));
		_timeLast = _time;
		_time.time_utc = 0;
		for(auto i = inputPins.begin();i!=inputPins.end();++i)
		{
			t64 this_time = ((const timeData*)(((PinBase*)i->pPin)->GetReadMem(i->hTimeHandle,sizeof(timeData))))->time;
			if(this_time > _time)
				_time = this_time;
		}
		if(inputPins.size() == 0)
		{
			_time = OSGetTime() - _timeBegin;
		}

		if(outputPin)
		{			
			timeData* mem =((timeData*)(((PinBase*)outputPin)->GetWriteMem(_timeHandle,sizeof(timeData))));
			mem->time = _time;
			//printf("%s wrote %I64x to %I64x\n",GetDomainName(guidDomain),_time.time_utc,mem);
		}

		Factory.Step(_time);
		ProcessMessages();

		DomainStep(_time);
	}

	void SharedDomainBase::Shutdown()
	{
		_timeLast = _time;
		_time.time_utc = 0;
		for(auto i = inputPins.begin();i!=inputPins.end();++i)
		{
			t64 this_time = ((const timeData*)(((PinBase*)i->pPin)->GetReadMem(i->hTimeHandle,sizeof(timeData))))->time;
			if(this_time > _time)
				_time = this_time;
		}
		if(inputPins.size() == 0)
			_time = OSGetTime() - _timeBegin;
		if(outputPin)
			((timeData*)(((PinBase*)outputPin)->GetWriteMem(_timeHandle,sizeof(timeData))))->time = _time;

		Factory.Step(_time);
		ProcessMessages();

		Factory.Shutdown();

		bShutdown = true;
		if(outputPin)
			outputPin->Shutdown();
		for(u32 i = 0;i < inputPins.size();++i)
			inputPins[i].pPin->Shutdown();
	}

	void SharedDomainBase::ProcessMessages()
	{
		const Message* m=nullptr;
		for(u32 i = 0;i < inputPins.size();++i)
			if(inputPins[i].inputDelay == 0)
				while(m=inputPins[i].pPin->GetNextMessage(m))
				{
					if(!MessageProc(m) && outputPin)
					{
						outputPin->ForwardMessage(*m);
					}
				}
		if(Resource)
			Resource->ProcessMessages();
	}

}
