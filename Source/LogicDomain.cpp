#include "header.h"
//#include "InputDomain.h"
#include "GenericActor.h"
#include "LogicDomain.h"

#include <boost/date_time.hpp>

const int nTasks = 1000;

namespace HAGE {

		class LogicTask : public TaskManager::genericTask
		{
		public:
			void operator() ()
			{
				static float f=1.534523f;
				for(int i=0;i<rand()%0xffff;++i)f=f*f;
			}
		private:
		};

		LogicDomain::LogicDomain()  : InputAI(-2),TestOut(Output.GetBasePin()), TestIn(Input.GetBasePin()), TestInAI(InputAI.GetBasePin())
		{
			printf("Init Logic\n");
		}

		bool LogicDomain::MessageProc(const Message* m)
		{
			switch(m->GetMessageCode())
			{
				case MESSAGE_ITEM_CREATED:
					{
						const SimpleMessage<MemHandle>* item = (const SimpleMessage<MemHandle>*)m;
						if(m->GetSource() == guidInputDomain)
						{
							TestIn.Open(item->GetData());
						}
						else if(m->GetSource() == guidAIDomain)
							TestInAI.Open(item->GetData());
					}
					return true;
				default:
					return SharedDomainBase::MessageProc(m);
			}
		}

		void LogicDomain::DomainInit(u64 step)
		{
			testActorId = Factory.CreateObject(Actor<LogicDomain>::getClassGuid(),(IObject**)&testActor);
			Output.PostMessage(SimpleMessage<MemHandle>(MESSAGE_ITEM_CREATED,TestOut.GetHandle()));
			printf("L%u",*TestIn);
			*TestOut = *TestIn;
		}
		void LogicDomain::DomainStep(u64 step)
		{
			*TestOut = *TestIn;

			//printf("(",step);

			static float f=1.534523f;
			for(int i=0;i<rand()%255;++i)f=f*f;
			LogicTask tasks[nTasks];
			for(int i=0;i<rand()%255;++i)
			{
				Tasks.QueueTask(&tasks[0]);
			}
			Tasks.Execute();
			if(step >= 2)
			{
				//printf("%u-%u",*TestInAI,*TestOut);
				assert(*TestInAI == *TestOut -2);
			}

			//printf(")",step);
		}

		LogicDomain::~LogicDomain()
		{
			printf("Destroy Logic\n");
		}

		const guid& LogicDomain::id = guidLogicDomain;
		const bool LogicDomain::continuous = false;
}
