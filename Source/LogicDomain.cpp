#include "header.h"
#include "UserInterface.h"
#include "LogicDomain.h"
#include "LActor.h"
#include "LSheet.h"

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
			Factory.RegisterObjectType<LogicSheet>();
			m_pUserInterface = new UserInterface();

			Vector3<> Init;
			
			for(int i =0;i<15;++i)
				Factory.CreateObject<LogicActor>(Init);

			SheetInit init;
			init.Center = Vector3<>(0.0,0.0,50.0f);
			init.HalfExtent = Vector3<>(-40.0f,-40.0f,0.0f);
			init.Normal = Vector3<>(0.0,0.0,1.0f);

			Factory.CreateObject<LogicSheet>(init);

			guids.resize(Factory.size());
			u32 nObjects2 = Factory.ForEachGetSome<guid,LogicActor>( [](LogicActor* o,guid& g) -> bool {return o->Step(g);} , &guids[0],(u32)guids.size());
			for(u32 i = 0; i<nObjects2; ++i)
			{
				Factory.DestroyObject(guids[i]);
				Vector3<> Init;
				Factory.CreateObject<LogicActor>(Init);
			}
			auto result = Factory.ForEach<int,LogicSheet>( [](LogicSheet* o) -> int {return o->Step();} );
		}

		bool LogicDomain::MessageProc(const Message* m)
		{
			if(m_pUserInterface->ProcessInputMessage(m))
				return true;
			return SharedDomainBase::MessageProc(m);
		}

		void LogicDomain::DomainStep(u64 step)
		{
			guids.resize(Factory.size());
			u32 nObjects2 = Factory.ForEachGetSome<guid,LogicActor>( [](LogicActor* o,guid& g) -> bool {return o->Step(g);} , &guids[0],guids.size());
			for(u32 i = 0; i<nObjects2; ++i)
			{
				Factory.DestroyObject(guids[i]);
				Vector3<> Init;
				Factory.CreateObject<LogicActor>(Init);
			}
			auto result = Factory.ForEach<int,LogicSheet>( [](LogicSheet* o) -> int {return o->Step();} );
		}

		LogicDomain::~LogicDomain()
		{

			delete m_pUserInterface;
			printf("Destroy Logic\n");
		}
}
