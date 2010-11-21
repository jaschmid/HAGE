/********************************************************/
/* FILE: HAGE.h                                         */
/* DESCRIPTION: HAGE Main Header                        */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#ifndef HAGE__MAIN__HEADER
#define HAGE__MAIN__HEADER

#include "preproc.h"
#include "types.h"
#include "results.h"
#include "messages.h"
#include "TLS.h"

#include "hage_math.h"

#include "interlocked_functions.h"

#include "IObject.h"
#include "IDomain.h"

#include "global_allocator.h"

#include "MainClass.h"

#include "DomainBase.h"
#include "ObjectBase.h"
#include "PinHelpers.h"
#include "RenderingAPIWrapper.h"

namespace HAGE
{
	class InputDomain;
	DECLARE_GUID(InputDomain,		0x8a601455,0xb21c,0x4870,0x902d,0x693abde180ba);
}

#endif
