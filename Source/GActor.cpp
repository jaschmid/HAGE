#include "header.h"
#include "GActor.h"

namespace HAGE {

	IObject* GraphicsActor::CreateInstance(guid objectId)
	{
		return (IObject*) new GraphicsActor(objectId);
	}

	GraphicsActor::GraphicsActor(guid ObjectId) :
		ObjectBase<GraphicsActor>(ObjectId)
	{
	}

	GraphicsActor::~GraphicsActor()
	{
	}

	DEFINE_CLASS_GUID(GraphicsActor);

}
