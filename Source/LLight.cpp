#include "header.h"
#include "LLight.h"

namespace HAGE {

	LogicLight* LogicLight::CreateInstance(const guid& objectId,const LightInit& vpos)
	{
		return new LogicLight(objectId,vpos);
	}

	bool LogicLight::Step()
	{
		if(_init.bMove)
		{
			_data.Position.x = _init.Position.x -cosf(GetTime().toSeconds())*2.0f;
			_data.Position.z = _init.Position.z +sinf(GetTime().toSeconds())*2.0f;
		}
		Output::Set(_data);
		return false;
	}

	LogicLight::LogicLight(guid ObjectId,const LightInit& init) : GenericLight(),ObjectBase<LogicLight>(ObjectId),_init(init)
	{
		_data.Color = _init.Color;
		_data.Position = _init.Position;
		_data.Range = _init.Range;
		Output::Set(_data);
	}

	LogicLight::~LogicLight()
	{
	}

	DEFINE_CLASS_GUID(LogicLight);

}
