#include "header.h"
#include "GenericActor.h"
#include "ResourceDomain.h"
const int nTasks = 1000;

namespace HAGE {
	
		class ResourceTask : public TaskManager::genericTask
		{
		public:
			void operator() ()
			{
			}
		private:
		};
		
		void ResourceDomain::DomainStep(u64 step)
		{


		}

		ResourceDomain::ResourceDomain()
		{
			printf("Init Resource\n");
		}

		ResourceDomain::~ResourceDomain()
		{
			printf("Destroy Resource\n");
		}

		const guid& ResourceDomain::id = guidResourceDomain;
		const bool ResourceDomain::continuous = false;
}