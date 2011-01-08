#include "header.h"
#include "RLight.h"

namespace HAGE {
	RenderingLight* RenderingLight::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source)
	{
		return new RenderingLight(ObjectId,h,source);
	}

	RenderingLight::RenderingLight(const guid& ObjectId,const MemHandle& h,const guid& source) :
		ObjectBase<RenderingLight>(ObjectId,h,source)
	{
		
	}

	GLightOut RenderingLight::Step(RenderingDomain* pRendering)
	{
		_data = Input1::Get();
		return _data;
	}

	RenderingLight::~RenderingLight()
	{
	}

	DEFINE_CLASS_GUID(RenderingLight);
}
