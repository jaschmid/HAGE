#ifndef SHEET__GENERIC__INCLUDED
#define SHEET__GENERIC__INCLUDED

#include "header.h"
#include <boost/random/mersenne_twister.hpp>

namespace HAGE {

const int SheetSize = 30;

class GenericSheet 
{
public:
	GenericSheet() {}
	~GenericSheet() {}
};

class LogicSheet;
class GraphicsSheet;
class RenderingSheet;

struct SheetInit
{
	Vector3<>	Center;
	Vector3<>	HalfExtent;
	Vector3<>	Normal;
};

struct LSheetOut
{
	std::array<Vector3<>,SheetSize*SheetSize>	positions;
	std::array<Vector3<>,SheetSize*SheetSize>	normals;
};

struct GSheetOut
{
	std::array<Vector3<>,SheetSize*SheetSize>	positions;
	std::array<Vector3<>,SheetSize*SheetSize>	normals;
};

DECLARE_CLASS_GUID(RenderingSheet,	0x12d47f0e,0x8813,0x4ad5,0xb13d,0x916b7ab4a618);
DECLARE_CLASS_GUID(LogicSheet,		0xcf361bc0,0xf572,0x4cc9,0xbc2f,0x547537da68a8);
DECLARE_CLASS_GUID(GraphicsSheet,	0xbde45fb2,0x50c5,0x4efb,0x907b,0xd1568676a554);

template<> class get_traits<LogicSheet> : public ObjectTraits<LogicSheet,LogicDomain,SheetInit,LSheetOut> {};
template<> class get_traits<GraphicsSheet> : public ObjectTraits<GraphicsSheet,GraphicsDomain,NoDirectInstantiation<>,GSheetOut,LogicSheet> {};
template<> class get_traits<RenderingSheet> : public ObjectTraits<RenderingSheet,RenderingDomain,NoDirectInstantiation<>,void,GraphicsSheet> {};

}

#endif
