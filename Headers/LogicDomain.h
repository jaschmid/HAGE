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
		void DomainStep(t64 time);

	private:
		
		virtual bool MessageProc(const Message* pMessage);
		
		std::vector<Vector3<>>	positions;
		std::vector<guid>		guids;

		UserInterface*	m_pUserInterface;

		friend class SharedTaskManager;
};

}

#endif