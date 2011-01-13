#include "header.h"
#include "GActor.h"

namespace HAGE {

	GraphicsActor* GraphicsActor::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source,const ActorGInit* pInit)
	{
		return new GraphicsActor(ObjectId,h,source,pInit);
	}

	int GraphicsActor::Step()
	{
		position = Input1::Get();
		Output::Set(position);
		return 1;
	}

	GraphicsActor::GraphicsActor(const guid& ObjectId,const MemHandle& h,const guid& source,const ActorGInit* pInit) :
		ObjectBase<GraphicsActor>(ObjectId,h,source)
	{
		position = Input1::Get();
		Output::Set(position);
		
		ActorRInit& rinit = Output::InitOut;
		strcpy(rinit.mesh,pInit->mesh);
		rinit.scale = pInit->scale;
	}

	GraphicsActor::~GraphicsActor()
	{
	}

	DEFINE_CLASS_GUID(GraphicsActor);

}
