#ifndef RESOURCE__DOMAIN__INCLUDED
#define RESOURCE__DOMAIN__INCLUDED

#include "HAGE.h"

namespace HAGE {

class ResourceDomain : public get_traits<ResourceDomain>
{
	public:
		ResourceDomain();
		~ResourceDomain();
		void DomainStep(u64 step);

		static const guid& id;
		static const bool continuous;
	private:

};

}

#endif