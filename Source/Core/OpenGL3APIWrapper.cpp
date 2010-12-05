#include <HAGE.h>
#include "OpenGL3APIWrapper.h"
#include <string.h>
#include "ResourceDomain.h"

#ifndef NO_OGL

#ifdef TARGET_WINDOWS

HWND GetHwnd();
HINSTANCE GetHInstance();

#elif defined(TARGET_LINUX)

Window* GetWindow();
Display* GetPDisplay();
GLXFBConfig GetPFBC();

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static bool ctxErrorOccurred = false;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    ctxErrorOccurred = true;
    return 0;
}

// we have to do Event processing on same thread as OGL calls on LINUX (fail)

void ProcessXEvents();

#endif

void OpenGL3APIWrapper::checkForCgError(const char *situation)
{
  CGerror error;
  const char *string = cgGetLastErrorString(&error);

  if (error != CG_NO_ERROR) {
    printf("%s: %s: %s\n",
      "HAGE", situation, string);
    if (error == CG_COMPILER_ERROR) {
      printf("%s\n", cgGetLastListing(myCgContext));
    }
    exit(1);
  }
}

HAGE::RenderingAPIWrapper* HAGE::RenderingAPIWrapper::CreateOpenGL3Wrapper()
{
	RenderingAPIWrapper* pResult = new OpenGL3APIWrapper();
	_pAllocator= pResult;
	HAGE::domain_access<ResourceDomain>::Get()->_RegisterResourceType(guid_of<IDrawableMesh>::Get(),&CDrawableMeshLoader::Initialize);
	return pResult;
}

OpenGL3APIWrapper::OpenGL3APIWrapper() :
#ifdef TARGET_WINDOWS
	m_hInst(GetHInstance()),
	m_hWnd(GetHwnd()),
	m_hDC(GetDC(m_hWnd)),
#elif defined(TARGET_LINUX)
    m_hrc(0),
    m_pWindow(GetWindow()),
    m_pDisplay(GetPDisplay()),
#endif
	m_NextVertexFormatEntry(0),
	m_CurrentRS(-1),
	m_CurrentBS(-1)
{
	myCgContext = cgCreateContext();
	checkForCgError("creating context");

#ifdef TARGET_WINDOWS
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);

	assert(nPixelFormat != 0);

	BOOL bResult = SetPixelFormat (m_hDC, nPixelFormat, &pfd);

	assert(bResult);

	HGLRC tempContext = wglCreateContext(m_hDC);
	wglMakeCurrent(m_hDC, tempContext);

	GLenum err = glewInit();
	assert(GLEW_OK == err);

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0
	};

    if(wglewIsSupported("WGL_ARB_create_context") == 1)
    {
		m_hrc = wglCreateContextAttribsARB(m_hDC,0, attribs);
		m_hrcL = wglCreateContextAttribsARB(m_hDC,m_hrc, attribs);
		wglMakeCurrent(NULL,NULL);
		wglDeleteContext(tempContext);
		//wglCreateContextAttribsARB(m_hDC,m_hrc, attribs);
		assert(m_hrc);
		assert(m_hrcL);
		wglMakeCurrent(m_hDC, m_hrc);
	}
	else
	{	//It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)
		m_hrc = tempContext;		
		m_hrcL = wglCreateContext(m_hDC);
		wglShareLists(m_hrc,m_hrcL);
	}

	assert(m_hrc);
