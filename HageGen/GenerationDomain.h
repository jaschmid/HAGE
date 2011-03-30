#ifndef GENERATION__DOMAIN__INCLUDED
#define GENERATION__DOMAIN__INCLUDED

#include "header.h"

namespace HAGE {

class GenerationDomain : public DomainBase<GenerationDomain>
{
	public:
		GenerationDomain();
		~GenerationDomain();
		void DomainStep(t64 time);

	private:
		

		const static int xBegin = 35;
		const static int xEnd = 40;
		const static int yBegin = 35;
		const static int yEnd = 40;
		
		const static int numToLoad = (xEnd-xBegin+1)*(yEnd-yBegin+1);

		TResourceAccess<IMeshData> data[xEnd-xBegin+1][yEnd-yBegin+1];
		bool loaded[xEnd-xBegin+1][yEnd-yBegin+1];

		typedef CEditableMesh<> MeshType;
		MeshType	_mesh;
		Vector3<> min,max;//extents of the mesh
		
		void writeOutputMesh();
		void mergeMeshVertices();

		virtual bool MessageProc(const Message* pMessage);
};

}

#endif