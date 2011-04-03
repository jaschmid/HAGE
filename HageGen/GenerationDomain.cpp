#include "header.h"

#include "GenerationDomain.h"


namespace HAGE {



		GenerationDomain::GenerationDomain() : _dataProc(xBegin,xEnd,yBegin,yEnd)
		{
			printf("Init Generation\n");
			
			FreeImage_Initialise();
		}

		bool GenerationDomain::MessageProc(const Message* m)
		{
			return SharedDomainBase::MessageProc(m);
		}		

		void GenerationDomain::DomainStep(t64 time)
		{

			if(_dataProc.Process())
				GetTask().Shutdown();
		}

		GenerationDomain::~GenerationDomain()
		{
			FreeImage_DeInitialise();
			printf("Destroy GenerationDomain\n");
		}
}
