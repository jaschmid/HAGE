#ifndef AI__DOMAIN__INCLUDED
#define AI__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class AIDomain : public DomainBase<AIDomain>
{
	public:
		AIDomain();
		~AIDomain();
		void DomainStep(u64 step);

	private:
		virtual bool MessageProc(const Message* pMessage);

		friend class SharedTaskManager;
};

}

#endif