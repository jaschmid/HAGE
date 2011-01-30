#include "HAGE.h"
#include "RenderingAPIWrapper.h"
#include "D3D11APIWrapper.h"
#include "OpenGL3APIWrapper.h"

namespace HAGE {

extern const APIWBlendState DefaultBlendState = {
	false,
	BLEND_ONE,
	BLEND_ZERO,
	BLEND_OP_ADD,
	BLEND_ONE,
	BLEND_ZERO,
	BLEND_OP_ADD,
	true, true, true, true
};
extern const APIWRasterizerState DefaultRasterizerState = {
	CULL_CCW,
	false,
	0,
	0.0f,
	0.0f,
	true,
	false,
	false
};


RenderingAPIAllocator* RenderingAPIAllocator::_pAllocator = nullptr;

RenderingAPIWrapper* RenderingAPIWrapper::CreateRenderingWrapper(APIWRendererType type,const APIWDisplaySettings* pSettings)
{
	switch(type)
	{
#if defined(TARGET_WINDOWS) && !defined(NO_D3D)
	default:
#endif
#ifndef NO_D3D
	case APIW_D3DWRAPPER:
		return D3D11APIWrapper::CreateD3D11Wrapper(pSettings);
		break;
#endif
#if !defined(TARGET_WINDOWS) || defined(NO_D3D)
	default:
#endif
#ifndef NO_OGL
	case APIW_OGLWRAPPER:
		return OpenGL3APIWrapper::CreateOGL3Wrapper(pSettings);
		break;
#endif
	}
}

}