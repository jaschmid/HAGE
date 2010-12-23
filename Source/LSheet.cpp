#include "header.h"
#include "LSheet.h"

namespace HAGE {

	LogicSheet* LogicSheet::CreateInstance(const guid& objectId,const SheetInit& vpos)
	{
		return new LogicSheet(objectId,vpos);
	}

	bool LogicSheet::Step()
	{
		static float t = 0.0f;
		t+=0.1f;
		Vector3<> xStep = ((_init.HalfExtent%_init.Normal)- (_init.HalfExtent))/(float)(SheetSize);
		Vector3<> yStep = ((_init.Normal%_init.HalfExtent)- (_init.HalfExtent))/(float)(SheetSize);
		for(int ix=0;ix<SheetSize;ix++)
			for(int iy=0;iy<SheetSize;iy++)
			{
				_data.positions[iy*SheetSize+ix] = _init.Center + _init.HalfExtent + xStep*ix + yStep*iy;
				// give sinus wave
				_data.positions[iy*SheetSize+ix] += _init.Normal*sinf(ix/10.0f+iy/10.0f+t);
			}
		Output::Set(_data);
		return false;
	}

	LogicSheet::LogicSheet(guid ObjectId,const SheetInit& init) : GenericSheet(),ObjectBase<LogicSheet>(ObjectId),_init(init)
	{
		Vector3<> xStep = ((init.HalfExtent%_init.Normal)- (init.HalfExtent))/(float)(SheetSize);
		Vector3<> yStep = ((_init.Normal%init.HalfExtent)- (init.HalfExtent))/(float)(SheetSize);
		for(int ix=0;ix<SheetSize;ix++)
			for(int iy=0;iy<SheetSize;iy++)
			{
				_data.positions[iy*SheetSize+ix] = _init.Center + init.HalfExtent + xStep*ix + yStep*iy;
				// give sinus wave
				_data.positions[iy*SheetSize+ix] += _init.Normal*sinf(ix/10.0f*iy/10.0f);
			}
		Output::Set(_data);
	}

	LogicSheet::~LogicSheet()
	{
	}

	DEFINE_CLASS_GUID(LogicSheet);

}
