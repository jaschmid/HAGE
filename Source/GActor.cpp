#include "header.h"
#include "GActor.h"

namespace HAGE {

	GraphicsActor* GraphicsActor::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source)
	{
		return new GraphicsActor(ObjectId,h,source);
	}

	int GraphicsActor::Step()
	{
		position = Input1::Get();
		color = Vector3<u8>(255,255,255);
		Output::Set(color);
		return 1;
	}

	GraphicsActor::GraphicsActor(const guid& ObjectId,const MemHandle& h,const guid& source) :
		ObjectBase<GraphicsActor>(ObjectId,h,source)
	{
		position = Input1::Get();
		color = Vector3<u8>(255,255,255);
		Output::Set(color);
	}

	GraphicsActor::~GraphicsActor()
	{
	}

	DEFINE_CLASS_GUID(GraphicsActor);

}
