#ifndef USER__INTERFACE__INCLUDED
#define USER__INTERFACE__INCLUDED

#include "header.h"
#include "InputDomain.h"
#include <array>

namespace HAGE {

class InputDomain;

class UserInterface : protected ObjectBase<InputDomain>
{
public:
	UserInterface(PinBase* pOut);
	~UserInterface();
	bool ProcessInputMessage(const Message* pMessage);
private:
	PinBase* m_pPinOut;
	static const u32 nKeyboardKeys = KEY_CODE_MAX+1;
	std::array<u8,nKeyboardKeys>		m_bKeyboardState;
	std::array<u8,6>					m_bMouseState;
	Vector2<>							m_vCursor;
	bool								m_bCursorVisible;
};

}

#endif