#ifndef AI__DOMAIN__INCLUDED
#define AI__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class AIDomain : public DomainBase<AIDomain>
{
	public:
		AIDomain();
		~AIDomain();
		void DomainInit(u64 step);
		void DomainStep(u64 step);

		static const guid& id;
		static const bool continuous;
	private:
		virtual bool MessageProc(const Message* pMessage);

		InputPin<LogicDomain,AIDomain>		Input;
		OutputPin<AIDomain>					Output;
		
		InputVar<u32> TestIn;
		OutputVar<u32>	TestOut;

		friend class SharedTaskManager;
};

}

#endif