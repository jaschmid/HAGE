#include "header.h"

#include "GenerationDomain.h"

namespace HAGE {

		GenerationDomain::GenerationDomain() 
		{
			printf("Init Generation\n");
		}

		bool GenerationDomain::MessageProc(const Message* m)
		{
			return SharedDomainBase::MessageProc(m);
		}

		void GenerationDomain::DomainStep(t64 time)
		{
			//printf("Time %f, Elapsed Time: %f\n",GetTime().toSeconds(),GetElapsedTime().toSeconds());

		}

		GenerationDomain::~GenerationDomain()
		{

			printf("Destroy GenerationDomain\n");
		}
}
