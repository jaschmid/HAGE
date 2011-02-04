#ifndef SOUND__DOMAIN__INCLUDED
#define SOUND__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class SoundDomain : public DomainBase<SoundDomain>
{
	public:
		SoundDomain();
		~SoundDomain();
		void DomainStep(t64 time);

	private:
		virtual bool MessageProc(const Message* pMessage);
		
		friend class SharedTaskManager;
};

}

#endif