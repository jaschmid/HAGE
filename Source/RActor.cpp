#include "header.h"
#include "RActor.h"

namespace HAGE {

	IObject* RenderingActor::CreateInstance(guid objectId)
	{
		return (IObject*) new RenderingActor(objectId);
	}

	RenderingActor::RenderingActor(guid ObjectId) : 
		BaseType(ObjectId)
	{
	}

	RenderingActor::~RenderingActor()
	{
	}
}