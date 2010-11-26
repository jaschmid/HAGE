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


			LogicActor*		testActor;
			guid			testActorId;
			testActorId = Factory.CreateObject(guid_of<LogicActor>::Get(),(IObject**)&testActor);
			
			for(int i =0;i<1500;++i)
			{
				LogicActor*		testActor;
				guid			testActorId;
				testActorId = Factory.CreateObject(guid_of<LogicActor>::Get(),(IObject**)&testActor);
			}

			auto result = Factory.ForEach<Vector3<>,LogicActor>( [](LogicActor* o) -> Vector3<> {return o->Init();} , guidNull );
			positions.assign(result.first,&result.first[result.second]);
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
			std::vector<Vector3<>>& rpos=positions;
			/*
			for(int i =0;i<1;++i)
			{
				LogicActor*		testActor;
				guid			testActorId;
				testActorId = Factory.CreateObject(guid_of<LogicActor>::Get(),(IObject**)&testActor);
			}*/
			positions.resize(Factory.GetNumObjects());
			u32 nObjects = Factory.ForEachEx<Vector3<>,LogicActor>( [](LogicActor* o) -> Vector3<> {return o->Init();} , &positions[0], positions.size());
			guids.resize(nObjects);
			u32 nObjects2 = Factory.ForEachGetSome<guid,LogicActor>( [rpos](LogicActor* o,guid& g) -> bool {return o->Step(rpos,g);} , &guids[0],positions.size());
			for(int i = 0; i<nObjects2; ++i)
			{
				Factory.DestroyObject(guids[i]);
				LogicActor*		testActor;
				guid			testActorId;
				testActorId = Factory.CreateObject(guid_of<LogicActor>::Get(),(IObject**)&testActor);
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
