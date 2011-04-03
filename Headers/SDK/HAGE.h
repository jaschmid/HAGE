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
#include "traits.h"

#include "global_allocator.h"

#include "MainClass.h"

#include "DomainBase.h"
#include "ObjectBase.h"
#include "PinHelpers.h"
#include "RenderingAPIWrapper.h"
#include "EffectContainer.h"

#include "ResourceFormats.h"

#include "SparseVirtualTextureFile.h"

namespace HAGE
{
	template<> class get_traits<InputDomain> : public DomainTraits<InputDomain,true> {};
	template<> class get_traits<ResourceDomain> : public DomainTraits<ResourceDomain,false> {};
}

#endif
