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
		void DomainStep(t64 time);

		void PostInputMessage(const Message& m)
		{
			OSInputQueue.PostMessage(m);
		}

		static const guid& id;
		static const bool continuous;
	private:
		// non synced input
		StaticMessageQueue<1024*128>	OSInputQueue;

		friend class SharedTaskManager;

		DebugUI*	m_pDebugInterface;
};

}

#endif