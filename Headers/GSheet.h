#ifndef SHEET__GRAPHICS__INCLUDED
#define SHEET__GRAPHICS__INCLUDED

#include "header.h"
#include "GenericSheet.h"
#include "GraphicsDomain.h"

namespace HAGE {

class GraphicsSheet  : public GenericSheet, public ObjectBase<GraphicsSheet>
{
public:
	static GraphicsSheet* CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source);
	
	int Step();

private:
	GraphicsSheet(const guid& ObjectId,const MemHandle& h,const guid& source);
	virtual ~GraphicsSheet();
	void GenerateNormals();

	GSheetOut	_data;
};


}
#endif