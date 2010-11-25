#ifndef LOGIC__DOMAIN__INCLUDED
#define LOGIC__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class AIDomain;
class UserInterface;

class LogicDomain : public DomainBase<LogicDomain>
{
	public:
		LogicDomain();
		~LogicDomain();
		void DomainStep(u64 step);

		static const guid& id;
		static const bool continuous;
	private:
		
		virtual bool MessageProc(const Message* pMessage);

		InputPin<InputDomain,LogicDomain>	Input;
		InputPin<AIDomain,LogicDomain>		InputAI;
		OutputPin<LogicDomain>	Output;

		guid			testActorId;
		
		std::vector<Vector3<>>	positions;
		std::vector<guid>		guids;

		UserInterface*	m_pUserInterface;

		friend class SharedTaskManager;
};

}

#endif