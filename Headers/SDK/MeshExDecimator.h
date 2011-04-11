/********************************************************/
/* FILE: MeshExDecimator.h                              */
/* DESCRIPTION: Extensions for Mesh Utils               */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __MESH_EX_DECIMATOR_H__
#define __MESH_EX_DECIMATOR_H__

#include <cmath>

namespace HAGE {


	
	template<class _Precision> struct MeshDecimatorQuadricData
	{
		Matrix4<_Precision>	Quadric;
		u32					nQuadrics;
	};

	template<class _Precision> struct MeshDecimatorEdgeData
	{
		_Precision			Cost;
		Vector3<_Precision>	DecimationPosition;
	};

	const static f64 COST_EPSILON	= 0.00001;
	
	template<class _MeshType,class _Features,class _Precision> class ImplMeshDecimatorProcessor
	{
	private:

		typedef typename _Features::BaseMesh::Vertex Vertex;
		typedef typename _Features::BaseMesh::Edge Edge;
		typedef typename _Features::BaseMesh::Face Face;
		typedef typename _Features::BaseMesh Base;
		typedef typename Base::IndexType IndexType;
	

		struct costComparable
		{
			_Precision	cost;
			u32			edges;

			bool operator < (const costComparable& other) const	
			{
				if( std::abs(other.cost - cost) < COST_EPSILON)
					return edges < other.edges;
				else
					return cost < other.cost;
			};
		};

	public:

		ImplMeshDecimatorProcessor()
		{
			static_assert( std::is_base_of<MeshGeometryPositionData,typename _MeshType::VertexType>::value , "MeshDecimatorProcessor requires Vertices to inherit from MeshGeometryPositionData");
			static_assert( std::is_base_of<MeshDecimatorQuadricData<_Precision>,typename _MeshType::VertexType>::value , "MeshDecimatorProcessor requires Vertices to inherit from MeshDecimatorQuadricData");
			static_assert( std::is_base_of<MeshGeometryNormalData,typename _MeshType::FaceType>::value , "MeshDecimatorProcessor requires Vertices to inherit from MeshGeometryNormalData");

			_customEdgeWeightingFunction = [](Edge& e) -> bool {return true;};
		}
		~ImplMeshDecimatorProcessor()
		{
		}

		void InitializeDecimate()
		{
			Final()->Compact();
			Final()->UpdateAllNormals();
			  
			for (unsigned int i = 0; i <  Final()->GetNumVertexIndices(); i++) 
			{
				Vertex v = Final()->GetVertex(i);

				if(v == Base::nullVertex)
					continue;

				// Compute the cost and push it to the heap
				v->Quadric = createQuadricForVert(v,&v->nQuadrics);
			}
			
			lookup.resize(Final()->GetNumEdgeIndices());

			for (unsigned int i = 0; i <  Final()->GetNumEdgeIndices(); i++) 
			{
				Edge e = Final()->GetEdge(i);
				
				lookup[i] = collapseMap.end();

				if(e == Base::nullEdge)
					continue;

				//Compute the cost and push it to the heap
				u32 count;
				if(count = computeCollapse(e))
					updateEdgeCollapse(e,count);
				else
					lookup[i] = collapseMap.end();
			}

		}

		IndexType DecimateFaces(IndexType nFaces)
		{
			Base::IndexType n = 0;
			while(n!=nFaces && decimate())
			{
				n++;

				
				if(Final()->GetNumFaces() % 1000 == 0
					|| Final()->GetNumFaces() % 1000 == 1)
				{
					float fMin = collapseMap.begin()->first.cost;
					float fMax = collapseMap.rbegin()->first.cost;
					printf("\t %i Faces remaining, %i Collapses, ErrMin(%e) ErrMax(%e)\n",(u32)Final()->GetNumFaces(),(u32)collapseMap.size(),fMin,fMax);
				}	
			}
			return Final()->GetNumFaces();
		}

		IndexType DecimateToFaceCount(IndexType nFaces)
		{
			while(Final()->GetNumFaces() > nFaces && decimate())
			{
				if(Final()->GetNumFaces() % 1000 == 0
					|| Final()->GetNumFaces() % 1000 == 1)
				{
					float fMin = collapseMap.begin()->first.cost;
					float fMax = collapseMap.rbegin()->first.cost;
					printf("\t %i Faces remaining, %i Collapses, ErrMin(%e) ErrMax(%e)\n",(u32)Final()->GetNumFaces(),(u32)collapseMap.size(),fMin,fMax);
				}	
			}
			return Final()->GetNumFaces();
		}

		IndexType DecimateToError(f32 fError)
		{
			while(decimate(fError))
			{
				if(Final()->GetNumFaces() % 1000 == 0
					|| Final()->GetNumFaces() % 1000 == 1)
				{
					float fMin = collapseMap.begin()->first.cost;
					float fMax = collapseMap.rbegin()->first.cost;
					printf("\t %i Faces remaining, %i Collapses, ErrMin(%e) ErrMax(%e)\n",(u32)Final()->GetNumFaces(),(u32)collapseMap.size(),fMin,fMax);
				}	
			}
			return Final()->GetNumFaces();
		}

		void SetCustomWeightingFunction(const std::function<bool(Edge&)>& function) {_customEdgeWeightingFunction = function; }
		
		_MeshType* Final() { return static_cast<_MeshType*>(this);}
		const _MeshType* Final() const { return static_cast<const _MeshType*>(this);}

	private:

		//private members

		//custom weighting function
		typename std::function<bool(Edge&)> _customEdgeWeightingFunction;		

		//multi map for edge collapses, not strictly required but speeds things up alot
		typedef typename std::multimap<costComparable,IndexType> collapseType;
		collapseType collapseMap;
		std::vector<typename collapseType::iterator>	lookup;

		//private methods
		
		Matrix4<_Precision> createQuadricForVert(Vertex v,u32* nCount) const
		{
			Matrix4<f64> Q = Matrix4<_Precision>::Zero();

			Face f = Base::nullFace;
			*nCount = 0;
			while( (f = Final()->GetNextVertexFace(v,f)) != Base::nullFace )
			{
				Vector4<f64> planeEq = 
					Vector4<f64>(
						f->Normal,
						-f->Normal * v->Position
					);
				Q += Matrix4<f64>::OuterProduct(planeEq,planeEq);
				(*nCount)++;
			}

			return Q;
		}

		bool decimate(f32 fMax = -1.0f)
		{
			if (Final()->GetNumFaces()  == 2) return false;

  
			while(!collapseMap.empty())
			{
				Edge e = getNextCollapse(fMax);

				if(e == Base::nullEdge)
					return false;

				auto vp = Final()->GetEdgeVertices(e);
				Vertex v = Base::nullVertex;
		
				Vector3<_Precision> pos = e->DecimationPosition;
				Matrix4<_Precision> qNew = vp[0]->Quadric + vp[1]->Quadric;
				u32 newCount = (vp[0]->nQuadrics + vp[1]->nQuadrics) - 4;
				//assert(vp[1].Index() != 2059);
				if((v = Final()->MergeVertex(vp)) != Base::nullVertex)
				{
					v->Position = pos;
					v->Quadric = qNew;
					v->nQuadrics = newCount;
					
					Edge e2 = Base::nullEdge;
					u32 nEdges = 0;
					while( (e2 = Final()->GetNextVertexEdge(v,e2) ) != Base::nullEdge)
					{
						u32 count;
						if(count = computeCollapse(e2))
							updateEdgeCollapse(e2,count);
						else if(lookup[e2.Index()] != collapseMap.end())
						{
							collapseMap.erase(lookup[e2.Index()]);
							lookup[e2.Index()] =  collapseMap.end();
						}

						++nEdges;
					}

					//printf("\tCollapsed Vertex is now connected to %u Edges\n",nEdges);

					//Final()->DebugValidateMesh();
					return true;
				}
			}

			return false;
		}
		
		u32 computeCollapse(Edge& e)
		{
			//need double precision or we get serious error creep
			assert(e != Base::nullEdge);
			auto vp = Final()->GetEdgeVertices(e);

			_Precision det;
			Matrix4<_Precision> QEdge = vp[0]->Quadric + vp[1]->Quadric;
			Matrix4<_Precision> QEdgeInv = Matrix4<_Precision>(QEdge.Row(0),QEdge.Row(1),QEdge.Row(2),Vector4<_Precision>(0.0,0.0,0.0,1.0)).Invert(&det);
			bool bValueAccepted = false;

			if(!QEdgeInv.IsNaN() && det > COST_EPSILON)
			{
				Vector4<_Precision> vIdeal = QEdgeInv*Vector4<_Precision>(0.0f,0.0f,0.0f,1.0f);
				e->DecimationPosition = vIdeal.xyz();
				e->Cost = vIdeal*(QEdge*vIdeal);

				//sanity check, if we moved out point by more than 2 times the edge length, something is wrong
				//prefer the other solution then
				if( !(e->DecimationPosition - vp[0]->Position) < 4*!(vp[1]->Position - vp[0]->Position) )
					bValueAccepted = true;
			}
			
			if(!bValueAccepted)
			{
				Vector4<_Precision> vM = Vector4<f64>((vp[0]->Position + vp[1]->Position)/2.0f,1.0f);
				Vector4<_Precision> v1 = Vector4<f64>(vp[0]->Position,1.0f);
				Vector4<_Precision> v2 = Vector4<f64>(vp[1]->Position,1.0f);
				
				_Precision eVM = vM*(QEdge*vM);
				_Precision eV1 = v1*(QEdge*v1);
				_Precision eV2 = v2*(QEdge*v2);
				if( eVM < eV1 && eVM < eV2)
				{
					e->DecimationPosition = vM.xyz();
					e->Cost = eVM;
				}
				else if( eV1 < eV2)
				{
					e->DecimationPosition = v1.xyz();
					e->Cost = eV1;
				}
				else
				{
					e->DecimationPosition = v2.xyz();
					e->Cost = eV2;
				}
			}
		
			if(e->Cost < 0.0f)
				e->Cost = 0.0f;

			//prevent normal flips

			auto fp = Final()->GetEdgeFaces(e);

			Face f = Base::nullFace;

			u32 nEdges = 0;

			for(int vertex = 0; vertex < 2; vertex++)
				while((f=Final()->GetNextVertexFace(vp[0],f)) != Base::nullFace)
				{
					//ignore edge faces
					if(f == fp[0] || f == fp[1])
						continue;

					auto vt = Final()->GetFaceVertices(f);
					std::array<Vector3<>,3> pos;

					for(int i = 0; i <3; ++i)
						pos[i] = (vt[i] == vp[vertex]) ? e->DecimationPosition : vt[i]->Position;

					//if dotproduct < 0 then we have a normal flip
					if( ((pos[1]-pos[0]) % (pos[2]-pos[0]))*((vt[1]->Position-vt[0]->Position) % (vt[2]->Position-vt[0]->Position)) < 0.0f)
						return 0;

					nEdges++;
				}
			
			return _customEdgeWeightingFunction(e)?(nEdges):0;
		}

		void updateEdgeCollapse(const Edge& e,u32 nCount)
		{
			if(lookup[e.Index()] != collapseMap.end())
				collapseMap.erase(lookup[e.Index()]);
			costComparable c;
			c.edges = nCount;
			c.cost = e->Cost;
			lookup[e.Index()] = collapseMap.insert(collapseType::value_type(c,e.Index()));
		}

		Edge getNextCollapse(f32 fMax=-1.0f)
		{
			Edge e = Base::nullEdge;
			while(!collapseMap.empty())
			{
				Edge e = Final()->GetEdge(collapseMap.begin()->second);
				_Precision cost = collapseMap.begin()->first.cost;
				u32 count = collapseMap.begin()->first.edges;
				lookup[collapseMap.begin()->second] = collapseMap.end();
				collapseMap.erase(collapseMap.begin());

				if(e == Base::nullEdge)
					continue;

				assert(e->Cost == cost);

				if(fMax <= 0.0f || cost < fMax)
					return e;
				else
				{
					updateEdgeCollapse(e,count);
					return Base::nullEdge;
				}
			}

			return Base::nullEdge;
		}
	};

	template< class _Precision = f32> struct MeshDecimatorFeature 
	{
		template<class _T,class _Base> struct Implementation
		{
			typedef ImplMeshDecimatorProcessor<_T,_Base,_Precision> type;
		};
		typedef set<MeshGeometryPositionData,MeshDecimatorQuadricData<_Precision>> VertexRequirements;
		typedef set<MeshDecimatorEdgeData<_Precision>> EdgeRequirements;
		typedef set<MeshGeometryNormalData> FaceRequirements;
	};

}

#endif
