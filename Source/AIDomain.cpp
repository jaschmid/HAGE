#include "header.h"
#include "GenericActor.h"
#include "AIDomain.h"
#include "LogicDomain.h"

namespace HAGE {

	AIDomain::AIDomain()
	{
		printf("Init AI\n");
	}

	bool AIDomain::MessageProc(const Message* m)
	{
		switch(m->GetMessageCode())
		{
			case MESSAGE_UI_UNKNOWN:
			default:
				SharedDomainBase::MessageProc(m);
				return true;
		}
	}

	void AIDomain::DomainStep(u64 step)
	{
	}

	AIDomain::~AIDomain()
	{
		printf("Destroy AI\n");
	}

	const guid& AIDomain::id(guidAIDomain);
	const bool AIDomain::continuous = false;
}