#elif defined(TARGET_LINUX)
    printf("begin init OGL\n");

    // Get the default screen's GLX extension list
    const char *glxExts = glXQueryExtensionsString( m_pDisplay,
                                                  DefaultScreen( m_pDisplay ) );

    // NOTE: It is not necessary to create or make current to a context before
    // calling glXGetProcAddressARB
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
           glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );


    // Install an X error handler so the application won't exit if GL 3.0
    // context allocation fails.
    //
    // Note this error handler is global.  All display connections in all threads
    // of a process use the same error handler, so be sure to guard against other
    // threads issuing X commands while this code is running.
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) =
      XSetErrorHandler(&ctxErrorHandler);

    // Check for the GLX_ARB_create_context extension string and the function.
    // If either is not present, use GLX 1.3 context creation method.
    if (!glXCreateContextAttribsARB )
    {
        printf( "glXCreateContextAttribsARB() not found"
                " ... using old-style GLX context\n" );
        if(!glXCreateNewContext)
        {
            printf("Holy moly no OpenGL on system!\n");
        }
        else
        {
            m_hrc= glXCreateNewContext( m_pDisplay, GetPFBC(), GLX_RGBA_TYPE, 0, True );
        }
    }

    // If it does, try to get a GL 3.0 context!
    else
    {
        int context_attribs[] =
          {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 2,
            GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            None
          };

        printf( "Creating context\n" );
        m_hrc = glXCreateContextAttribsARB( m_pDisplay, GetPFBC(), 0,
                                          True, context_attribs );

        // Sync to ensure any errors generated are processed.
        XSync( m_pDisplay, False );
        if ( !ctxErrorOccurred && m_hrc )
		{
          printf( "Created GL 3.0 context\n" );
	      m_hrcL = glXCreateContextAttribsARB( m_pDisplay, GetPFBC(), m_hrc,
                                          True, context_attribs );		
		  assert(m_hrcL);
		}
        else
        {
          // Couldn't create GL 3.0 context.  Fall back to old-style 2.x context.
          // When a context version below 3.0 is requested, implementations will
          // return the newest context version compatible with OpenGL versions less
          // than version 3.0.
          // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
          context_attribs[1] = 1;
          // GLX_CONTEXT_MINOR_VERSION_ARB = 0
          context_attribs[3] = 0;

          ctxErrorOccurred = false;

          printf( "Failed to create GL 3.0 context"
                  " ... using old-style GLX context\n" );
          m_hrc = glXCreateContextAttribsARB( m_pDisplay, GetPFBC(), 0,
                                            True, context_attribs );
          m_hrcL = glXCreateContextAttribsARB( m_pDisplay, GetPFBC(), m_hrc,
                                            True, context_attribs );

		  assert(m_hrc);
		  assert(m_hrcL);
		}
    }

    // Sync to ensure any errors generated are processed.
    XSync( m_pDisplay, False );

    // Restore the original error handler
    XSetErrorHandler( oldHandler );

    if ( ctxErrorOccurred || !m_hrc )
    {
        printf( "Failed to create an OpenGL context\n" );
        exit(1);
    }

    // Verifying that context is a direct context
    if ( ! glXIsDirect ( m_pDisplay, m_hrc ) )
    {
        printf( "Indirect GLX rendering context obtained\n" );
    }
    else
    {
        printf( "Direct GLX rendering context obtained\n" );
    }

    printf( "Making context current\n" );
    glXMakeCurrent( m_pDisplay, *m_pWindow, m_hrc );

	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
	    printf("Glew failed to initialize: %s\n",glewGetErrorString(err));
	    exit(1);
	}
#endif
	//Checking GL version
	const char *GLVersionString = (const char*)glGetString(GL_VERSION);

	//Or better yet, use the GL3 way to get the version number
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

	printf("Initializing Open GL %i.%i\n",OpenGLVersion[0],OpenGLVersion[1]);
	
	 //prepare the loading context
	glEnableClientState(GL_VERTEX_ARRAY);

#ifdef TARGET_WINDOWS
	//disable VSync
	wglSwapIntervalEXT(0);
#endif

	glClearColor(0.5f, 0.1f, 0.2f, 0.0f);  /* Red background */

	myCgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	cgGLSetOptimalOptions(myCgVertexProfile);
	myCgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(myCgFragmentProfile);
	cgSetParameterSettingMode(myCgContext, CG_DEFERRED_PARAMETER_SETTING);

	SetRasterizerState(GetRasterizerStateCode(&HAGE::DefaultRasterizerState));
	SetBlendState(GetBlendStateCode(&HAGE::DefaultBlendState,1,false));
	
	glFlush();
#ifdef TARGET_WINDOWS
	wglMakeCurrent(NULL, NULL);
	wglMakeCurrent(m_hDC, m_hrcL);
#elif defined(TARGET_LINUX)
    glXMakeCurrent( m_pDisplay, 0, 0 );
	glXMakeCurrent( m_pDisplay, *m_pWindow, m_hrcL );
#endif

	 //prepare the loading context
	glEnableClientState(GL_VERTEX_ARRAY);

#ifdef TARGET_WINDOWS
	//disable VSync
	wglSwapIntervalEXT(0);
#endif

	glClearColor(0.5f, 0.1f, 0.2f, 0.0f);  /* Red background */

	cgGLSetDebugMode(CG_FALSE);
	cgSetParameterSettingMode(myCgContext, CG_DEFERRED_PARAMETER_SETTING);

	cgGLSetOptimalOptions(myCgVertexProfile);
	checkForCgError("selecting vertex profile");

	cgGLSetOptimalOptions(myCgFragmentProfile);
	checkForCgError("selecting fragment profile");

	SetRasterizerState(GetRasterizerStateCode(&HAGE::DefaultRasterizerState));
	SetBlendState(GetBlendStateCode(&HAGE::DefaultBlendState,1,false));
	
	m_DebugUIRenderer = new HAGE::RenderDebugUI(this);
	
	glFlush();
