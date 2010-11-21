#include "header.h"
#include "UserInterface.h"
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

		LogicDomain::LogicDomain()  : InputAI(-2)
		{			
			Factory.RegisterObjectType<Actor<LogicDomain>>();
			printf("Init Logic\n");
		}

		bool LogicDomain::MessageProc(const Message* m)
		{
			if(m_pUserInterface->ProcessInputMessage(m))
				return true;
			switch(m->GetMessageCode())
			{
				case MESSAGE_ITEM_CREATED:
				default:
					return SharedDomainBase::MessageProc(m);
			}
		}

		void LogicDomain::DomainInit(u64 step)
		{
			m_pUserInterface = new UserInterface(&Output.GetBasePin());
			testActorId = Factory.CreateObject(Actor<LogicDomain>::getClassGuid(),(IObject**)&testActor);
		}
		void LogicDomain::DomainStep(u64 step)
		{

			//printf("(",step);

			static float f=1.534523f;
			for(int i=0;i<rand()%255;++i)f=f*f;
			LogicTask tasks[nTasks];
			for(int i=0;i<rand()%255;++i)
			{
				Tasks.QueueTask(&tasks[0]);
			}
			Tasks.Execute();

			//printf(")",step);
		}
		void LogicDomain::DomainShutdown(u64 step)
		{
			delete m_pUserInterface;
		}

		LogicDomain::~LogicDomain()
		{
			printf("Destroy Logic\n");
		}

		const guid& LogicDomain::id = guidLogicDomain;
		const bool LogicDomain::continuous = false;
}
