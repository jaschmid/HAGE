#include "header.h"
#include "RActor.h"

namespace HAGE {

	RenderingActor* RenderingActor::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source)
	{
		return new RenderingActor(ObjectId,h,source);
	}

	RenderingActor::RenderingActor(const guid& ObjectId,const MemHandle& h,const guid& source) :
		ObjectBase<RenderingActor>(ObjectId,h,source)
	{
		char temp[256];
		sprintf(temp,"mesh%i.ply",rand()%3);
		_mesh = GetResource()->OpenResource<IDrawableMesh>(temp);
		if(Input1::IsReady() )
		{
			position = Input1::Get();
		}
	}

	int RenderingActor::Step(RenderingDomain* pRendering)
	{
		if(Input1::IsReady() )
		{
			position = Input1::Get();
			_mesh->Draw(position,pRendering->GetViewMatrix(),pRendering->GetProjectionMatrix());
		}
		return 1;
	}

	RenderingActor::~RenderingActor()
	{
	}

	DEFINE_CLASS_GUID(RenderingActor);
}
