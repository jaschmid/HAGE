#include "header.h"
#include "UserInterface.h"
#include "LogicDomain.h"
#include "LActor.h"
#include "LSheet.h"
#include "LLight.h"

#include "SettingsLoader.h"

#include <boost/date_time.hpp>

const int nTasks = 1000;

static const char* MeshNames[3] = {
	"@art.mpq\\CREATURE\\DeathwingHuman\\DeathwingHuman.M2",
	"@art.mpq\\CREATURE\\AncientSeaTurtleFire\\AncientSeaTurtleFire.M2",
	"@art.mpq\\CREATURE\\Landro\\Landro.M2"
};

namespace HAGE {

		class LogicTask : public TaskManager::genericTask
		{
		public:
			void operator() ()
			{
				static float f=1.534523f;
				for(int i=0;i<GetRandInt()%0xffff;++i)f=f*f;
			}
		private:
		};

		LogicDomain::LogicDomain()  : positions(0)
		{
			printf("Init Logic\n");
			initSettings();
			GetFactory().RegisterObjectType<LogicActor>();
			GetFactory().RegisterObjectType<LogicSheet>();
			GetFactory().RegisterObjectType<LogicLight>();
			m_pUserInterface = new UserInterface();

			ActorInit boxinit;
			boxinit.mass = 0.0f;
			boxinit.behavior = 2;
			boxinit.location = Vector3<>(0.0f,0.0f,0.0f);
			strcpy(boxinit.mesh,"Box");
			boxinit.scale = Vector3<>(8.0f,8.0f,8.0f);
			GetFactory().CreateObject<LogicActor>(boxinit);

			ActorInit ainit;
			ainit.mass = 0.2f;

			Vector3<> ply_location(settings->getf32Setting("ply_spawn_x"),
									settings->getf32Setting("ply_spawn_y"),
									settings->getf32Setting("ply_spawn_z"));
			ainit.scale =Vector3<>(settings->getf32Setting("ply_scale_x"),
									settings->getf32Setting("ply_scale_y"),
									settings->getf32Setting("ply_scale_z"));
			float ply_range = settings->getf32Setting("ply_spawn_range");
			
			for(int i =0;i<settings->getu32Setting("num_ply_objs");++i)
			{	
				strcpy(ainit.mesh,MeshNames[GetRandInt()%3]);
				ainit.behavior = GetRandInt()%3;
				ainit.location = ply_location + Vector3<>((GetRandFloat()-0.5f)*2.0f,(GetRandFloat()-0.5f)*2.0f,(GetRandFloat()-0.5f)*2.0f)*ply_range;
		
				GetFactory().CreateObject<LogicActor>(ainit);
			}

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

			GetFactory().CreateObject<LogicSheet>(init);

			LightInit linit;
			linit.Position = Vector3<>( settings->getf32Setting("light1_x"),
										settings->getf32Setting("light1_y"),
										settings->getf32Setting("light1_z"));
			linit.Color = Vector3<>(settings->getf32Setting("light1_r"),
									settings->getf32Setting("light1_g"),
									settings->getf32Setting("light1_b"));
			linit.Range = settings->getf32Setting("light1_range");

			GetFactory().CreateObject<LogicLight>(linit);

			linit.Position = Vector3<>( settings->getf32Setting("light2_x"),
										settings->getf32Setting("light2_y"),
										settings->getf32Setting("light2_z"));
			linit.Color = Vector3<>(settings->getf32Setting("light2_r"),
									settings->getf32Setting("light2_g"),
									settings->getf32Setting("light2_b"));
			linit.Range = settings->getf32Setting("light2_range");

			GetFactory().CreateObject<LogicLight>(linit);

			linit.Position = Vector3<>( settings->getf32Setting("light3_x"),
										settings->getf32Setting("light3_y"),
										settings->getf32Setting("light3_z"));
			linit.Color = Vector3<>(settings->getf32Setting("light3_r"),
									settings->getf32Setting("light3_g"),
									settings->getf32Setting("light3_b"));
			linit.Range = settings->getf32Setting("light3_range");

			GetFactory().CreateObject<LogicLight>(linit);
		
			guids.resize(GetFactory().size());
			u32 nObjects2 = GetFactory().ForEachGetSome<guid,LogicActor>( [](LogicActor* o,guid& g) -> bool {return o->Step(g);} , &guids[0],(u32)guids.size());
			for(u32 i = 0; i<nObjects2; ++i)
			{
				GetFactory().DestroyObject(guids[i]);
				GetFactory().CreateObject<LogicActor>(ainit);
			}
			auto result = GetFactory().ForEach<int,LogicSheet>( [](LogicSheet* o) -> int {return o->Step();} );
		}

		bool LogicDomain::MessageProc(const Message* m)
		{
			if(m_pUserInterface->ProcessInputMessage(m))
				return true;
			return SharedDomainBase::MessageProc(m);
		}

		void LogicDomain::DomainStep(t64 time)
		{
			//printf("Time %f, Elapsed Time: %f\n",GetTime().toSeconds(),GetElapsedTime().toSeconds());

			guids.resize(GetFactory().size());
			u32 nObjects2 = GetFactory().ForEachGetSome<guid,LogicActor>( [](LogicActor* o,guid& g) -> bool {return o->Step(g);} , &guids[0],guids.size());
			for(u32 i = 0; i<nObjects2; ++i)
			{
				GetFactory().DestroyObject(guids[i]);		
				ActorInit ainit;	
				ainit.mass = 1.0f;
				strcpy(ainit.mesh,MeshNames[GetRandInt()%3]);
				ainit.scale =Vector3<>(settings->getf32Setting("ply_scale_x"),
									settings->getf32Setting("ply_scale_y"),
									settings->getf32Setting("ply_scale_z"));
				Vector3<> ply_location(settings->getf32Setting("ply_spawn_x"),
										settings->getf32Setting("ply_spawn_y"),
										settings->getf32Setting("ply_spawn_z"));
				float ply_range = settings->getf32Setting("ply_spawn_range");
				ainit.behavior = 0;
				ainit.location = ply_location + Vector3<>((GetRandFloat()-0.5f)*2.0f,(GetRandFloat()-0.5f)*2.0f,(GetRandFloat()-0.5f)*2.0f)*ply_range;
				GetFactory().CreateObject<LogicActor>(ainit);
			}
			auto result = GetFactory().ForEach<int,LogicSheet>( [](LogicSheet* o) -> int {return o->Step();} );
			
			auto result2 = GetFactory().ForEach<int,LogicLight>( [](LogicLight* o) -> int {return o->Step();} );
		}

		LogicDomain::~LogicDomain()
		{

			delete m_pUserInterface;
			deleteSettings();
			printf("Destroy Logic\n");
		}
}