#ifdef TARGET_WINDOWS
	wglMakeCurrent(NULL, NULL);
#elif defined(TARGET_LINUX)
    glXMakeCurrent( m_pDisplay, 0, 0 );
#endif
}

OpenGL3APIWrapper::~OpenGL3APIWrapper()
{
	delete m_DebugUIRenderer;

    cgDestroyContext( myCgContext );

	// Cleanup OpenGL
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

#ifdef TARGET_WINDOWS
	wglMakeCurrent(NULL, NULL);
	if(m_hrc)
	{
		wglDeleteContext(m_hrc);
		m_hrc = NULL;
	}
	if(m_hrcL)
	{
		wglDeleteContext(m_hrcL);
		m_hrcL = NULL;
	}

#elif defined(TARGET_LINUX)

    glXMakeCurrent( m_pDisplay, 0, 0 );
    glXDestroyContext(m_pDisplay,m_hrc);
	glXDestroyContext(m_pDisplay,m_hrcL);
#endif
}

void OpenGL3APIWrapper::BeginAllocation()
{
#ifdef TARGET_WINDOWS
	if(!wglMakeCurrent(m_hDC, m_hrcL))
	{
		printf("Could not begin allocation");
	}
#elif defined(TARGET_LINUX)
    glXMakeCurrent( m_pDisplay, *m_pWindow, m_hrcL );
#endif
}

void OpenGL3APIWrapper::EndAllocation()
{
	glFlush();
#ifdef TARGET_WINDOWS
	if(!wglMakeCurrent(NULL, NULL))
	{
		printf("Could not begin allocation");
	}
#elif defined(TARGET_LINUX)
    glXMakeCurrent( m_pDisplay, 0, 0 );
#endif
}

void OpenGL3APIWrapper::BeginFrame()
{
#ifdef TARGET_WINDOWS
	if(!wglMakeCurrent(m_hDC, m_hrc))
	{
		printf("Could not begin frame");
	}
#elif defined(TARGET_LINUX)
    glXMakeCurrent( m_pDisplay, *m_pWindow, m_hrc );
    // might aswell do that here...
    ProcessXEvents();
#endif
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	cgGLEnableProfile(GetVertexProfile());
	checkForCgError("enabling vertex profile");

	cgGLEnableProfile(GetFragmentProfile());
	checkForCgError("enabling fragment profile");
}

void OpenGL3APIWrapper::PresentFrame()
{
	// Debug UI
	m_DebugUIRenderer->Draw();

	cgGLDisableProfile(GetVertexProfile());
	checkForCgError("disabling vertex profile");

	cgGLDisableProfile(GetFragmentProfile());
	checkForCgError("disabling fragment profile");
	
	glFlush();

#ifdef TARGET_WINDOWS
	SwapBuffers(m_hDC);

	if(!wglMakeCurrent(NULL, NULL))
	{
		printf("Could not end frame");
	}

	//framerate hack yay
    static HAGE::u64 last = 0;
	static HAGE::u64 freq = 0;
    static float sum = 0;
    static int nSum = 0;
    HAGE::u64 current;
    QueryPerformanceCounter((LARGE_INTEGER*)&current);

    HAGE::u64 diff = current - last;
    last = current;

	if(freq == 0)
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

    sum +=1.0f/(float)diff*(float)freq;
    nSum++;

    if(nSum%100==0)
        printf("Average %.02f - current %.02f\n",sum/(float)nSum,1.0f/(float)diff*(float)freq);

#elif defined(TARGET_LINUX)
    glXSwapBuffers(m_pDisplay,*m_pWindow);
    glXMakeCurrent( m_pDisplay, 0, 0 );

    //framerate hack yay
    static timeval last = {0,0};
    static float sum = 0;
    static int nSum = 0;
    timeval current;
    gettimeofday(&current,NULL);

    long sec = current.tv_sec - last.tv_sec;
    long usec = current.tv_usec - last.tv_usec;
    if(sec!=0)
    {
        usec += sec*1000000;
    }
    last = current;

    sum +=1.0f/(float)usec*(float)1000000;
    nSum++;

    if(nSum%100==0)
        printf("Average %.02f - current %.02f\n",sum/(float)nSum,1.0f/(float)usec*(float)1000000);

#endif
}

HAGE::APIWVertexBuffer* OpenGL3APIWrapper::CreateVertexBuffer(const char* szVertexFormat,const void* pData,HAGE::u32 nElements,bool bInstanceData)
{
	return new OGL3VertexBuffer(this,szVertexFormat,pData,nElements,bInstanceData);
}

