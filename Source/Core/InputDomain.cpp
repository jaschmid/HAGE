#include "HAGE.h"
#include "DebugUI.h"
#include "InputDomain.h"

#include <boost/date_time.hpp>


namespace HAGE {

		struct test_struct
		{
			int a,b,c;
		};
		
		InputDomain::InputDomain()  :m_pDebugInterface(new DebugUI)
		{
			while(const Message* m=OSInputQueue.GetTopMessage())
			{
				if(failed(m_pDebugInterface->ProcessInputMessage(m)))
					Output.PostMessage(*m);

				OSInputQueue.PopMessage();
			}
			printf("Init Input\n");
		}


		void InputDomain::DomainStep(u64 step)
		{
			// for now just forward input
			while(const Message* m=OSInputQueue.GetTopMessage())
			{
				if(!(m_pDebugInterface->ProcessInputMessage(m)))
					Output.PostMessage(*m);

				OSInputQueue.PopMessage();
			}
			/*
			auto result = Factory.ForEach<test_struct,IObject>( [](IObject* o) -> test_struct {test_struct t; t.a=1; return t;} , guidNull );*/
		}

		InputDomain::~InputDomain()
		{
			delete m_pDebugInterface;
		}

		const guid& InputDomain::id = guidInputDomain;
		const bool InputDomain::continuous = false;
}
