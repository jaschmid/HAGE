#include "header.h"
#include "GenericActor.h"
#include "RenderingDomain.h"
#include "UserInterfaceRendering.h"
#include "RActor.h"

namespace HAGE {

	bool RenderingDomain::MessageProc(const Message* m)
	{
		if(IsMessageType(m->GetMessageCode(),MESSAGE_UI_UNKNOWN))
		{
			switch(m->GetMessageCode())
			{
			case MESSAGE_UI_ADJUST_CAMERA:
				{
					MessageUIAdjustCamera* pMessage=(MessageUIAdjustCamera*)m;
					fCameraX+=pMessage->GetRotateX();
					fCameraY+=pMessage->GetRotateY();
					fCameraZ+=pMessage->GetZoom();
				}
				return true;
			}
			return pInterface->ProcessUserInterfaceMessage(m);
		}
		switch(m->GetMessageCode())
		{
			case MESSAGE_UI_UNKNOWN:
			default:
				return SharedDomainBase::MessageProc(m);
		}
	}

	void RenderingDomain::DrawIco(Vector3<> position)
	{
		Ico->Draw(position,
			Matrix4<>::Translate(Vector3<>(0.0,0.0,fCameraZ))*Matrix4<>::AngleRotation(Vector3<>(1.0,0.0,0.0),fCameraX)*Matrix4<>::AngleRotation(Vector3<>(0.0,1.0,0.0),fCameraY),
			Matrix4<>::Perspective(0.1f,200000.0f,1.3f,3.0f/4.0f*1.3f));
	}

	void RenderingDomain::DomainStep(u64 step)
	{
		pWrapper->BeginFrame();
		
		auto result = Factory.ForEach<int,RenderingActor>( [this](RenderingActor* o) -> int {return o->Step(this);} , guidNull ,true);	

		pWrapper->PresentFrame();
	}

	RenderingDomain::RenderingDomain() : 
		fCameraX(0.0),fCameraY(0.0),fCameraZ(50.0)
	{
		Factory.RegisterObjectType<RenderingActor>();

		pWrapper = RenderingAPIWrapper::CreateD3D11Wrapper();
		//pWrapper = RenderingAPIWrapper::CreateOpenGL3Wrapper();

		pWrapper->BeginAllocation();
		
		// Create the vertex buffer

		pInterface = new UserInterfaceRendering(pWrapper);

		Ico = Resource->OpenResource<IDrawableMesh>("bunny.ply");

		pWrapper->EndAllocation();

		printf("Init Rendering\n");
	}

	RenderingDomain::~RenderingDomain()
	{
		printf("Destroy Rendering\n");

		if(pInterface)delete pInterface;
		if(pWrapper)delete pWrapper;
	}
}
