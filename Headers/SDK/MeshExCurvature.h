/********************************************************/
/* FILE: MeshExDCurvature.h                             */
/* DESCRIPTION: Extensions for Mesh Utils               */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __MESH_EX_CURVATURE_H__
#define __MESH_EX_CURVATURE_H__

namespace HAGE {
	
	struct MeshGeometryCurvatureData
	{
		f32		Curvature;
	};

	
	template<class _MeshType,class _Features> class ImplMeshCurvatureProcessor
	{
	private:

		typedef typename _Features::BaseMesh::Vertex Vertex;
		typedef typename _Features::BaseMesh::Edge Edge;
		typedef typename _Features::BaseMesh::Face Face;
		typedef typename _Features::BaseMesh BaseType;
		typedef _MeshType FinalType;
		
		_MeshType* Final() { return static_cast<_MeshType*>(this);}
		const _MeshType* Final() const { return static_cast<const _MeshType*>(this);}
	public:

		f32 CalculateVertexCurvature(const Vertex& v) const
		{
			const Vector3<> &vi =v->Position;

			Vector3<> H = Vector3<>(0.0f,0.0f,0.0f);
			float A = 0.0f,cot2;
			const Vector3<>& normal = v->Normal;
  
			Vector3<> v_first,v_second;
			Vector3<> v_curr[3];

			auto curr = Final()->GetFirstVertexElementTriple(v);

			if(std::get<0>(curr) == BaseType::nullVertex)
				return 0.0f;

			v_first = std::get<0>(curr)->Position;
			curr = Final()->GetNextVertexElementTriple(v,curr);

			if(std::get<0>(curr) == BaseType::nullVertex)
				return 0.0f;

			v_second = std::get<0>(curr)->Position;
			curr = Final()->GetNextVertexElementTriple(v,curr);
			
			if(std::get<0>(curr) == BaseType::nullVertex)
				return 0.0f;
			
			v_curr[2] = v_second;
			v_curr[1] = v_first;

			do
			{	
				v_curr[0] = v_curr[1];
				v_curr[1] = v_curr[2];
				v_curr[2] = std::get<0>(curr)->Position;
				
				cot2 = getCotangents(vi,v_curr[0],v_curr[1],v_curr[2]);
		
				H+= (vi-v_curr[1])*cot2;
				A+= cot2*(vi-v_curr[1]).sqLength();
			}
			while(std::get<0>(curr = Final()->GetNextVertexElementTriple(v,curr)) != BaseType::nullVertex);
							
			cot2 = getCotangents(vi,v_curr[1],v_curr[2],v_first);
		
			H+= (vi-v_curr[2])*cot2;
			A+= cot2*(vi-v_curr[2]).sqLength();

			cot2 = getCotangents(vi,v_curr[2],v_first,v_second);
		
			H+= (vi-v_first)*cot2;
			A+= cot2*(vi-v_first).sqLength();

			return (f32)(2.0 / A * (H *  normal));
		}

		void UpdateVertexCurvature(Vertex& v)
		{
			v->Curvature = CalculateVertexCurvature(v);
		}


		void UpdateAllCurvatures()
		{
			for(FinalType::IndexType i = 0; i < Final()->GetNumVertexIndices();++i)
			{
				Vertex v = Final()->GetVertex(i);
				if(v!=BaseType::nullVertex)
					UpdateVertexCurvature(v);
			}
		}

	private:

		static f32 getCotangents(const Vector3<>& vi, const Vector3<>& vp, const Vector3<>& vj, const Vector3<>& vn)
		{
			return cotangent(vj,vp,vi) + cotangent(vi,vn,vj);
		}
		
		static f32 cotangent(const Vector3<>& a, const Vector3<>& b, const Vector3<>& c)
		{
			const Vector3<> ba = a - b;
			const Vector3<> bc = c - b;

			return bc*ba / (bc % ba).length();
		}
	};

	struct MeshCurvatureFeature 
	{
		template<class _T,class _Base> struct Implementation
		{
			typedef ImplMeshCurvatureProcessor<_T,_Base> type;
		};
		typedef set<MeshGeometryPositionData,MeshGeometryNormalData,MeshGeometryCurvatureData> VertexRequirements;
		typedef void EdgeRequirements;
		typedef void FaceRequirements;
	};
}

#endif
