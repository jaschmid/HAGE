#include "HAGE.h"
#include "DebugUI.h"
/*
	MESSAGE_UI_UNKNOWN			= 0x00110000,
	MESSAGE_UI_CURSOR_UPDATE	= 0x00110001,
	MESSAGE_UI_ADJUST_CAMERA	= 0x00110002,
	MESSAGE_UI_MOVE_PLAYER		= 0x00110003,
	*/
namespace HAGE {

	DebugUI::DebugUI() : m_vCursor(0.0,0.0),m_bVisible(false)
	{
		ResetKeyBuffer();	
	}
	DebugUI::~DebugUI()
	{
	}
	void DebugUI::PostMessageUI(const Message& m)
	{
		RenderDebugUI::DebugUIControl.PostMessage(m);
	}
	void DebugUI::ResetKeyBuffer()
	{
		for(auto i = m_bKeyboardState.begin();i!=m_bKeyboardState.end();++i)
			*i=0;
		for(auto i = m_bMouseState.begin();i!=m_bMouseState.end();++i)
			*i=0;
	}
	bool DebugUI::ProcessInputMessage(const Message* pMessage)
	{
		if(!m_bVisible)
		{
			if(pMessage->GetMessageCode() == MESSAGE_INPUT_KEYDOWN && ((MessageInputKeydown*)pMessage)->GetKey() == KEY_CODE_ACCENT_GRAVE)
			{
				m_bVisible = true;
				PostMessageUI(MessageUIShow());
				ResetKeyBuffer();

				//lets not process that again
				m_bKeyboardState[KEY_CODE_ACCENT_GRAVE] = 1;
				//and then act as if visible
			}
			else
				return false;
		}

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

						if(m->GetKey() == KEY_CODE_ACCENT_GRAVE)
						{
							//hide!
							m_bVisible = false;
							PostMessageUI(MessageUIHide());
							return true;
						}
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
						{
							m_vCursor.x+=m->GetChange();
							if(m_vCursor.x > 1.0f)
								m_vCursor.x = 1.0f;
							else if(m_vCursor.x < -1.0f)
								m_vCursor.x = -1.0f;
							PostMessageUI(MessageUICursorUpdate(m_vCursor.x,m_vCursor.y,true,false));
						}
						break;
					case MOUSE_AXIS_Y:
						{
							m_vCursor.y+=m->GetChange();
							if(m_vCursor.y > 1.0f)
								m_vCursor.y = 1.0f;
							else if(m_vCursor.y < -1.0f)
								m_vCursor.y = -1.0f;
							PostMessageUI(MessageUICursorUpdate(m_vCursor.x,m_vCursor.y,true,false));
						}
						break;
					case MOUSE_AXIS_Z:
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
			ResetKeyBuffer();
			return true;
		}
		return true;
	}
}
