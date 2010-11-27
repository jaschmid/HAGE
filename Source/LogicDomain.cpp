#include "header.h"
#include "UserInterface.h"
#include "LogicDomain.h"
#include "LActor.h"

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

		LogicDomain::LogicDomain()  : positions(0)
		{
			printf("Init Logic\n");
			Factory.RegisterObjectType<LogicActor>();
			m_pUserInterface = new UserInterface();

			Vector3<> Init;
			Factory.CreateObject<LogicActor>(Init);
			
			for(int i =0;i<1500;++i)
			{
				LogicActor*		testActor;
				guid			testActorId;
				Factory.CreateObject<LogicActor>(Init);
			}

			guids.resize(Factory.size());
			u32 nObjects2 = Factory.ForEachGetSome<guid,LogicActor>( [](LogicActor* o,guid& g) -> bool {return o->Step(g);} , &guids[0],guids.size());
			for(int i = 0; i<nObjects2; ++i)
			{
				Factory.DestroyObject(guids[i]);
				Vector3<> Init;
				Factory.CreateObject<LogicActor>(Init);
			}
		}

		bool LogicDomain::MessageProc(const Message* m)
		{
			if(m_pUserInterface->ProcessInputMessage(m))
				return true;
			switch(m->GetMessageCode())
			{
				case 0:
				default:
					return SharedDomainBase::MessageProc(m);
			}
		}

		void LogicDomain::DomainStep(u64 step)
		{
			guids.resize(Factory.size());
			u32 nObjects2 = Factory.ForEachGetSome<guid,LogicActor>( [](LogicActor* o,guid& g) -> bool {return o->Step(g);} , &guids[0],guids.size());
			for(int i = 0; i<nObjects2; ++i)
			{
				Factory.DestroyObject(guids[i]);
				Vector3<> Init;
				Factory.CreateObject<LogicActor>(Init);
			}

		}

		LogicDomain::~LogicDomain()
		{

			delete m_pUserInterface;
			printf("Destroy Logic\n");
		}

		const guid& LogicDomain::id = guidLogicDomain;
		const bool LogicDomain::continuous = false;
}
