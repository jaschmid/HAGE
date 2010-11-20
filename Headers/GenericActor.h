#ifndef ACTOR__GENERIC__INCLUDED
#define ACTOR__GENERIC__INCLUDED

#include "header.h"

namespace HAGE {


class GenericActor : public IObject
{
public:
	GenericActor(guid ObjectId) : objectId(ObjectId) {}
	virtual ~GenericActor() {}

	static const guid& getClassGuid(){static const guid guid={{{0xeeeeeeeeeeeeeeee,0x0000000000000002}}}; return guid;}

	virtual bool MessageProc(const MessageObjectUnknown* pMessage)
	{
		return false;
	}

	virtual result Step(u64 step) = 0;

	virtual result QueryInterface(guid id,void** ppInterface)
	{
		return E_FAIL;
	}

	const guid& GetId(){return objectId;}

protected:

	virtual result Destroy()
	{
		delete this;
		return S_OK;
	}

private:

	const guid	objectId;
};

template<class _T> class Actor : public GenericActor, public ObjectBase<_T>
{
public:
	static const bool isImplemented = false;

	virtual result Step(u64 step) {return E_FAIL;};

	static IObject* CreateInstance(guid ObjectId){return nullptr;}
private:
};

class LogicDomain;
class GraphicsDomain;
class RenderingDomain;

template<> class Actor<LogicDomain> : public GenericActor, public ObjectBase<LogicDomain>
{
public:
	static IObject* CreateInstance(guid ObjectId);

	virtual result Step(u64 step);

	static const bool isImplemented = true;
private:
	Actor<LogicDomain>(guid ObjectId);
	virtual ~Actor<LogicDomain>();

	Vector3<>		position;

	OutputVar<Vector3<>>	TestOut;
};

template<> class Actor<GraphicsDomain>  : public GenericActor, public ObjectBase<GraphicsDomain>
{
public:
	static IObject* CreateInstance(guid ObjectId);
	virtual bool MessageProc(const MessageObjectUnknown* pMessage);

	virtual result Step(u64 step);

	static const bool isImplemented = true;
private:
	Actor<GraphicsDomain>(guid ObjectId);
	virtual ~Actor<GraphicsDomain>();

	Vector3<>		position;
	Vector3<u8>		color;

	InputVar<Vector3<>>		PositionIn;
	OutputVar<Vector3<u8>>	ColorOut;
};

template<> class Actor<RenderingDomain>  : public GenericActor, public ObjectBase<RenderingDomain>
{
public:
	static IObject* CreateInstance(guid ObjectId);
	virtual bool MessageProc(const MessageObjectUnknown* pMessage);

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
