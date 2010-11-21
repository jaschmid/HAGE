#include "HAGE.h"
#include "RenderingAPIWrapper.h"

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

}