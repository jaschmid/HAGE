#include "header.h"
#include "UserInterfaceRendering.h"
/*
	MESSAGE_UI_UNKNOWN			= 0x00110000,
	MESSAGE_UI_CURSOR_UPDATE	= 0x00110001,
	MESSAGE_UI_ADJUST_CAMERA	= 0x00110002,
	MESSAGE_UI_MOVE_PLAYER		= 0x00110003,
	*/
namespace HAGE {

	UserInterfaceRendering::UserInterfaceRendering(RenderingAPIWrapper* pWrapper) : m_pWrapper(pWrapper)
	{
	}
	UserInterfaceRendering::~UserInterfaceRendering()
	{
	}
	bool UserInterfaceRendering::ProcessUserInterfaceMessage(const Message* pMessage)
	{
		return true;
	}

	void UserInterfaceRendering::Draw()
	{
	}
}