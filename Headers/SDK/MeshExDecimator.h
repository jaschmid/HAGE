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
#include <boost/intrusive/treap_set.hpp>
#include <boost/random/mersenne_twister.hpp>

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

	const static f64 COST_EPSILON	= 0.000001;
	
	template<class _MeshType,class _Features,class _Precision> class ImplMeshDecimatorProcessor
	{
	private:
		
		typedef typename _Features::BaseMesh::Vertex Vertex;
		typedef typename _Features::BaseMesh::Edge Edge;
		typedef typename _Features::BaseMesh::Face Face;
		typedef typename _Features::BaseMesh Base;
		typedef typename Base::IndexType IndexType;
	
		struct costItem
		{	
			_Precision	cost;
			u32			edges;
			IndexType	index;
		};

	public:
		
		typedef std::function<typename Base::VertexType (typename const Base::VertexPair&,typename const Base::Edge&)> UpdateFunction;

		static typename Base::VertexType DefaultMergeFunction(typename const Base::VertexPair& vp,typename const Base::Edge& e)
		{
			Base::VertexType result;
			if(e != Base::nullEdge)
				result.Position = e->DecimationPosition;
			else
				result.Position = (vp[0]->Position + vp[1]->Position) * 0.5f;

			result.Quadric = vp[0]->Quadric + vp[1]->Quadric;
			result.nQuadrics = (vp[0]->nQuadrics + vp[1]->nQuadrics) - 4;

			return result;
		}

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
			
			data.prepare(Final()->GetNumEdgeIndices());

			for (unsigned int i = 0; i <  Final()->GetNumEdgeIndices(); i++) 
			{
				Edge e = Final()->GetEdge(i);
				
				if(e == Base::nullEdge)
					continue;

				//Compute the cost and push it to the heap
				u32 count;
				if(count = computeCollapse(e))
				{
					costItem c;
					c.index = e.Index();
					c.cost = e->Cost;
					c.edges = count;
					data.push_back(c);
				}
			}
			
		}

		

		IndexType DecimateFaces(IndexType nFaces,const UpdateFunction& func = DefaultMergeFunction)
		{
			Base::IndexType n = 0;
			while(n!=nFaces && decimate(func))
			{
				n++;

				
				if(Final()->GetNumFaces() % 10000 == 0
					|| Final()->GetNumFaces() % 10000 == 1)
				{
					f32 fMin = (f32)data.getMinErr();
					f32 fMax = (f32)data.getMaxErr();
					printf("\t %i Faces remaining, %i Collapses, ErrMin(%e) ErrMax(%e)\n",(u32)Final()->GetNumFaces(),(u32)data.size(),fMin,fMax);
				}	
			}
			return Final()->GetNumFaces();
		}

		IndexType DecimateToFaceCount(IndexType nFaces,const UpdateFunction& func = DefaultMergeFunction)
		{
			while(Final()->GetNumFaces() > nFaces && decimate(func))
			{
				if(Final()->GetNumFaces() % 10000 == 0
					|| Final()->GetNumFaces() % 10000 == 1)
				{
					f32 fMin = (f32)data.getMinErr();
					f32 fMax = (f32)data.getMaxErr();
					printf("\t %i Faces remaining, %i Collapses, ErrMin(%e) ErrMax(%e)\n",(u32)Final()->GetNumFaces(),(u32)data.size(),fMin,fMax);
				}	
			}
			return Final()->GetNumFaces();
		}

		IndexType DecimateToError(f32 fError,const UpdateFunction& func = DefaultMergeFunction)
		{
			while(decimate(func,fError))
			{
				if(Final()->GetNumFaces() % 10000 == 0
					|| Final()->GetNumFaces() % 10000 == 1)
				{
					f32 fMin = (f32)data.getMinErr();
					f32 fMax = (f32)data.getMaxErr();
					printf("\t %i Faces remaining, %i Collapses, ErrMin(%e) ErrMax(%e)\n",(u32)Final()->GetNumFaces(),(u32)data.size(),fMin,fMax);
				}	
			}
			return Final()->GetNumFaces();
		}

		void SetCustomWeightingFunction(const std::function<bool(Edge&)>& function) {_customEdgeWeightingFunction = function; }

	private:
		
		_MeshType* Final() { return static_cast<_MeshType*>(this);}
		const _MeshType* Final() const { return static_cast<const _MeshType*>(this);}
		//private members

		//custom weighting function
		typename std::function<bool(Edge&)> _customEdgeWeightingFunction;		

		//multi map for edge collapses, not strictly required but speeds things up alot

		class comparableContainer
		{
		private:

			boost::mt19937				_rand_generator;
			
			class costComparable : public boost::intrusive::bs_set_base_hook<>
			{
			public:
				costItem	item;
				bool		used;
				u32			prio;
				
			   unsigned int get_priority() const
			   {  
				   return this->prio;   
			   }

				friend bool operator < (const costComparable &a, const costComparable &b)	
				{
					if( std::abs(a.item.cost - b.item.cost) < COST_EPSILON)
						return a.item.edges < b.item.edges;
					else
						return a.item.cost < b.item.cost;
				};
				friend bool operator > (const costComparable &a, const costComparable &b)	
				{
					if( std::abs(a.item.cost - b.item.cost) < COST_EPSILON)
						return a.item.edges > b.item.edges;
					else
						return a.item.cost > b.item.cost;
				};

				friend bool priority_order (const costComparable &a, const costComparable &b)
				{  
					return a.prio < b.prio;
				}  //Lower value means higher priority
			
				friend bool priority_inverse_order (const costComparable &a, const costComparable &b)
				{  
					return a.prio > b.prio;
				}  //Lower value means higher priority
			};

		public:
			comparableContainer() : collapseMap(),_rand_generator(0)
			{
			}

			void prepare(typename Base::IndexType count)
			{
				collapseMap.clear();
				memoryVector.clear();
				memoryVector.resize(count);

				for(typename Base::IndexType i = 0; i < count; ++i)
				{
					memoryVector[i].item.index = i;
					memoryVector[i].used = false;
					memoryVector[i].prio = _rand_generator();
				}
			}

			void push_back(const costItem& c)
			{
				
				memoryVector[c.index].item=c;
				memoryVector[c.index].used = true;
				collapseMap.insert(memoryVector[c.index]);
			}
			
			void updateCollapse(const costItem& c)
			{
				if(memoryVector[c.index].used)
					collapseMap.erase(collapseMap.iterator_to(memoryVector[c.index]));

				memoryVector[c.index].item=c;
				memoryVector[c.index].used = true;
				collapseMap.insert(memoryVector[c.index]);
			}

			const costItem* pop()
			{
				if(collapseMap.empty())
					return nullptr;
				
				costComparable* result = &(*collapseMap.begin());
				collapseMap.erase(collapseMap.iterator_to(*result));
				result->used = false;
				return &(result->item);
			}

			void remove(typename Base::IndexType index)
			{
				if(memoryVector[index].used)
				{
					collapseMap.erase(collapseMap.iterator_to(memoryVector[index]));
					memoryVector[index].used = false;
				}
			}

			_Precision getMinErr() const
			{
				if(collapseMap.empty())
					return 0.0;
				return collapseMap.begin()->item.cost;
			}

			_Precision getMaxErr() const
			{
				if(collapseMap.empty())
					return 0.0;
				return collapseMap.rbegin()->item.cost;
			}

			IndexType size() const
			{
				return 0;
			}

			bool empty() const
			{
				return collapseMap.empty();
			}
		private:

			typedef typename std::vector< costComparable >     memoryType;
			memoryType memoryVector;
			typedef typename boost::intrusive::treap_multiset< costComparable , boost::intrusive::compare< std::less<costComparable> > >     collapseType;
			collapseType collapseMap;
		};

		comparableContainer	data;
		
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

		bool decimate(const UpdateFunction& func,f32 fMax = -1.0f)
		{
			if (Final()->GetNumFaces()  == 2) return false;

  
			while(!data.empty())
			{
				Edge e = getNextCollapse(fMax);

				if(e == Base::nullEdge)
					return false;

				auto vp = Final()->GetEdgeVertices(e);
				Vertex v = Base::nullVertex;
		
				Base::VertexType newVert = func(vp,e);

				//assert(vp[1].Index() != 2059);
				if((v = Final()->MergeVertex(vp)) != Base::nullVertex)
				{
					Final()->GetVertexData(v) = newVert;
					
					Edge e2 = Base::nullEdge;
					u32 nEdges = 0;
					while( (e2 = Final()->GetNextVertexEdge(v,e2) ) != Base::nullEdge)
					{
						u32 count;
						if(count = computeCollapse(e2))
							updateEdgeCollapse(e2,count);
						else
							removeEdgeCollapse(e2);

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
				if( !(e->DecimationPosition - vp[0]->Position) < 4.0*!(vp[1]->Position - vp[0]->Position) )
					bValueAccepted = true;
			}
			
			if(!bValueAccepted)
			{
				Vector4<_Precision> vM = Vector4<_Precision>((vp[0]->Position + vp[1]->Position)/2.0f,1.0f);
				Vector4<_Precision> v1 = Vector4<_Precision>(vp[0]->Position,1.0f);
				Vector4<_Precision> v2 = Vector4<_Precision>(vp[1]->Position,1.0f);
				
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
			
			u32 nEdges = 0;
			
			typename Base::ElementTriple trip;
  
			for(int vertex = 0; vertex < 2; vertex++)
				while(std::get<1>(trip=Final()->GetNextVertexElementTriple(vp[vertex],trip)) != Base::nullEdge)
				{
					//ignore edge faces
					typename Base::Face f = std::get<2>(trip);

					if(f == Base::nullFace)
						return 0;//reject edge collapses on edge

					if(f == fp[0] || f == fp[1])
						continue;

					auto vt = Final()->GetFaceVertices(f);
					std::array<Vector3<>,3> pos;

					for(int i = 0; i <3; ++i)
						pos[i] = (vt[i] == vp[vertex]) ? e->DecimationPosition : vt[i]->Position;

					
					Vector3<_Precision> d1 = (pos[1]-pos[0]);
					Vector3<_Precision> d2 = (pos[2]-pos[0]);

					Vector3<_Precision> normal_after = (d1 % d2);
					Vector3<_Precision> normal_before = ((vt[1]->Position-vt[0]->Position) % (vt[2]->Position-vt[0]->Position));
					if( normal_after*normal_before < 0.0f)
						return 0;//reject normal flipping collapses

					_Precision cos_angle = (d1*d2)/(d1.length()*d2.length());

					if(cos_angle > 0.98 || cos_angle < -0.98)
						return 0; //reject very thin triangle generating collapses

					nEdges++;
				}
			
			return _customEdgeWeightingFunction(e)?(nEdges):0;
		}

		void updateEdgeCollapse(const Edge& e,u32 nCount)
		{
			costItem c;
			c.index = e.Index();
			c.cost = e->Cost;
			c.edges = nCount;
			data.updateCollapse(c);
		}
		void removeEdgeCollapse(const Edge& e)
		{
			data.remove(e.Index());
		}

		Edge getNextCollapse(f32 fMax=-1.0f)
		{
			Edge e = Base::nullEdge;
			while(!data.empty())
			{
				const costItem* c = data.pop();
				Edge e = Final()->GetEdge(c->index);
				_Precision cost = c->cost;
				u32 count = c->edges;

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
