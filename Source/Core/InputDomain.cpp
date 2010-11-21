#include "HAGE.h"
#include "DebugUI.h"
#include "InputDomain.h"

#include <boost/date_time.hpp>


namespace HAGE {

		InputDomain::InputDomain()  :m_pDebugInterface(new DebugUI)
		{
			printf("Init Input\n");
		}

		void InputDomain::DomainInit(u64 step)
		{
			// for now just forward input
			while(const Message* m=OSInputQueue.GetTopMessage())
			{
				if(failed(m_pDebugInterface->ProcessInputMessage(m)))
					Output.PostMessage(*m);

				OSInputQueue.PopMessage();
			}
		}
		void InputDomain::DomainStep(u64 step)
		{
			// for now just forward input
			while(const Message* m=OSInputQueue.GetTopMessage())
			{
				if(!(m_pDebugInterface->ProcessInputMessage(m)))
					Output.PostMessage(*m);

				OSInputQueue.PopMessage();
			}

		}

		InputDomain::~InputDomain()
		{
			delete m_pDebugInterface;
		}

		const guid& InputDomain::id = guidInputDomain;
		const bool InputDomain::continuous = false;
}
