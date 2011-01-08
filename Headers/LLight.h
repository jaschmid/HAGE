#ifndef LIGHT__LOGIC__INCLUDED
#define LIGHT__LOGIC__INCLUDED

#include "header.h"
#include "GenericLight.h"
#include "LogicDomain.h"

namespace HAGE {

class LogicLight : public GenericLight, public ObjectBase<LogicLight>
{
public:
	static LogicLight* CreateInstance(const guid& ObjectId,const LightInit& init);
	
	bool Step();

private:
	LogicLight(guid ObjectId,const LightInit& init);
	virtual ~LogicLight();

	LightInit		_init;
	LLightOut		_data;
};

}
#endif