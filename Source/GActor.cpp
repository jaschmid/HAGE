#include "header.h"
#include "GActor.h"

namespace HAGE {

	GraphicsActor* GraphicsActor::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source,const ActorGInit* pInit)
	{
		return new GraphicsActor(ObjectId,h,source,pInit);
	}

	int GraphicsActor::Step()
	{
		data = Input1::Get();
		Output::Set(data);
		return 1;
	}

	GraphicsActor::GraphicsActor(const guid& ObjectId,const MemHandle& h,const guid& source,const ActorGInit* pInit) :
		ObjectBase<GraphicsActor>(ObjectId,h,source)
	{
		data = Input1::Get();
		Output::Set(data);
		
		ActorRInit& rinit = Output::InitOut;
		rinit = *pInit;
	}

	GraphicsActor::~GraphicsActor()
	{
	}

	DEFINE_CLASS_GUID(GraphicsActor);

}
