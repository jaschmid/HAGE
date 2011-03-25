#include "header.h"

namespace HAGE {

DECLARE_DOMAIN(GenerationDomain,		0xeacb7ad7,0x7380,0x4ffa,0xae97,0x41b616721e97);

template<> class get_traits<GenerationDomain> : public		DomainTraits<GenerationDomain,true,InputDomain> {};

}