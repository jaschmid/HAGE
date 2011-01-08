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
				for(int i=0;i<rand()%255;++i)f=f*f;
			}
		private:
		};

		GraphicsDomain::GraphicsDomain()
		{
			Factory.RegisterObjectType<GraphicsActor>();
			Factory.RegisterObjectType<GraphicsSheet>();
			Factory.RegisterObjectType<GraphicsLight>();
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

		void GraphicsDomain::DomainStep(u64 step)
		{
			auto result = Factory.ForEach<int,GraphicsActor>( [](GraphicsActor* o) -> int {return o->Step();} );
			auto result2 = Factory.ForEach<int,GraphicsSheet>( [](GraphicsSheet* o) -> int {return o->Step();} );
			auto result3 = Factory.ForEach<int,GraphicsLight>( [](GraphicsLight* o) -> int {return o->Step();} );
		}

		GraphicsDomain::~GraphicsDomain()
		{
			printf("Destroy Graphic\n");
		}
}
