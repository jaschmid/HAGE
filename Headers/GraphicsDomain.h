#ifndef GRAPHICS__DOMAIN__INCLUDED
#define GRAPHICS__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class GraphicsDomain : public DomainBase<GraphicsDomain>
{
	public:
		GraphicsDomain();
		~GraphicsDomain();
		void DomainStep(t64 time);

	private:
		virtual bool MessageProc(const Message* pMessage);
};

}

#endif