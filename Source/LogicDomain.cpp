#include "header.h"
#include "UserInterface.h"
#include "LogicDomain.h"
#include "LActor.h"
#include "LSheet.h"
#include "LLight.h"

#include "SettingsLoader.h"

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
			initSettings();
			Factory.RegisterObjectType<LogicActor>();
			Factory.RegisterObjectType<LogicSheet>();
			Factory.RegisterObjectType<LogicLight>();
			m_pUserInterface = new UserInterface();

			Vector3<> Init;
			
			for(int i =0;i<100;++i)
				Factory.CreateObject<LogicActor>(Init);

			SheetInit init;
			//init.Center = Vector3<>(0.0,0.0,50.0f);
			//init.HalfExtent = Vector3<>(-40.0f,-40.0f,0.0f);
			//init.Normal = Vector3<>(0.0,0.0,1.0f);
			/*init.Center = Vector3<>(0.0f,-10.0f,0.0f);
			init.HalfExtent = Vector3<>(-settings->getf32Setting("cloth_width")/2.0,0.0f,-settings->getf32Setting("cloth_width")/2.0);
			init.Normal = Vector3<>(0.0f,-1.0f,0.0f);*/

			//Factory.CreateObject<LogicSheet>(init);

			LightInit linit;
			linit.Position = Vector3<>(0.0,0.0,-50.0f);
			linit.Color = Vector3<>(1.0f,1.0f,1.0f);
			linit.Range = 200.0f;

			Factory.CreateObject<LogicLight>(linit);

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
			
			auto result2 = Factory.ForEach<int,LogicLight>( [](LogicLight* o) -> int {return o->Step();} );
		}

		LogicDomain::~LogicDomain()
		{

			delete m_pUserInterface;
			deleteSettings();
			printf("Destroy Logic\n");
		}
}
