#ifndef GENERATION__DOMAIN__INCLUDED
#define GENERATION__DOMAIN__INCLUDED

#include "header.h"
#include "DataProcessor.h"

#define FREEIMAGE_LIB
#include <FreeImage.h>
namespace HAGE {


class GenerationDomain : public DomainBase<GenerationDomain>
{
	public:
		GenerationDomain();
		~GenerationDomain();
		void DomainStep(t64 time);

	private:
		

		const static int xBegin = 28;//27;
		const static int xEnd = 36;//43;
		const static int yBegin = 28;//27;
		const static int yEnd = 36;//43;
		
		DataProcessor	_dataProc;

		virtual bool MessageProc(const Message* pMessage);
};

}

#endif