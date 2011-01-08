#ifndef LIGHT__RENDERING__INCLUDED
#define LIGHT__RENDERING__INCLUDED

#include "header.h"
#include "GenericLight.h"
#include "RenderingDomain.h"

namespace HAGE {

class RenderingLight  : public GenericLight, public ObjectBase<RenderingLight>
{
public:
	static RenderingLight* CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source);
	
	GLightOut Step(RenderingDomain* pRendering);

private:
	RenderingLight(const guid& ObjectId,const MemHandle& h,const guid& source);

	GLightOut _data;
	virtual ~RenderingLight();
};

}
#endif