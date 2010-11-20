#include "header.h"
#include "UserInterface.h"
/*
	MESSAGE_UI_UNKNOWN			= 0x00110000,
	MESSAGE_UI_CURSOR_UPDATE	= 0x00110001,
	MESSAGE_UI_ADJUST_CAMERA	= 0x00110002,
	MESSAGE_UI_MOVE_PLAYER		= 0x00110003,
	*/
namespace HAGE {

	UserInterface::UserInterface(PinBase* pOut) : m_pPinOut(pOut),m_vCursor(0.0,0.0),m_bCursorVisible(true)
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

						if(m->GetKey() == KEY_CODE_ESC)
							GenerateShutdownMessage();

						printf("%08x\n",m->GetKey());
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
			return true;
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
						if(m->GetKey() == MOUSE_BUTTON_1)
						{
							m_bCursorVisible = true;
							m_pPinOut->PostMessage(MessageUICursorUpdate(m_vCursor.x,m_vCursor.y,m_bCursorVisible,true));
						}
					}
				}
			}
			return true;
		case MESSAGE_INPUT_AXIS_RELATIVE:
			{
				MessageInputAxisRelative* m = (MessageInputAxisRelative*)pMessage;
				if(m->GetDevice() == guidDefMouse)
					switch(m->GetAxis())
					{
					case MOUSE_AXIS_X:
						if(m_bMouseState[MOUSE_BUTTON_1])
						{
							m_pPinOut->PostMessage(MessageUIAdjustCamera(m->GetChange(),0,0));
							if(m_bCursorVisible)
							{
								m_bCursorVisible=false;
								m_pPinOut->PostMessage(MessageUICursorUpdate(m_vCursor.x,m_vCursor.y,m_bCursorVisible,true));
							}
						}
						else
						{
							m_vCursor.x+=m->GetChange();
							if(m_vCursor.x > 1.0f)
								m_vCursor.x = 1.0f;
							else if(m_vCursor.x < -1.0f)
								m_vCursor.x = -1.0f;
							m_pPinOut->PostMessage(MessageUICursorUpdate(m_vCursor.x,m_vCursor.y,m_bCursorVisible,true));
						}
						break;
					case MOUSE_AXIS_Y:
						if(m_bMouseState[MOUSE_BUTTON_1])
						{
							m_pPinOut->PostMessage(MessageUIAdjustCamera(0,m->GetChange(),0));
							if(m_bCursorVisible)
							{
								m_bCursorVisible=false;
								m_pPinOut->PostMessage(MessageUICursorUpdate(m_vCursor.x,m_vCursor.y,m_bCursorVisible,true));
							}
						}
						else
						{
							m_vCursor.y+=m->GetChange();
							if(m_vCursor.y > 1.0f)
								m_vCursor.y = 1.0f;
							else if(m_vCursor.y < -1.0f)
								m_vCursor.y = -1.0f;
							m_pPinOut->PostMessage(MessageUICursorUpdate(m_vCursor.x,m_vCursor.y,m_bCursorVisible,true));
						}
						break;
					case MOUSE_AXIS_Z:
						m_pPinOut->PostMessage(MessageUIAdjustCamera(0,0,m->GetChange()));
						break;
					}
			}
			return true;
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
			return true;
		}
		return false;
	}
}
