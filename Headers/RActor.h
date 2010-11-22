#ifndef ACTOR__RENDERING__INCLUDED
#define ACTOR__RENDERING__INCLUDED

#include "header.h"
#include "GenericActor.h"
#include "RenderingDomain.h"

namespace HAGE {
	

template<> class Actor<RenderingDomain>  : public GenericActor, public ObjectBase<RenderingDomain>
{
public:
	static IObject* CreateInstance(guid ObjectId);
	virtual bool MessageProc(const MessageObjectUnknown* pMessage);
	
	int Step(RenderingDomain* pRendering)
	{
		position = *PositionIn;
		color = *ColorIn;
		pRendering->DrawIco(position);
		return 1;
	}

	virtual result Step(u64 step);

	static const bool isImplemented = true;
private:
	Actor<RenderingDomain>(guid ObjectId);
	virtual ~Actor<RenderingDomain>();


	Vector3<>		position;
	Vector3<u8>		color;

	InputVar<Vector3<>>		PositionIn;
	InputVar<Vector3<u8>>	ColorIn;
};

}
#endif