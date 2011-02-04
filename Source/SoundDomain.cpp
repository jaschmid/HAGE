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
				for(int i=0;i<GetRandInt()%0xffff;++i)f=f*f;
			}
		private:
		};

		bool SoundDomain::MessageProc(const Message* m)
		{
			switch(m->GetMessageCode())
			{
				case MESSAGE_UI_UNKNOWN:
				default:
					return SharedDomainBase::MessageProc(m);
			}
		}
		
		void SoundDomain::DomainStep(t64 time)
		{

		}

		SoundDomain::SoundDomain()
		{
			printf("Init Sound\n");
		}

		SoundDomain::~SoundDomain()
		{
			printf("Destroy Sound\n");
		}
}
