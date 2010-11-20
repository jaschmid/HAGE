#include "header.h"
#include "GenericActor.h"
#include "InputDomain.h"
#include "UserInterface.h"

#include <boost/date_time.hpp>


namespace HAGE {
	/*
	struct UKey
	{
		u32 Key;
		guid Device;
	};

	struct Keybind
	{
		u32 Command;
		UKey Key,mod1,mod2,mod3;
	};

	const Keybind defaultKeySetup[] = {
		{ 0,	{MOUSE_AXIS_X,guidDefMouse},		{MOUSE_BUTTON_2,guidDefMouse},		{KEY_CODE_NONE,guidNull},		{KEY_CODE_NONE,guidNull} },
		{ 1,	{MOUSE_AXIS_Y,guidDefMouse},		{MOUSE_BUTTON_2,guidDefMouse},		{KEY_CODE_NONE,guidNull},		{KEY_CODE_NONE,guidNull} },
		{ 2,	{MOUSE_AXIS_Y,guidDefMouse},		{MOUSE_BUTTON_1,guidDefMouse},		{MOUSE_BUTTON_2,guidDefMouse},	{KEY_CODE_NONE,guidNull} }
	};*/

	//const u32 nKeybinds = sizeof(defaultKeySetup) / sizeof(Keybind);

		InputDomain::InputDomain()  :TestOut(Output.GetBasePin()),m_pInterface(new UserInterface(&Output.GetBasePin()))
		{
			printf("Init Input\n");
		}

		void InputDomain::DomainInit(u64 step)
		{
			*TestOut = (u32)step;
			printf("I%u",*TestOut);
			Output.PostMessage(SimpleMessage<MemHandle>(MESSAGE_ITEM_CREATED,TestOut.GetHandle()));
		}
		void InputDomain::DomainStep(u64 step)
		{
			// for now just forward input
			while(const Message* m=OSInputQueue.GetTopMessage())
			{
				if(failed(m_pInterface->ProcessInputMessage(m)))
					Output.PostMessage(*m);

				OSInputQueue.PopMessage();
			}

			*TestOut = (u32)step;

			//printf("<",step);

			static float f=1.534523f;
			for(int i=0;i<rand()%0xffff;++i)f=f*f;

			//printf(">",step);
		}

		InputDomain::~InputDomain()
		{
			delete m_pInterface;
			printf("Destroy Input\n");
		}

		const guid& InputDomain::id = guidInputDomain;
		const bool InputDomain::continuous = false;
}
