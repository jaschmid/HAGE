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

		GraphicsDomain::GraphicsDomain() : TestIn(LogicInput.GetBasePin()),TestOut(Output.GetBasePin())
		{
			printf("Init Graphic\n");
		}

		bool GraphicsDomain::MessageProc(const Message* m)
		{
			switch(m->GetMessageCode())
			{
				case MESSAGE_ITEM_CREATED:
					{
						const SimpleMessage<MemHandle>* item = (const SimpleMessage<MemHandle>*)m;
						TestIn.Open(item->GetData());
					}
					return true;
				default:
					SharedDomainBase::MessageProc(m);
					return true;
			}
		}

		void GraphicsDomain::DomainInit(u64 step)
		{
			Output.PostMessage(SimpleMessage<MemHandle>(MESSAGE_ITEM_CREATED,TestOut.GetHandle()));
			*TestOut = *TestIn;
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

			*TestOut = *TestIn;


			//printf("}",step);

		}

		GraphicsDomain::~GraphicsDomain()
		{
			printf("Destroy Graphic\n");
		}

		const guid& GraphicsDomain::id = guidGraphicsDomain;
		const bool GraphicsDomain::continuous = false;
}
