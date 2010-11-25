#ifndef DEBUG_USER__INTERFACE__INCLUDED
#define DEBUG_USER__INTERFACE__INCLUDED

#include "HAGE.h"
#include "InputDomain.h"
#include "RenderDebugUI.h"
#include <array>

namespace HAGE {

class InputDomain;

class DebugUI : protected DomainMember<InputDomain>
{
public:
	DebugUI();
	~DebugUI();
	bool ProcessInputMessage(const Message* pMessage);
private:
	void PostMessageUI(const Message& m);
	void ResetKeyBuffer();
	static const u32 nKeyboardKeys = KEY_CODE_MAX+1;
	std::array<u8,nKeyboardKeys>		m_bKeyboardState;
	std::array<u8,6>					m_bMouseState;
	Vector2<>							m_vCursor;
	bool								m_bVisible;
};

}

#endif