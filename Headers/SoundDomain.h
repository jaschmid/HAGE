#ifndef SOUND__DOMAIN__INCLUDED
#define SOUND__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class SoundDomain : public DomainBase<SoundDomain>
{
	public:
		SoundDomain();
		~SoundDomain();
		void DomainInit(u64 step);
		void DomainStep(u64 step);

		static const guid& id;
		static const bool continuous;
	private:
		virtual bool MessageProc(const Message* pMessage);

		InputPin<GraphicsDomain,SoundDomain>	Input;
		InputPin<LogicDomain,SoundDomain>		InputDirect;
		
		InputVar<u32> TestIn;

		friend class SharedTaskManager;
};

}

#endif