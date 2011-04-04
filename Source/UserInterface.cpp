#include "header.h"
#include "UserInterface.h"
/*
	MESSAGE_UI_UNKNOWN			= 0x00110000,
	MESSAGE_UI_CURSOR_UPDATE	= 0x00110001,
	MESSAGE_UI_ADJUST_CAMERA	= 0x00110002,
	MESSAGE_UI_MOVE_PLAYER		= 0x00110003,
	*/
namespace HAGE {

	UserInterface::UserInterface()
	{
		for(auto i = m_bKeyboardState.begin();i!=m_bKeyboardState.end();++i)
			*i=0;
		for(auto i = m_bMouseState.begin();i!=m_bMouseState.end();++i)
			*i=0;
	}
	UserInterface::~UserInterface()
	{
	}
	bool UserInterface::ProcessInputMessage(const Message* pMessage)
	{

		switch(pMessage->GetMessageCode())
		{
		case MESSAGE_INPUT_KEYDOWN:
			{
				MessageInputKeydown* m = (MessageInputKeydown*)pMessage;
				if(m->GetDevice() == guidDefKeyboard)
				{
					if(!m_bKeyboardState[m->GetKey()])
					{
						//keydown
						m_bKeyboardState[m->GetKey()] = 1;
					}
				}
				else if(m->GetDevice() == guidDefMouse)
				{
					if(!m_bMouseState[m->GetKey()])
					{
						//keydown
						m_bMouseState[m->GetKey()] = 1;
					}
				}
			}
			return false;
		case MESSAGE_INPUT_KEYUP:
			{
				MessageInputKeyup* m = (MessageInputKeyup*)pMessage;
				if(m->GetDevice() == guidDefKeyboard)
				{
					if(m_bKeyboardState[m->GetKey()])
					{
						//keyup
						m_bKeyboardState[m->GetKey()] = 0;
					}
				}
				else if(m->GetDevice() == guidDefMouse)
				{
					if(m_bMouseState[m->GetKey()])
					{
						//keyup
						m_bMouseState[m->GetKey()] = 0;
					}
				}
			}
			return false;
		case MESSAGE_INPUT_AXIS_RELATIVE:
			{
				MessageInputAxisRelative* m = (MessageInputAxisRelative*)pMessage;
				if(m->GetDevice() == guidDefMouse)
					switch(m->GetAxis())
					{
					case MOUSE_AXIS_X:
						if(m_bMouseState[MOUSE_BUTTON_1])
						{
							PostMessage(MessageUIAdjustCamera(m->GetChange(),0,0));
						}
						break;
					case MOUSE_AXIS_Y:
						if(m_bMouseState[MOUSE_BUTTON_1])
						{
							PostMessage(MessageUIAdjustCamera(0,m->GetChange(),0));
						}
						break;
					case MOUSE_AXIS_Z:
						PostMessage(MessageUIAdjustCamera(0,0,m->GetChange()));
						break;
					}
			}
			return false;
		case MESSAGE_INPUT_AXIS_ABSOLUTE:
			{
				MessageInputAxisAbsolute* m = (MessageInputAxisAbsolute*)pMessage;
			}
			return true;
		case MESSAGE_INPUT_RESET:
			for(auto i = m_bKeyboardState.begin();i!=m_bKeyboardState.end();++i)
				*i=0;
			for(auto i = m_bMouseState.begin();i!=m_bMouseState.end();++i)
				*i=0;
			return false;
		}
		return false;
	}
}
