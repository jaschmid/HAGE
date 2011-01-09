#ifndef SHEET__LOGIC__INCLUDED
#define SHEET__LOGIC__INCLUDED

#include "header.h"
#include "GenericSheet.h"
#include "LogicDomain.h"

namespace HAGE {


class LogicSheet : public GenericSheet, public ObjectBase<LogicSheet>
{
	struct SpringDamper{
		u32 i0; //index 0 (for position, velocities and forces)
		u32 i1; //index 1
		f32 sc; //spring constant
		f32 dc; //damper constant
		f32 rl; //rest Length
		LogicSheet& owner;
		SpringDamper(u32 i0,u32 i1,f32 sc,f32 dc,f32 rl,LogicSheet& owner):
			i0(i0),
			i1(i1),
			sc(sc),
			dc(dc),
			rl(rl),
			owner(owner)
		{}

		void calculateForce();
	};
public:
	static LogicSheet* CreateInstance(const guid& ObjectId,const SheetInit& init);
	
	bool Step();

private:
	LogicSheet(guid ObjectId,const SheetInit& init);
	virtual ~LogicSheet();

	SheetInit		_init;
	LSheetOut		_data;

	Vector3<> *forces;
	Vector3<> *velocities;
	SpringDamper **springDampers;
	u32 nSpringDampers;
	f32 *mass;
	std::map<std::string, std::vector<int> > vertexGroups;
	bool *pinned;
	void calculateForces();
	void calculateVelocities(f32 dt);
	void applyVelocities(f32 dt);
	void setPinned(std::string vertexGroup,bool pinn = true);
	void initVertexGroups();
};

}
#endif