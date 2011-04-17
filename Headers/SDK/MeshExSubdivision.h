/********************************************************/
/* FILE: MeshExDSubdivision.h                           */
/* DESCRIPTION: Extensions for Mesh Utils               */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __MESH_EX_SUBDIVISION_H__
#define __MESH_EX_SUBDIVISION_H__

namespace HAGE {
	
	
	template<class _MeshType,class _Features> class ImplMeshSubdivisionProcessor
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
		
		typedef std::vector<std::pair<typename BaseType::VertexType,bool>> EDataT;

		typedef std::function< 
			typename bool(
				typename const BaseType::Edge&,
				typename BaseType::VertexType&,
				typename const BaseType&) > EdgeRuleFunction;

		typedef std::function< 
			typename void(
				typename const BaseType::Vertex&,
				typename BaseType::VertexType&,
				typename const EDataT&,
				typename const BaseType&) > VertexRuleFunction;


		static bool DefaultEdgeRule(
			typename const BaseType::Edge& e,
			typename BaseType::VertexType& newData,
			typename const BaseType& mesh)
		{
			auto vp = mesh.GetEdgeVertices(e);
			auto vo = mesh.GetEdgeOpposingVertices(e);
			if(vo[0] != BaseType::nullVertex && vo[1] != BaseType::nullVertex)
				newData.Position = ((vp[0]->Position + vp[1]->Position)*3.0f+vo[0]->Position+vo[1]->Position)/8.0f;
			else
				newData.Position = (vp[0]->Position + vp[1]->Position)/2.0f;
			return true;
		}
			
		static void DefaultVertexRule(
			typename const BaseType::Vertex& v,
			typename BaseType::VertexType& newData,
			typename const EDataT& edge_data,
			typename const BaseType& mesh)
		{
			auto et = mesh.GetFirstVertexElementTriple(v);

			Vector3<> sum = Vector3<>(0.0f,0.0f,0.0f);
			u32 count = 0;

			do
			{
				if(!edge_data[(size_t)std::get<1>(et).Index()].second)
				{
					continue;
				}
				sum += std::get<0>(et)->Position;
				++count;
			}
			while(std::get<0>(et = mesh.GetNextVertexElementTriple(v,et)) != BaseType::nullVertex);

			float beta;

			if(count <= 2)
				beta = 0.0f;
			else if(count == 3)
				beta = 3.0f/16.0f;
			else
				beta = 3.0f/8.0f/(float)count;

			newData.Position = v->Position * (1.0f-((float)count)*beta) + sum*beta;
		}

		void Subdivide(
			const EdgeRuleFunction& er= DefaultEdgeRule,
			const VertexRuleFunction& vr= DefaultVertexRule)
		{
			Final()->Compact();
			
			size_t sizeE = (size_t)Final()->GetNumEdgeIndices();
			size_t sizeV = (size_t)Final()->GetNumVertexIndices();
			EDataT dataE(sizeE);
			std::vector<typename BaseType::VertexType> dataV(sizeV);

			for(size_t i = 0; i < sizeE; ++i)
				dataE[i].second = er( Final()->GetEdge(i) , dataE[i].first ,*Final());
				
			for(size_t i = 0; i < sizeV; ++i)
				vr( Final()->GetVertex(i) , dataV[i], dataE ,*Final());

			BaseType::IndexType original_edges = Final()->GetNumEdgeIndices();
			BaseType::IndexType original_vertices = Final()->GetNumVertexIndices();

			std::vector<Edge> rotate_edges((size_t)Final()->GetNumFaceIndices());

			for( BaseType::IndexType i = 0; i < original_edges;++i)
				if(dataE[(size_t)i].second)
				{
					Edge e= Final()->GetEdge(i);
					if(e==BaseType::nullEdge)
						continue;

					auto fp = Final()->GetEdgeFaces(e);
					auto vp = Final()->GetEdgeVertices(e);

					Vertex v_original[2] = {BaseType::nullVertex,BaseType::nullVertex};
					for(int i_f = 0; i_f<2; ++i_f)
						if(fp[i_f] != BaseType::nullFace)
						{
							auto vt = Final()->GetFaceVertices(fp[i_f]);
							for(int i_v=0;i_v<3;++i_v)
							{
								if(vt[i_v].Index() >= original_vertices)
								{
									v_original[i_f] = BaseType::nullVertex;
									break;
								}
								else if(vt[i_v] != vp[0] && vt[i_v] != vp[1])
									v_original[i_f] = vt[i_v];
							}

							if(v_original[i_f] != BaseType::nullVertex)
							{
								//if not all 3 edges are split, don't flip edge
								auto et = Final()->GetFaceEdges(fp[i_f]);
								for(int i_e=0;i_e<3;++i_e)
									if(!dataE[(size_t)et[i_e].Index()].second)
									{
										v_original[i_f] = BaseType::nullVertex;
										break;
									}
							}
						}

					Vertex v = Final()->SplitEdge(e);

					for(int i_v = 0; i_v < 2; i_v ++)
						if(v_original[i_v] != BaseType::nullVertex)
						{
							Edge e_first = Final()->GetEdge(Final()->MakePair(v_original[i_v],v));
							assert(e_first != BaseType::nullEdge);
							assert(fp[i_v] != BaseType::nullFace);
							rotate_edges[(size_t)fp[i_v].Index()] = e_first;
						}	

					Final()->GetVertexData(v) = dataE[(size_t)i].first;
				}

			for(BaseType::IndexType i = 0; i <rotate_edges.size();++i)
				if(rotate_edges[(size_t)i] != BaseType::nullEdge)
					Final()->FlipEdge(rotate_edges[(size_t)i]);

			
			for(BaseType::IndexType i = 0; i <original_vertices;++i)
				Final()->GetVertexData(Final()->GetVertex(i)) = dataV[(size_t)i];
		}
			
	};

	struct MeshSubdivisionFeature 
	{
		template<class _T,class _Base> struct Implementation
		{
			typedef ImplMeshSubdivisionProcessor<_T,_Base> type;
		};
		typedef void VertexRequirements;
		typedef void EdgeRequirements;
		typedef void FaceRequirements;
	};
}

#endif
