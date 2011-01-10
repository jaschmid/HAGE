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
			
			for(int i =0;i<settings->getu32Setting("num_ply_objs");++i)
				Factory.CreateObject<LogicActor>(Init);

			SheetInit init;
			//init.Center = Vector3<>(0.0,0.0,50.0f);
			//init.HalfExtent = Vector3<>(-40.0f,-40.0f,0.0f);
			//init.Normal = Vector3<>(0.0,0.0,1.0f);
			
			init.Center = Vector3<>(	settings->getf32Setting("cloth_center_x"),
										settings->getf32Setting("cloth_center_y"),
										settings->getf32Setting("cloth_center_z"));
			init.HalfExtent = Vector3<>(settings->getf32Setting("cloth_half_extent_x"),
										settings->getf32Setting("cloth_half_extent_y"),
										settings->getf32Setting("cloth_half_extent_z"));
			init.Normal = Vector3<>(    settings->getf32Setting("cloth_init_normal_x"),
										settings->getf32Setting("cloth_init_normal_y"),
										settings->getf32Setting("cloth_init_normal_z"));

			//Factory.CreateObject<LogicSheet>(init);

			LightInit linit;
			linit.Position = Vector3<>( settings->getf32Setting("light1_x"),
										settings->getf32Setting("light1_y"),
										settings->getf32Setting("light1_z"));
			linit.Color = Vector3<>(settings->getf32Setting("light1_r"),
									settings->getf32Setting("light1_g"),
									settings->getf32Setting("light1_b"));
			linit.Range = settings->getf32Setting("light1_range");

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
