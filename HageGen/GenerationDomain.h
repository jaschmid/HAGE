#ifndef GENERATION__DOMAIN__INCLUDED
#define GENERATION__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class GenerationDomain : public DomainBase<GenerationDomain>
{
	public:
		GenerationDomain();
		~GenerationDomain();
		void DomainStep(t64 time);

	private:
		
		virtual bool MessageProc(const Message* pMessage);
};

}

#endif