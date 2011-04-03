#include "header.h"
#include "GraphicsDomain.h"
#include "GActor.h"
#include "GSheet.h"
#include "GLight.h"

const int nTasks = 1000;

namespace HAGE {
		class GraphicsTask : public TaskManager::genericTask
		{
		public:
			void operator() ()
			{
				static float f=1.534523f;
				for(int i=0;i<GetRandInt()%255;++i)f=f*f;
			}
		private:
		};

		GraphicsDomain::GraphicsDomain()
		{
			GetFactory().RegisterObjectType<GraphicsActor>();
			GetFactory().RegisterObjectType<GraphicsSheet>();
			GetFactory().RegisterObjectType<GraphicsLight>();
			printf("Init Graphic\n");
		}

		bool GraphicsDomain::MessageProc(const Message* m)
		{
			switch(m->GetMessageCode())
			{
				case MESSAGE_UI_UNKNOWN:
				default:
					return SharedDomainBase::MessageProc(m);
			}
		}

		void GraphicsDomain::DomainStep(t64 time)
		{
			auto result = GetFactory().ForEach<int,GraphicsActor>( [](GraphicsActor* o) -> int {return o->Step();} );
			auto result2 = GetFactory().ForEach<int,GraphicsSheet>( [](GraphicsSheet* o) -> int {return o->Step();} );
			auto result3 = GetFactory().ForEach<int,GraphicsLight>( [](GraphicsLight* o) -> int {return o->Step();} );
		}

		GraphicsDomain::~GraphicsDomain()
		{
			printf("Destroy Graphic\n");
		}
}
