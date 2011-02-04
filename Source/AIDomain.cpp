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

	void AIDomain::DomainStep(t64 time)
	{
	}

	AIDomain::~AIDomain()
	{
		printf("Destroy AI\n");
	}
}