HAGE::APIWConstantBuffer* OpenGL3APIWrapper::CreateConstantBuffer(HAGE::u32 nSize)
{
	return new OGL3ConstantBuffer(this,nSize);
}

HAGE::APIWEffect* OpenGL3APIWrapper::CreateEffect(const char* pVertexProgram,const char* pFragmentProgram,
		const HAGE::APIWRasterizerState* pRasterizerState, const HAGE::APIWBlendState* pBlendState,
		const HAGE::u32 nBlendStates, bool AlphaToCoverage)
{
	return new OGL3Effect(this,pVertexProgram,pFragmentProgram,GetRasterizerStateCode(pRasterizerState),GetBlendStateCode(pBlendState,nBlendStates,AlphaToCoverage));
}

HAGE::APIWVertexArray* OpenGL3APIWrapper::CreateVertexArray(HAGE::u32 nPrimitives,
		HAGE::APIWPrimitiveType PrimitiveType,
		HAGE::APIWVertexBuffer** pBuffers,
		HAGE::u32 nBuffers,
		const HAGE::u32* pIndexBufferData)
{
	return new OGL3VertexArray(this,nPrimitives,PrimitiveType,pBuffers,nBuffers,pIndexBufferData);
}

HAGE::u8	OpenGL3APIWrapper::GetVertexFormatCode(const char* name)
{
	std::string s_name(name);
	auto item=m_VertexStringTable.find(s_name);
	assert(item != m_VertexStringTable.end());
	return item->second;
}

HAGE::u32	OpenGL3APIWrapper::GetVertexSize(HAGE::u8 code)
{
	return m_VertexFormatList[code].uVertexSize;
}

const OpenGL3APIWrapper::VertexFormatEntry*	OpenGL3APIWrapper::GetVertexFormat(HAGE::u8 code)
{
	return &m_VertexFormatList[code];
}

void OpenGL3APIWrapper::RegisterVertexFormat(const char* szName,const HAGE::VertexDescriptionEntry* pDescription,HAGE::u32 nNumEntries)
{
	std::string s_name(szName);
	assert( m_VertexStringTable.find(s_name) == m_VertexStringTable.end());
	assert( m_NextVertexFormatEntry <= 255 );

	VertexFormatEntry& new_entry = m_VertexFormatList[m_NextVertexFormatEntry];


	new_entry.pOriginalDescription = new HAGE::VertexDescriptionEntry[nNumEntries];
	memcpy(new_entry.pOriginalDescription,pDescription,sizeof(HAGE::VertexDescriptionEntry)*nNumEntries);

	new_entry.nElements = nNumEntries;

	int out = 0;
	int vertex_size = 0;
	for(HAGE::u32 i=0;i<nNumEntries;++i)
		for(HAGE::u32 j=0;j<pDescription[i].nSubElements;++j,++out)
		{
			switch(new_entry.pOriginalDescription[i].fFormat)
			{
			case HAGE::R32G32B32A32_FLOAT:
				vertex_size+=sizeof(HAGE::f32)*4;
				break;
			case HAGE::R32G32B32_FLOAT:
				vertex_size+=sizeof(HAGE::f32)*3;
				break;
			case HAGE::R32G32_FLOAT:
				vertex_size+=sizeof(HAGE::f32)*2;
				break;
			case HAGE::R32_FLOAT:
				vertex_size+=sizeof(HAGE::f32)*1;
				break;
			}
		}

	new_entry.uVertexSize = vertex_size;

	m_VertexStringTable.insert(std::pair<std::string,HAGE::u8>(s_name,(HAGE::u8)m_NextVertexFormatEntry));

	++m_NextVertexFormatEntry;
}

HAGE::u16	OpenGL3APIWrapper::GetRasterizerStateCode(const HAGE::APIWRasterizerState* pState)
{
	return m_RasterizerStates.GetKey(*pState);
}

