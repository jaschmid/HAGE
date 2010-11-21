#include "header.h"
#include "GenericActor.h"
#include "GraphicsDomain.h"

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
			Factory.RegisterObjectType<Actor<GraphicsDomain>>();
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

		void GraphicsDomain::DomainInit(u64 step)
		{
			printf("G");
		}

		void GraphicsDomain::DomainStep(u64 step)
		{

			//printf("{",step);

			static float f=1.534523f;
			for(int i=0;i<rand()%255;++i)f=f*f;
			GraphicsTask tasks[nTasks];
			for(int i=0;i<rand()%255;++i)
			{
				Tasks.QueueTask(&tasks[0]);
			}
			Tasks.Execute();


			//printf("}",step);

		}

		GraphicsDomain::~GraphicsDomain()
		{
			printf("Destroy Graphic\n");
		}

		const guid& GraphicsDomain::id = guidGraphicsDomain;
		const bool GraphicsDomain::continuous = false;
}
