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
		if(Input1::IsReady() && Input2::IsReady())
		{
			position = Input1::Get();
			color = Input2::Get();
		}
	}

	int RenderingActor::Step(RenderingDomain* pRendering)
	{
		if(Input1::IsReady() && Input2::IsReady())
		{
			position = Input1::Get();
			color = Input2::Get();
			pRendering->DrawIco(position);
		}
		return 1;
	}

	RenderingActor::~RenderingActor()
	{
	}

	DEFINE_CLASS_GUID(RenderingActor);
}