void		OpenGL3APIWrapper::SetRasterizerState(HAGE::u16 code)
{
	if(code == m_CurrentRS)
		return;
	m_CurrentRS=code;
	const HAGE::APIWRasterizerState& state = m_RasterizerStates.GetItem(code);

	if(state.bDepthClipEnable)
		glEnable (GL_DEPTH_TEST);
	else
		glDisable (GL_DEPTH_TEST);

	switch(state.CullMode)
	{
		case HAGE::CULL_NONE:	glDisable(GL_CULL_FACE);					break;
		case HAGE::CULL_CCW:	glEnable(GL_CULL_FACE);glFrontFace(GL_CW);	break;
		case HAGE::CULL_CW:		glEnable(GL_CULL_FACE);glFrontFace(GL_CCW);	break;
	}

	if(state.bWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

HAGE::u16	OpenGL3APIWrapper::GetBlendStateCode(const HAGE::APIWBlendState* pState,HAGE::u32 nBlendStates,bool bAlphaToCoverage)
{
	BlendStateEX state;
	state.bAlphaToCoverage = bAlphaToCoverage;
	state.nBlendStates = nBlendStates;
	HAGE::u32 i;
	for(i=0;i<nBlendStates;++i)
		state.BlendStates[i]=pState[i];
	for(;i<8;++i)
		state.BlendStates[i]=HAGE::DefaultBlendState;

	return m_BlendStates.GetKey(state);
}

inline GLenum HAGEBlendOpToOGLBlendOp(const HAGE::APIWBlendOp& op)
{
	switch(op)
	{
	case HAGE::BLEND_OP_ADD:			return GL_FUNC_ADD;
	case HAGE::BLEND_OP_SUBTRACT:		return GL_FUNC_SUBTRACT;
	case HAGE::BLEND_OP_REV_SUBTRACT:	return GL_FUNC_REVERSE_SUBTRACT;
	case HAGE::BLEND_OP_MIN:			return GL_MIN;
	case HAGE::BLEND_OP_MAX:			return GL_MAX;
	default: assert("Unsupported Blend Mode"); return 0;
	}
}

inline GLenum HAGEBlendModeToOGLBlendMode(const HAGE::APIWBlendMode& mode)
{
	switch(mode)
	{
	case HAGE::BLEND_ZERO:					return GL_ZERO;
	case HAGE::BLEND_ONE:					return GL_ONE;
	case HAGE::BLEND_SRC_COLOR:				return GL_SRC_COLOR;
	case HAGE::BLEND_INV_SRC_COLOR:			return GL_ONE_MINUS_SRC_COLOR;
	case HAGE::BLEND_SRC_ALPHA:				return GL_SRC_ALPHA;
	case HAGE::BLEND_INV_SRC_ALPHA:			return GL_ONE_MINUS_SRC_ALPHA;
	case HAGE::BLEND_DEST_ALPHA:			return GL_DST_ALPHA;
	case HAGE::BLEND_INV_DEST_ALPHA:		return GL_ONE_MINUS_DST_ALPHA;
	case HAGE::BLEND_DEST_COLOR:			return GL_DST_COLOR;
	case HAGE::BLEND_INV_DEST_COLOR:		return GL_ONE_MINUS_DST_COLOR;
	case HAGE::BLEND_SRC_ALPHA_SAT:			return GL_SRC_ALPHA_SATURATE;
	case HAGE::BLEND_BLEND_FACTOR:			return GL_CONSTANT_COLOR;
	case HAGE::BLEND_INV_BLEND_FACTOR:		return GL_ONE_MINUS_CONSTANT_COLOR;
		/*
	case HAGE::BLEND_SRC1_COLOR:			return GL_ZERO;
	case HAGE::BLEND_INV_SRC_1_COLOR:		return GL_ZERO;
	case HAGE::BLEND_SRC1_ALPHA:			return GL_ZERO;
	case HAGE::BLEND_INV_SRC1_ALPHA:		return GL_ZERO;
		*/
	default: assert("Multiple Rendertargets currently not supported\n"); return 0;
	}
}

void		OpenGL3APIWrapper::SetBlendState(HAGE::u16 code)
{
	if(code == m_CurrentBS)
		return;
	m_CurrentBS=code;
	const BlendStateEX& state = m_BlendStates.GetItem(code);

	if(state.bAlphaToCoverage)
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
	else
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);

	assert(state.nBlendStates == 1);

	for(HAGE::u32 i =0;i<state.nBlendStates;++i)
	{
		if(state.BlendStates[i].bBlendEnable)
		{
			glEnable(GL_BLEND);
			glBlendEquationSeparate(
					HAGEBlendOpToOGLBlendOp(state.BlendStates[i].BlendOp),
					HAGEBlendOpToOGLBlendOp(state.BlendStates[i].BlendOpAlpha)
				);
			glBlendFuncSeparate(/*i,*/
				HAGEBlendModeToOGLBlendMode(state.BlendStates[i].SrcBlend),
				HAGEBlendModeToOGLBlendMode(state.BlendStates[i].DestBlend),
				HAGEBlendModeToOGLBlendMode(state.BlendStates[i].SrcBlendAlpha),
				HAGEBlendModeToOGLBlendMode(state.BlendStates[i].DestBlendAlpha)
				);
		}
		else
			glDisable(GL_BLEND);
	}

}

#endif
