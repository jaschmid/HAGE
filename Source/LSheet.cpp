#include "header.h"
#include "LSheet.h"

#include "SettingsLoader.h"

#include <stdio.h>
#include <time.h>



//#define STATIC_TIME_STEP 0.001
#define MAX_DT 0.004

namespace HAGE {

	LogicSheet* LogicSheet::CreateInstance(const guid& objectId,const SheetInit& vpos)
	{
		return new LogicSheet(objectId,vpos);
	}

	bool LogicSheet::Step()
	{
#ifndef STATIC_TIME_STEP
		static clock_t tt = clock();
		f32 dt = clock()-tt;
		tt += dt;
		if(dt == 0) dt = 0.01;
		dt = dt*CLOCKS_PER_SEC / 1000.0;
		dt /= 1000.0;
#else
		f32 dt = STATIC_TIME_STEP;
#endif
		
		if(dt>MAX_DT)dt = MAX_DT;
		calculateForces();
		calculateVelocities(dt);
		applyVelocities(dt);
		
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
			}
		
		nSpringDampers = 2*(2*SheetSize*SheetSize - 3*SheetSize + 1);
		forces = new Vector3<>[SheetSize*SheetSize];
		velocities = new Vector3<>[SheetSize*SheetSize];
		springDampers = new SpringDamper*[nSpringDampers];
		pinned = new bool[SheetSize*SheetSize];
		mass = new f32[SheetSize*SheetSize];
		for(u32 i=0;i<SheetSize*SheetSize;i++)
		{
			velocities[i] = Vector3<>(0,0,0);
			pinned[i] = 0;
			mass[i] = settings->getf32Setting("cloth_particle_mass");
		}
		initVertexGroups();
		u32 i0,i1,current = 0;
		f32 sc = settings->getf32Setting("cloth_spring_constant");
		f32 dc = settings->getf32Setting("cloth_damping_constant");
		f32 xl = sqrt(!xStep);
		f32 yl = sqrt(!yStep);
		f32 dl = sqrt(xl*xl+yl*yl);
		for(int x=0;x<SheetSize;x++)for(int y=0;y<SheetSize;y++)
		{
			if(x<SheetSize-1){ // Horizontal
				i0 = y*SheetSize+x;
				springDampers[current++] = new SpringDamper(i0,i0+1,sc,dc,xl,*this);
			}
			if(y<SheetSize-1){ // vertical
				i0 = y*SheetSize+x;
				springDampers[current++] = new SpringDamper(i0,i0+SheetSize,sc,dc,yl,*this);
			}
			if(y<SheetSize-1&&x<SheetSize-1){ //Diagonal
				i0 = y*SheetSize+x;
				i1 = (y+1)*SheetSize+x+1;
				springDampers[current++] = new SpringDamper(i0,i1,sc,dc,dl,*this);

				i0 = y*SheetSize+x+1;
				i1 = (y+1)*SheetSize+x;
				springDampers[current++] = new SpringDamper(i0,i1,sc,dc,dl,*this);
			}
				
		}
		Output::Set(_data);
	}

	LogicSheet::~LogicSheet()
	{
		if(forces)delete forces;
		if(velocities)delete velocities;
		if(springDampers)delete springDampers;
		if(mass)delete mass;
		if(pinned)delete pinned;
	}

	DEFINE_CLASS_GUID(LogicSheet);


	void LogicSheet::calculateForces(){
		for(u32 i=0;i<SheetSize*SheetSize;i++)
			forces[i] = Vector3<>(0.0f,mass[i]/-9.89,0.0f); //gravity
		
		for(u32 i=0;i<nSpringDampers;i++)
			springDampers[i]->calculateForce();
	}

	void LogicSheet::SpringDamper::calculateForce(){
		Vector3<> *pos0 = &(owner._data.positions[i0]);
		Vector3<> *pos1 = &(owner._data.positions[i1]);
		Vector3<> *f0 = &(owner.forces[i0]);
		Vector3<> *f1 = &(owner.forces[i1]);
		Vector3<> *v0 = &(owner.velocities[i0]);
		Vector3<> *v1 = &(owner.velocities[i1]);

		Vector3<> dir = (*pos1)-(*pos0);
		f32 l = sqrt(!dir);
		dir.x /= l;dir.y /= l;dir.z /= l;

		f32 vp0 = dir*(*v0);
		f32 vp1 = dir*(*v1);

		f32 sf = -sc*(1-(l/rl)); //Spring force
		f32 df = -dc*(vp0-vp1);  //damping force
		f32 sdf = sf+df;         //Spring force + damping force = resulting force
		*f0 += dir*sdf;          //Add force to first particle
		*f1 -= dir*sdf;          //Add negative force to second particle
	}

	void LogicSheet::calculateVelocities(f32 dt){
		for(unsigned int i = 0;i<SheetSize*SheetSize;i++){
			if(pinned[i])
				velocities[i] = Vector3<>(0,0,0);
			else
				velocities[i] += forces[i]*(dt/mass[i]);
		}
	}

	bool collision(Vector3<> pos){
		Vector3<> c(settings->getf32Setting("cloth_fake_sphere_x"),settings->getf32Setting("cloth_fake_sphere_y"),settings->getf32Setting("cloth_fake_sphere_z"));
		return !(pos-c)<settings->getf32Setting("cloth_fake_sphere_r")*settings->getf32Setting("cloth_fake_sphere_r");
	}

	void LogicSheet::applyVelocities(f32 dt){
		for(unsigned int i = 0;i<SheetSize*SheetSize;i++)if(!pinned[i]){
			Vector3<> oldPos = _data.positions[i];
			_data.positions[i] += velocities[i] * dt;
			if(collision(_data.positions[i])){
				velocities[i] = Vector3<>(0,0,0);
				_data.positions[i] = oldPos;
			}

		}
	}

	void LogicSheet::setPinned(std::string vertexGroup,bool pinn){
		for(int i = 0; i<vertexGroups[vertexGroup].size();i++){
			pinned[vertexGroups[vertexGroup][i]] = pinn;
		}
	}

	void LogicSheet::initVertexGroups(){
		for(int x=0;x<SheetSize;x++)for(int y=0;y<SheetSize;y++)
		{
			u32 index = y*SheetSize+x;
			if(x==0){
				vertexGroups["edge1"].push_back(index);
			}if(y==0){
				vertexGroups["edge2"].push_back(index);
			}if(x==SheetSize-1){
				vertexGroups["edge3"].push_back(index);
			}if(y==SheetSize-1){
				vertexGroups["edge4"].push_back(index);
			}
			if(x==0||y==0||x==SheetSize-1||y==SheetSize-1)
				vertexGroups["edges"].push_back(index);
			if(x==0 && y==0){
				vertexGroups["corner1"].push_back(index);
				vertexGroups["corners"].push_back(index);
			}if(x==0 && y==SheetSize-1){
				vertexGroups["corner2"].push_back(index);
				vertexGroups["corners"].push_back(index);
			}if(x==SheetSize-1 && y==0){
				vertexGroups["corner3"].push_back(index);
				vertexGroups["corners"].push_back(index);
			}if(x==SheetSize-1 && y==SheetSize-1){
				vertexGroups["corner4"].push_back(index);
				vertexGroups["corners"].push_back(index);
			}
		}

		for(int i = 0;i<settings->getu32Setting("cloth_number_of_pinned_groups");i++){
			std::stringstream ss;
			ss << "cloth_pinned_group_" << i;
			setPinned(settings->getstringSetting(ss.str()));
		}
	}
}