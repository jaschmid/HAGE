#include <HAGE.h>
#include "InputDomain.h"
#include "TaskManager.h"
#include "SharedTaskManager.h"

namespace HAGE {
void __InternalHAGEMain();
}

#ifdef TARGET_LINUX
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/uuid/string_generator.hpp>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>

namespace UnixGlobal
{
	HAGE::InputDomain* pInput = nullptr;
    Display*    display;
    Window      win;
    GLXFBConfig bestFbc;
    bool bShutdown = false;
    Cursor      cursor;
}

using namespace UnixGlobal;


Window* GetWindow()
{
	return &win;
}

Display* GetPDisplay()
{
	return display;
}

GLXFBConfig GetPFBC()
{
	return bestFbc;
}

int main(int argc,char** argv)
{
    //XInitThreads();
    display = XOpenDisplay(0);

    if( !display)
    {
        printf( "Failed to open X display\n");
        exit(1);
    }

    // Get a matching FB config
    static int visual_attribs[] =
    {
      GLX_X_RENDERABLE    , True,
      GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
      GLX_RENDER_TYPE     , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
      GLX_RED_SIZE        , 8,
      GLX_GREEN_SIZE      , 8,
      GLX_BLUE_SIZE       , 8,
      GLX_ALPHA_SIZE      , 8,
      GLX_DEPTH_SIZE      , 24,
      GLX_STENCIL_SIZE    , 8,
      GLX_DOUBLEBUFFER    , True,
      //GLX_SAMPLE_BUFFERS  , 1,
      //GLX_SAMPLES         , 4,
      None
    };

    int glx_major, glx_minor;

    // FBConfigs were added in GLX version 1.3.
    if ( !glXQueryVersion( display, &glx_major, &glx_minor ) ||
       ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) )
    {
        printf( "Invalid GLX version" );
        exit(1);
    }

    printf( "Getting matching framebuffer configs\n" );
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig( display, DefaultScreen( display ),
                                        visual_attribs, &fbcount );
    if ( !fbc )
    {
        printf( "Failed to retrieve a framebuffer config\n" );
        exit(1);
    }
    printf( "Found %d matching FB configs.\n", fbcount );

    // Pick the FB config/visual with the most samples per pixel
    printf( "Getting XVisualInfos\n" );
    int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

    int i;
    for ( i = 0; i < fbcount; i++ )
    {
        XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
        if ( vi )
        {
          int samp_buf, samples;
          glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
          glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );

          printf( "  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d,"
                  " SAMPLES = %d\n",
                  i, vi -> visualid, samp_buf, samples );

          if ( best_fbc < 0 || samp_buf && samples > best_num_samp )
            best_fbc = i, best_num_samp = samples;
          if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
            worst_fbc = i, worst_num_samp = samples;
        }
        XFree( vi );
    }

    bestFbc = fbc[ best_fbc ];

    // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
    XFree( fbc );

    // Get a visual
    XVisualInfo *vi = glXGetVisualFromFBConfig( display, bestFbc );
    printf("FBC: %08x\n",bestFbc);
    printf( "Chosen visual ID = 0x%x\n", vi->visualid );

    printf( "Creating colormap\n" );
    XSetWindowAttributes swa;
    Colormap cmap;
    swa.colormap = cmap = XCreateColormap( display,
                                         RootWindow( display, vi->screen ),
                                         vi->visual, AllocNone );
    swa.background_pixmap = None ;
    swa.border_pixel      = 0;
    swa.event_mask        = StructureNotifyMask;

    printf( "Creating window\n" );
    win = XCreateWindow( display, RootWindow( display, vi->screen ),
                              100, 120, 640, 480, 0, vi->depth, InputOutput,
                              vi->visual,
                              CWBorderPixel|CWColormap|CWEventMask, &swa );
    if ( !win )
    {
        printf( "Failed to create window.\n" );
        exit(1);
    }

    // Done with the visual info data
    XFree( vi );

    XStoreName( display, win, "GL 3.0 Window" );


    XSelectInput( display, win, KeyPressMask|KeyReleaseMask|KeymapStateMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|StructureNotifyMask|FocusChangeMask);

    printf( "Mapping window\n" );
    XMapWindow( display, win );

    Pixmap blank;
    XColor dummy;
    char data[1] = {0};
    blank = XCreateBitmapFromData(display,win,data,1,1);
    if(blank == None) printf("out of memory\n");
    cursor = XCreatePixmapCursor(display,blank,blank,&dummy,&dummy,0,0);
    XFreePixmap(display,blank);

    while(1)
    {
        XEvent e;
        XNextEvent(display, &e);
        if(e.type == MapNotify);
            break;
    }

	HAGE::SharedTaskManager* pSharedTaskManager =new HAGE::SharedTaskManager();
	
	pInput = pSharedTaskManager->StartThreads();

	// Wait for shutdown
    while(!bShutdown)
    {
        usleep(1000000);
    }

	pSharedTaskManager->StopThreads();

    XDestroyWindow(display,win);

	delete pSharedTaskManager;

	return 0;
}

