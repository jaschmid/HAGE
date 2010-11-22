#ifndef ACTOR__GENERIC__INCLUDED
#define ACTOR__GENERIC__INCLUDED

#include "header.h"
#include <boost/random/mersenne_twister.hpp>

namespace HAGE {


class GenericActor : public IObject
{
public:
	GenericActor(guid ObjectId) : objectId(ObjectId) {}
	virtual ~GenericActor() {}

	static const guid& getClassGuid(){static const guid guid={{{0xeeeeeeeeeeeeeeee,0x0000000000000002}}}; return guid;}
	static const std::array<guid,2>& getCapabilities(){static const std::array<guid,2> val= {guidNull,getClassGuid()}; return val;}

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

	float getFRand()
	{
		static boost::mt19937 rgen;
		return ((float)rgen()/(float)rgen.max())*2.0f-1.0f;
	}

	Vector3<> Init()
	{
		position=Vector3<>(getFRand()*20.0f,getFRand()*20.0f,getFRand()*20.0f);
		speed=Vector3<>(getFRand()*.2f,getFRand()*.2f,getFRand()*.2f);
		acceleration=Vector3<>(0.0f,0.0f,0.0f);
		return position;
	}
	Vector3<> Step(const std::vector<Vector3<>>& positions)
	{
		acceleration = Vector3<>(0.0f,0.0f,0.0f);
		for(auto i=positions.begin();i!=positions.end();++i)
		{
			Vector3<> vd = *i-position;
			float		fd2 = !(vd);
			if(fd2 >= 0.3)
			{
				float		fd = sqrtf(fd2);
				vd = vd/fd;
				acceleration = acceleration + vd/fd2;	
			}
		}
		
		speed = speed + acceleration * 0.05f;
		position = position + speed *0.05f;
		if(position.x<=-100.0)
			speed.x = -speed.x;
		if(position.y<=-100.0)
			speed.y = -speed.y;
		if(position.z<=-100.0)
			speed.z = speed.z;
		if(position.x>=100.0)
			speed.x = -speed.x;
		if(position.y>=100.0)
			speed.y = -speed.y;
		if(position.z>=100.0)
			speed.z = -speed.z;
		*TestOut=position;
		return position;
	}

	static const bool isImplemented = true;
private:
	Actor<LogicDomain>(guid ObjectId);
	virtual ~Actor<LogicDomain>();

	Vector3<>		position;
	Vector3<>		speed;
	Vector3<>		acceleration;

	OutputVar<Vector3<>>	TestOut;
};

template<> class Actor<GraphicsDomain>  : public GenericActor, public ObjectBase<GraphicsDomain>
{
public:
	static IObject* CreateInstance(guid ObjectId);
	virtual bool MessageProc(const MessageObjectUnknown* pMessage);

	virtual result Step(u64 step);

	int Step()
	{
		position = *PositionIn;
		color = Vector3<u8>(255,255,255);
		*ColorOut=color;
		return 1;
	}

	static const bool isImplemented = true;
private:
	Actor<GraphicsDomain>(guid ObjectId);
	virtual ~Actor<GraphicsDomain>();

	Vector3<>		position;
	Vector3<u8>		color;


	InputVar<Vector3<>>		PositionIn;
	OutputVar<Vector3<u8>>	ColorOut;
};

}

#endif
