#ifndef GRAPHICS__DOMAIN__INCLUDED
#define GRAPHICS__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class GraphicsDomain : public DomainBase<GraphicsDomain>
{
	public:
		GraphicsDomain();
		~GraphicsDomain();
		void DomainStep(u64 step);

		static const guid& id;
		static const bool continuous;
	private:
		virtual bool MessageProc(const Message* pMessage);
};

}

#endif