extern HAGE::u32 x_keycode_to_scancode(unsigned int keycode,Display* XDisplay);

extern void ProcessXEvents()
{
	if(bShutdown)
		return;
    while(XPending(display))
    {
        XEvent e;
        XNextEvent(display, &e);
        XWindowAttributes attributes;

        switch(e.type)
        {
            case FocusIn:
                // recieved focus, grab mouse pointer
                XGrabPointer(display,win,true,PointerMotionMask,GrabModeAsync,GrabModeAsync,win,None,CurrentTime);
                // also move it to center
                XGetWindowAttributes(display,win,&attributes);
                XWarpPointer(display,None,win,0,0,0,0,attributes.width/2,attributes.height/2);
                XDefineCursor(display,win,cursor);
                break;
            case KeyPress:
                pInput->PostInputMessage(HAGE::MessageInputKeydown(HAGE::guidDefKeyboard,x_keycode_to_scancode(e.xkey.keycode,display),0));
                break;
           case KeyRelease:
                pInput->PostInputMessage(HAGE::MessageInputKeyup(HAGE::guidDefKeyboard,x_keycode_to_scancode(e.xkey.keycode,display),0));
                break;
            case ButtonPress:
            case ButtonRelease:
                {
                    HAGE::u32 button=0;
                    switch(e.xbutton.button)
                    {
                        case Button1:button=HAGE::MOUSE_BUTTON_1;break;
                        case Button2:button=HAGE::MOUSE_BUTTON_2;break;
                        case Button3:button=HAGE::MOUSE_BUTTON_3;break;
                        case Button4:button=HAGE::MOUSE_BUTTON_4;break;
                        case Button5:button=HAGE::MOUSE_BUTTON_5;break;
                    }
                    if(e.type == ButtonPress)
                    {
                        //keydown
                        pInput->PostInputMessage(HAGE::MessageInputKeydown(HAGE::guidDefMouse,button,0));
                    }
                    else
                    {
                        //keyup
                        pInput->PostInputMessage(HAGE::MessageInputKeyup(HAGE::guidDefMouse,button,0));
                    }
                }
                break;
            case MotionNotify:
                {
                    XGetWindowAttributes(display,win,&attributes);
                    if( e.xmotion.x == attributes.width/2 && e.xmotion.y == attributes.height/2 )
                        break;//ignore

                    int dx = e.xmotion.x - attributes.width/2;
                    int dy = e.xmotion.y - attributes.height/2;

                    if(dx != 0)
                    {
                        pInput->PostInputMessage(HAGE::MessageInputAxisRelative(HAGE::guidDefMouse,HAGE::MOUSE_AXIS_X,(float)dx/(float)0xff));
                    }
                    if(dy != 0)
                    {
                        pInput->PostInputMessage(HAGE::MessageInputAxisRelative(HAGE::guidDefMouse,HAGE::MOUSE_AXIS_Y,-1.0f*(float)dy/(float)0xff));
                    }

                    //set to center
                    XWarpPointer(display,None,win,0,0,0,0,attributes.width/2,attributes.height/2);
                }
                break;
            case FocusOut:
                // lost focus, release mouse pointer
                XUngrabPointer(display,CurrentTime);
                XUndefineCursor(display,win);
                break;
        }
    }
}

extern void OSLeaveMessageQueue()
{
	bShutdown = true;
}

#endif
