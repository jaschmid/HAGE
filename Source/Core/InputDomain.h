#ifndef INPUT__DOMAIN__INCLUDED
#define INPUT__DOMAIN__INCLUDED

#include "HAGE.h"

namespace HAGE {

class DebugUI;

class InputDomain : public DomainBase<InputDomain>
{
	public:
		InputDomain();
		~InputDomain();
		void DomainStep(u64 step);

		void PostInputMessage(const Message& m)
		{
			OSInputQueue.PostMessage(m);
		}

		static const guid& id;
		static const bool continuous;
	private:
		OutputPin<InputDomain>	Output;

		// non synced input
		StaticMessageQueue<1024*128>	OSInputQueue;

		friend class SharedTaskManager;

		DebugUI*	m_pDebugInterface;
};

}

#endif