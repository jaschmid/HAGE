#include "header.h"
#include "GSheet.h"

namespace HAGE {

	GraphicsSheet* GraphicsSheet::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source)
	{
		return new GraphicsSheet(ObjectId,h,source);
	}

	void GraphicsSheet::GenerateNormals()
	{
		memcpy(_data.normals.data(),Input1::Get().normals.data(),SheetSize*SheetSize*sizeof(Vector3<>));
		memcpy(_data.positions.data(),Input1::Get().positions.data(),SheetSize*SheetSize*sizeof(Vector3<>));
		/*for(int ix= 0;ix<SheetSize;ix++)
			for(int iy= 0;iy<SheetSize;iy++)
			{
				// 0 = left, 1= right, 2= top, 3= bottom
				Vector3<> d[4];

				if(ix!=0)
					d[0]=_data.positions[iy*SheetSize+ix-1]-_data.positions[iy*SheetSize+ix];
				else
					d[0]=Vector3<>(0.0f,0.0f,0.0f);

				if(ix!=SheetSize-1)
					d[1]=_data.positions[iy*SheetSize+ix+1]-_data.positions[iy*SheetSize+ix];
				else
					d[1]=Vector3<>(0.0f,0.0f,0.0f);

				if(iy!=0)
					d[2]=_data.positions[(iy-1)*SheetSize+ix]-_data.positions[iy*SheetSize+ix];
				else
					d[2]=Vector3<>(0.0f,0.0f,0.0f);

				if(iy!=SheetSize-1)
					d[3]=_data.positions[(iy+1)*SheetSize+ix]-_data.positions[iy*SheetSize+ix];
				else
					d[3]=Vector3<>(0.0f,0.0f,0.0f);

				Vector3<> top_left = d[2]%d[0];
				Vector3<> top_right = d[1]%d[2];
				Vector3<> bottom_right = d[3]%d[1];
				Vector3<> bottom_left = d[0]%d[3];
				// normalize
				int nvalues = 0;
				if(!top_left){nvalues++;top_left=top_left/sqrtf(!top_left);}
				if(!top_right){nvalues++;top_right=top_right/sqrtf(!top_right);}
				if(!bottom_left){nvalues++;bottom_left=bottom_left/sqrtf(!bottom_left);}
				if(!bottom_right){nvalues++;bottom_right=bottom_right/sqrtf(!bottom_right);}

				_data.normals[iy*SheetSize+ix] = -(top_left+top_right+bottom_left+bottom_right)/(float)nvalues;
			}*/
		for(int ix= 0;ix<SheetSize;ix++)
			for(int iy= 0;iy<SheetSize;iy++)
			{
				_data.normals[iy*SheetSize+ix + SheetSize*SheetSize]	= -_data.normals[iy*SheetSize+ix ];
				_data.positions[iy*SheetSize+ix + SheetSize*SheetSize]	= _data.positions[iy*SheetSize+ix ];
			}
	}

	int GraphicsSheet::Step()
	{
		GenerateNormals();
		Output::Set(_data);
		return 1;
	}

	GraphicsSheet::GraphicsSheet(const guid& ObjectId,const MemHandle& h,const guid& source) :
		ObjectBase<GraphicsSheet>(ObjectId,h,source)
	{
		GenerateNormals();
		Output::Set(_data);
	}

	GraphicsSheet::~GraphicsSheet()
	{
	}

	DEFINE_CLASS_GUID(GraphicsSheet);

}
