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
	}

	int RenderingActor::Step(RenderingDomain* pRendering)
	{
		position = Input1::Get();
		if(Input2::IsReady())
			color = Input2::Get();
		pRendering->DrawIco(position);
		return 1;
	}

	RenderingActor::~RenderingActor()
	{
	}

	DEFINE_CLASS_GUID(RenderingActor);
}
