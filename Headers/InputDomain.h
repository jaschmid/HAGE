#ifndef INPUT__DOMAIN__INCLUDED
#define INPUT__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class UserInterface;

class InputDomain : public DomainBase<InputDomain>
{
	public:
		InputDomain();
		~InputDomain();
		void DomainInit(u64 step);
		void DomainStep(u64 step);

		void PostInputMessage(const Message& m)
		{
			OSInputQueue.PostMessage(m);
		}

		static const guid& id;
		static const bool continuous;
	private:
		OutputPin<InputDomain>	Output;

		OutputVar<u32> TestOut;

		// non synced input
		StaticMessageQueue<1024*128>	OSInputQueue;

		friend class SharedTaskManager;

		UserInterface* m_pInterface;
};

}

#endif