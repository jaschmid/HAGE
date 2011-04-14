/********************************************************/
/* FILE: MeshExGeometry.h                               */
/* DESCRIPTION: Extensions for Mesh Utils               */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __MESH_EX_GEOMETRY_H__
#define __MESH_EX_GEOMETRY_H__

namespace HAGE {

	
	struct MeshGeometryPositionData
	{
		Vector3<>	Position;
	};
	
	struct MeshGeometryNormalData
	{
		Vector3<>	Normal;
	};

	template<class _MeshType,class _Features> class ImplMeshGeometryProcessor
	{
	private:

		typedef typename _Features::BaseMesh::Vertex Vertex;
		typedef typename _Features::BaseMesh::Edge Edge;
		typedef typename _Features::BaseMesh::Face Face;
		typedef typename _Features::BaseMesh BaseType;
		typedef _MeshType FinalType;

	public:
		ImplMeshGeometryProcessor()
		{
			static_assert( std::is_base_of<MeshGeometryPositionData,typename _MeshType::VertexType>::value , "MeshGeometryProcessor requires Vertices to inherit from MeshGeometryPositionData");
			static_assert( std::is_base_of<MeshGeometryNormalData,typename _MeshType::VertexType>::value , "MeshGeometryProcessor requires Vertices to inherit from MeshGeometryNormalData");
			static_assert( std::is_base_of<MeshGeometryNormalData,typename _MeshType::FaceType>::value , "MeshGeometryProcessor requires Vertices to inherit from MeshGeometryNormalData");
		}
		~ImplMeshGeometryProcessor()
		{
		}
		
		static Vector3<> CalculateFaceNormal(const Vector3<>& p1,const Vector3<>& p2,const Vector3<>& p3)
		{
			return ((p2-p1) % (p3-p1)).normalize();
		}

		Vector3<> CalculateFaceNormal(const Face& f) const
		{
			auto vp = Final()->GetFaceVertices(f);
			return CalculateFaceNormal(vp[0]->Position,vp[1]->Position,vp[2]->Position);
		}

		void UpdateFaceNormal(const Face& f)
		{
			f->Normal = CalculateFaceNormal(f);
		}

		void UpdateVertexNormal(const Vertex& v)
		{
			Face f = BaseType::nullFace;

			Vector3<> accumulator = Vector3<>(0.0f,0.0f,0.0f);

			while( (f = Final()->GetNextVertexFace(v,f)) != BaseType::nullFace)
				accumulator += f->Normal;

			v->Normal = accumulator.normalize();
		}

		void UpdateAllNormals()
		{
			//not really optimized but works
			for(FinalType::IndexType i = 0; i < Final()->GetNumFaceIndices();++i)
			{
				Face f = Final()->GetFace(i);
				if(f!=BaseType::nullFace)
					UpdateFaceNormal(f);
			}

			for(FinalType::IndexType i = 0; i < Final()->GetNumVertexIndices();++i)
			{
				Vertex v = Final()->GetVertex(i);
				if(v!=BaseType::nullVertex)
					UpdateVertexNormal(v);
			}
		}
	private:

		_MeshType* Final() { return static_cast<_MeshType*>(this);}
		const _MeshType* Final() const { return static_cast<const _MeshType*>(this);}
	};
	
	struct MeshGeometryFeature 
	{
		template<class _T,class _Base> struct Implementation
		{
			typedef ImplMeshGeometryProcessor<_T,_Base> type;
		};
		typedef set<MeshGeometryPositionData,MeshGeometryNormalData> VertexRequirements;
		typedef void EdgeRequirements;
		typedef set<MeshGeometryNormalData> FaceRequirements;
	};

}

#endif
