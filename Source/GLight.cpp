#include "header.h"
#include "GLight.h"

namespace HAGE {

	GraphicsLight* GraphicsLight::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source)
	{
		return new GraphicsLight(ObjectId,h,source);
	}

	int GraphicsLight::Step()
	{
		_data.Color = Input1::Get().Color;
		_data.Position = Input1::Get().Position;
		_data.Range = Input1::Get().Range;
		_data.bVisible = true;
		Output::Set(_data);
		return 1;
	}

	GraphicsLight::GraphicsLight(const guid& ObjectId,const MemHandle& h,const guid& source) :
		ObjectBase<GraphicsLight>(ObjectId,h,source)
	{
		_data.Color = Input1::Get().Color;
		_data.Position = Input1::Get().Position;
		_data.Range = Input1::Get().Range;
		_data.bVisible = true;
		Output::Set(_data);
	}

	GraphicsLight::~GraphicsLight()
	{
	}

	DEFINE_CLASS_GUID(GraphicsLight);

}
