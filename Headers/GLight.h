#ifndef LIGHT__GRAPHICS__INCLUDED
#define LIGHT__GRAPHICS__INCLUDED

#include "header.h"
#include "GenericLight.h"
#include "GraphicsDomain.h"

namespace HAGE {

class GraphicsLight  : public GenericLight, public ObjectBase<GraphicsLight>
{
public:
	static GraphicsLight* CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source);
	
	int Step();

private:
	GraphicsLight(const guid& ObjectId,const MemHandle& h,const guid& source);
	virtual ~GraphicsLight();
	void GenerateNormals();

	GLightOut	_data;
};


}
#endif