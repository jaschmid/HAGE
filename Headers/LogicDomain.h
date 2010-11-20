#ifndef LOGIC__DOMAIN__INCLUDED
#define LOGIC__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class AIDomain;

class LogicDomain : public DomainBase<LogicDomain>
{
	public:
		LogicDomain();
		~LogicDomain();
		void DomainInit(u64 step);
		void DomainStep(u64 step);

		static const guid& id;
		static const bool continuous;
	private:
		
		virtual bool MessageProc(const Message* pMessage);

		InputPin<InputDomain,LogicDomain>	Input;
		InputPin<AIDomain,LogicDomain>		InputAI;
		OutputPin<LogicDomain>	Output;

		OutputVar<u32>	TestOut;
		InputVar<u32>	TestIn;
		InputVar<u32>	TestInAI;
		Actor<LogicDomain>*	testActor;
		guid			testActorId;

		friend class SharedTaskManager;
};

}

#endif