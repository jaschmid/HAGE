#ifndef SHEET__LOGIC__INCLUDED
#define SHEET__LOGIC__INCLUDED

#include "header.h"
#include "GenericSheet.h"
#include "LogicDomain.h"

namespace HAGE {

#define CIRCULAR_MOTION

class LogicSheet : public GenericSheet, public ObjectBase<LogicSheet>
{
public:
	static LogicSheet* CreateInstance(const guid& ObjectId,const SheetInit& init);
	
	bool Step();

private:
	LogicSheet(guid ObjectId,const SheetInit& init);
	virtual ~LogicSheet();

	SheetInit		_init;
	LSheetOut		_data;
};

}
#endif