#include "header.h"
#include "GraphicsDomain.h"
#include "GActor.h"

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
			printf("Init Graphic\n");
		}

		bool GraphicsDomain::MessageProc(const Message* m)
		{
			switch(m->GetMessageCode())
			{
				case MESSAGE_UI_UNKNOWN:
				default:
					SharedDomainBase::MessageProc(m);
					return true;
			}
		}

		void GraphicsDomain::DomainStep(u64 step)
		{
			auto result = Factory.ForEach<int,GraphicsActor>( [](GraphicsActor* o) -> int {return o->Step();} , guidNull );
		}

		GraphicsDomain::~GraphicsDomain()
		{
			printf("Destroy Graphic\n");
		}
}
