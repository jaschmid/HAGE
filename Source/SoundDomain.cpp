#include "header.h"
#include "GenericActor.h"
#include "SoundDomain.h"

namespace HAGE {

		class SoundTask : public TaskManager::genericTask
		{
		public:
			void operator() ()
			{
				static float f=1.534523f;
				for(int i=0;i<rand()%0xffff;++i)f=f*f;
			}
		private:
		};

		bool SoundDomain::MessageProc(const Message* m)
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
					return SharedDomainBase::MessageProc(m);
			}
		}

		void SoundDomain::DomainInit(u64 step)
		{
			printf("S");
		}

		void SoundDomain::DomainStep(u64 step)
		{

			//printf("!",step);

			static float f=1.534523f;
			for(int i=0;i<rand()%0xffff;++i)f=f*f;
			SoundTask tasks[255];
			for(int i=0;i<rand()%255;++i)
			{
				Tasks.QueueTask(&tasks[0]);
			}

			Tasks.Execute();

			//printf("?",step);

		}

		SoundDomain::SoundDomain() : TestIn(Input.GetBasePin())
		{
			printf("Init Rendering\n");
		}

		SoundDomain::~SoundDomain()
		{
			printf("Destroy Rendering\n");
		}

		const guid& SoundDomain::id = guidSoundDomain;
		const bool SoundDomain::continuous = false;
}
