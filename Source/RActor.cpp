#include "header.h"
#include "RActor.h"

namespace HAGE {

	IObject* RenderingActor::CreateInstance(guid objectId)
	{
		return (IObject*) new RenderingActor(objectId);
	}

	RenderingActor::RenderingActor(guid ObjectId) :
		ObjectBase<RenderingActor>(ObjectId)
	{
	}

	RenderingActor::~RenderingActor()
	{
	}

	DEFINE_CLASS_GUID(RenderingActor);
}
