#include <HAGE.h>
#include "InputDomain.h"
#include "TaskManager.h"
#include "SharedTaskManager.h"

namespace HAGE {
void __InternalHAGEMain();
}

#ifdef TARGET_WINDOWS
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <boost/uuid/string_generator.hpp>

struct RawInputDevice
{
	std::wstring		szName;
	HAGE::guid			hage_guid;
	RID_DEVICE_INFO		DevInfo;
};

namespace WindowsGlobal
{
	HINSTANCE instance;
	HWND hWnd;
	int CmdShow;
	HAGE::InputDomain* pInput = nullptr;
	bool		bShutdown= false;
    MSG         DestroyMessage;

	static const WORD MAX_CONSOLE_LINES = 500;

	//input stuff
	static const int RAW_INPUT_BUFFER_SIZE = 1024*65;
	static HAGE::u8 raw_input_buffer[RAW_INPUT_BUFFER_SIZE];
	std::map<HANDLE,RawInputDevice> InputDevices;
};

using namespace WindowsGlobal;

HWND GetHwnd()
{
	return hWnd;
}

HINSTANCE GetHInstance()
{
	return instance;
}

typedef struct _HIDP_CAPS
{
    USHORT    Usage;
    USHORT    UsagePage;
    USHORT   InputReportByteLength;
    USHORT   OutputReportByteLength;
    USHORT   FeatureReportByteLength;
    USHORT   Reserved[17];

    USHORT   NumberLinkCollectionNodes;

    USHORT   NumberInputButtonCaps;
    USHORT   NumberInputValueCaps;
    USHORT   NumberInputDataIndices;

    USHORT   NumberOutputButtonCaps;
    USHORT   NumberOutputValueCaps;
    USHORT   NumberOutputDataIndices;

    USHORT   NumberFeatureButtonCaps;
    USHORT   NumberFeatureValueCaps;
    USHORT   NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;


LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
)
{
	instance=hInstance;
	CmdShow=nCmdShow;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),	&coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
	coninfo.dwSize);

	// redirect unbuffered STDOUT to the console

	long lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	int hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	FILE* fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
    WNDCLASSEX  WndClsEx;

    WndClsEx.cbSize        = sizeof(WNDCLASSEX);
    WndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
    WndClsEx.lpfnWndProc   = WndProc;
    WndClsEx.cbClsExtra    = NULL;
    WndClsEx.cbWndExtra    = NULL;
    WndClsEx.hInstance     = instance;
    WndClsEx.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    WndClsEx.hCursor       = nullptr;
    WndClsEx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClsEx.lpszMenuName  = NULL;
    WndClsEx.lpszClassName = L"HAGE";
    WndClsEx.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&WndClsEx);

    hWnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
                          WndClsEx.lpszClassName,
                          L"HAGE MAIN WINDOW",
                          WS_CAPTION | WS_VISIBLE | WS_OVERLAPPED | WS_SYSMENU | WS_BORDER,
                          100,
                          120,
                          640,
                          480,
                          NULL,
                          NULL,
                          instance,
                          NULL);

    ShowWindow(hWnd, CmdShow);
    UpdateWindow(hWnd);

	// get all input devices

	RAWINPUTDEVICELIST	Devices[256];
	UINT nDevices = sizeof(Devices)/sizeof(RAWINPUTDEVICELIST);
	assert((nDevices=GetRawInputDeviceList(Devices,&nDevices,sizeof(RAWINPUTDEVICELIST))) > 0);


	// register for raw input
	RAWINPUTDEVICE rid;

	BYTE			preparsedBuffer[RAW_INPUT_BUFFER_SIZE];
	TCHAR			StringBuffer[1024];
	RID_DEVICE_INFO info;
	bool bMouseRegistered = false,bKeyboardRegistered = false;
	boost::uuids::string_generator gen;

	for(HAGE::u32 i=0;i<nDevices;++i)
	{
		UINT size = info.cbSize = sizeof(RID_DEVICE_INFO);
		UINT StringLength=1024;
		UINT nPreparsed= RAW_INPUT_BUFFER_SIZE;
		RawInputDevice HageDevice;
		size = GetRawInputDeviceInfo(Devices[i].hDevice,RIDI_DEVICEINFO,&info,&size);
		StringLength = GetRawInputDeviceInfo(Devices[i].hDevice,RIDI_DEVICENAME,StringBuffer,&StringLength);
		nPreparsed = GetRawInputDeviceInfo(Devices[i].hDevice,RIDI_PREPARSEDDATA,preparsedBuffer,&nPreparsed);
		if(StringLength == -1)
		{
			wprintf(L"Could not Get Device Name %08x\n",Devices[i].hDevice);
			continue;
		}
		else if(size == -1)
		{
			wprintf(L"Could not Get Device Info for \"%s\"\n",StringBuffer);
			continue;
		}
		else if(nPreparsed == -1)
		{
			wprintf(L"Could not Preparsed Data for for \"%s\"\n",StringBuffer);
			continue;
		}

		// try to register it
		if(Devices[i].dwType == RIM_TYPEKEYBOARD)
		{
			rid.usUsagePage = 0x01; 
			rid.usUsage = 0x06; 

			if(!bKeyboardRegistered) // only once for all keyboards
			{
				// register keyboard
				rid.dwFlags = RIDEV_NOHOTKEYS | RIDEV_APPKEYS | RIDEV_NOLEGACY;   // adds HID keyboard and also ignores legacy keyboard messages
				rid.hwndTarget = hWnd;

				assert(RegisterRawInputDevices(&rid, 1, sizeof(rid)));
				bKeyboardRegistered = true;
			}
		}
		else if(Devices[i].dwType == RIM_TYPEMOUSE)
		{
			rid.usUsagePage = 0x01; 
			rid.usUsage = 0x02; 
			
			if(!bMouseRegistered) // only once for all mice
			{
				// register mouse
				rid.dwFlags = 0;   // adds HID mouse and also ignores legacy mouse messages
				rid.hwndTarget = hWnd;

				assert(RegisterRawInputDevices(&rid, 1, sizeof(rid)));
				bMouseRegistered = true;
			}
		}
		else
		{
			// first 8 bytes = "HiDP KDR"
			HIDP_CAPS* pCaps = (HIDP_CAPS*)&preparsedBuffer[8];

			if(info.hid.usUsagePage == 0xd)
				continue;

			rid.usUsagePage = info.hid.usUsagePage; 
			rid.usUsage = info.hid.usUsage; 
			rid.dwFlags = RIDEV_NOLEGACY;   // adds HID mouse and also ignores legacy mouse messages
			rid.hwndTarget = hWnd;

			// no HID support atm
			continue;

			if(!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
			{
				rid.dwFlags = 0; // try without NOLEGACY
				if(!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
				{
					wprintf(L"Could not register HID Device for Raw Input: \"%s\"\n",StringBuffer);
					continue;
				}
			}

			// try to read preparsed data
			/*HIDP_BUTTON_CAPS t[256];
			USHORT n=256;
			HidP_GetButtonCaps(HidP_Input,t,&n,(HIDP_PREPARSED_DATA*)preparsedBuffer);*/
		}

		HageDevice.DevInfo = info;
		HageDevice.szName = std::wstring(StringBuffer);
		HageDevice.hage_guid = *(HAGE::guid*)gen(StringBuffer).data;

		InputDevices.insert(std::pair<HANDLE,RawInputDevice>(Devices[i].hDevice,HageDevice));

		wprintf(L"Registered: \"%s\"\n",HageDevice.szName.c_str());
	}

	HAGE::SharedTaskManager* pSharedTaskManager =new HAGE::SharedTaskManager();
	
	pInput = pSharedTaskManager->StartThreads();

	// Message queue
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
    MSG         Msg;
    while( GetMessage(&Msg, NULL, 0, 0) )
    {
		if( Msg.message == WM_CLOSE )
		{
			bShutdown= true;	
			pSharedTaskManager->StopThreads();
		}
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);

	delete pSharedTaskManager;
	
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(bShutdown)
	{
		if(Msg == WM_DESTROY)
			PostQuitMessage(WM_QUIT);
		return DefWindowProc(hWnd,Msg,wParam,lParam);
	}

    switch(Msg)
    {
	case WM_ACTIVATE:
		if(wParam & WA_ACTIVE || wParam & WA_CLICKACTIVE)
		{
			ShowCursor(FALSE);
			RECT client;
			GetClientRect(hWnd,&client);
			ClientToScreen(hWnd,(LPPOINT)&client.left);
			ClientToScreen(hWnd,(LPPOINT)&client.right);
			ClipCursor(&client);
		}
		else
		{
			ShowCursor(TRUE);
			ClipCursor(nullptr);
			pInput->PostInputMessage(HAGE::MessageInputReset());
		}
		return 0;
	case WM_INPUT:
		{
			UINT nSize = RAW_INPUT_BUFFER_SIZE;
			nSize=GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw_input_buffer,&nSize,sizeof(RAWINPUTHEADER));
			RAWINPUT* raw =(RAWINPUT*)raw_input_buffer;

			std::map<HANDLE,RawInputDevice>::iterator it = InputDevices.find(raw->header.hDevice);

			switch(raw->header.dwType)
			{
			case RIM_TYPEKEYBOARD:
				{
					HAGE::u32 e = 0;
					if(raw->data.keyboard.Flags & RI_KEY_E0)
						e |= 0x00000100;
					if(raw->data.keyboard.Flags & RI_KEY_E1)
						e |= 0x00000200;
					if(raw->data.keyboard.Flags & RI_KEY_BREAK)
					{
						pInput->PostInputMessage(HAGE::MessageInputKeyup(HAGE::guidDefKeyboard,e|(HAGE::u32)(raw->data.keyboard.MakeCode&0xff),0));
					}
					else
					{
						pInput->PostInputMessage(HAGE::MessageInputKeydown(HAGE::guidDefKeyboard,e|(HAGE::u32)(raw->data.keyboard.MakeCode&0xff),0));
					}
				}
				break;
			case RIM_TYPEMOUSE:
				{
					if(raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
					{
						// wheel move
						pInput->PostInputMessage(HAGE::MessageInputAxisRelative(HAGE::guidDefMouse,HAGE::MOUSE_AXIS_Z,(float)*(SHORT*)&raw->data.mouse.usButtonData/(float)0xff));
					}
					else
					{
						// mouse movement
						if(! (raw->data.mouse.usFlags &MOUSE_MOVE_ABSOLUTE))
						{
							if(raw->data.mouse.lLastX != 0)
							{
								pInput->PostInputMessage(HAGE::MessageInputAxisRelative(HAGE::guidDefMouse,HAGE::MOUSE_AXIS_X,(float)*(LONG*)&raw->data.mouse.lLastX/(float)0xff));
							}
							if(raw->data.mouse.lLastY != 0)
							{
								pInput->PostInputMessage(HAGE::MessageInputAxisRelative(HAGE::guidDefMouse,HAGE::MOUSE_AXIS_Y,-1.0f*(float)*(LONG*)&raw->data.mouse.lLastY/(float)0xff));
							}
						}
						// button press buttons 1 to 5
						for(int i=0;i<5;++i)
						{
							if(raw->data.mouse.ulButtons & (1<<(i*2)))
							{
								//keydown
								pInput->PostInputMessage(HAGE::MessageInputKeydown(HAGE::guidDefMouse,HAGE::MOUSE_BUTTON_1+i,0));
							}
							else if(raw->data.mouse.ulButtons & (1<<(i*2+1)))
							{
								//keyup
								pInput->PostInputMessage(HAGE::MessageInputKeyup(HAGE::guidDefMouse,HAGE::MOUSE_BUTTON_1+i,0));
							}
						}
					}
				}
				break;
			case RIM_TYPEHID:
			default:
				{
					HAGE::guid device = it->second.hage_guid;
					wprintf(L"Key pressed on: \"%s\"",it->second.szName.c_str());
					int pos=0;
					for(HAGE::u32 i=0;i<raw->data.hid.dwCount;++i)
					{
						printf("\n");
						for(HAGE::u32 j=0;j<raw->data.hid.dwSizeHid;++j)
						{
							printf("%02x ",raw->data.hid.bRawData[pos]);
							++pos;
						}
					}
					printf("\n");
				}
				break;
			}
		}
		// need to do cleanup
		return DefWindowProc(hWnd, Msg, wParam, lParam);
    default:
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
    return 0;
}

extern void OSLeaveMessageQueue()
{
	assert(PostMessage(hWnd,WM_CLOSE,0,0));
}

#endif