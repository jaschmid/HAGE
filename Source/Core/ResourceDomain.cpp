#include "HAGE.h"
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

}