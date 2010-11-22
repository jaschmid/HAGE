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

		LogicDomain::LogicDomain()  : InputAI(-2),positions(0)
		{			
			printf("Init Logic\n");
			Factory.RegisterObjectType<Actor<LogicDomain>>();
			m_pUserInterface = new UserInterface(&Output.GetBasePin());

			
			testActorId = Factory.CreateObject(Actor<LogicDomain>::getClassGuid(),(IObject**)&testActor);
			for(int i =0;i<500;++i)
			{
				Actor<LogicDomain>*	testActor;
				guid			testActorId;
				testActorId = Factory.CreateObject(Actor<LogicDomain>::getClassGuid(),(IObject**)&testActor);
			}

			auto result = Factory.ForEach<Vector3<>,Actor<LogicDomain>>( [](Actor<LogicDomain>* o) -> Vector3<> {return o->Init();} , guidNull );
			positions.assign(result.first,&result.first[result.second]);
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

		void LogicDomain::DomainStep(u64 step)
		{

			std::vector<Vector3<>>& rpos=positions;
						
			auto result = Factory.ForEach<Vector3<>,Actor<LogicDomain>>( [rpos](Actor<LogicDomain>* o) -> Vector3<> {return o->Step(rpos);} , guidNull);
			positions.assign(result.first,&result.first[result.second]);

		}

		LogicDomain::~LogicDomain()
		{
			
			delete m_pUserInterface;
			printf("Destroy Logic\n");
		}

		const guid& LogicDomain::id = guidLogicDomain;
		const bool LogicDomain::continuous = false;
}
