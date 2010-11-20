#include "header.h"
#include "GenericActor.h"
#include "AIDomain.h"
#include "LogicDomain.h"

namespace HAGE {

	AIDomain::AIDomain() : TestIn(Input.GetBasePin()),TestOut(Output.GetBasePin())
	{
		printf("Init AI\n");
	}

	bool AIDomain::MessageProc(const Message* m)
	{
		switch(m->GetMessageCode())
		{
			case MESSAGE_ITEM_CREATED:
				{
					const SimpleMessage<MemHandle>* item = (const SimpleMessage<MemHandle>*)m;
					TestIn.Open(item->GetData());
				}
				return true;
			default:
				SharedDomainBase::MessageProc(m);
				return true;
		}
	}

	void AIDomain::DomainInit(u64 step)
	{
		Output.PostMessage(SimpleMessage<MemHandle>(MESSAGE_ITEM_CREATED,TestOut.GetHandle()));

		*TestOut = *TestIn;
	}

	void AIDomain::DomainStep(u64 step)
	{

		//printf(",",step);


		static float f=1.534523f;
		for(int i=0;i<rand()%0xffff;++i)f=f*f;
		*TestOut = *TestIn;

		//printf(".",step);

	}

	AIDomain::~AIDomain()
	{
		printf("Destroy AI\n");
	}

	const guid& AIDomain::id(guidAIDomain);
	const bool AIDomain::continuous = false;
}
