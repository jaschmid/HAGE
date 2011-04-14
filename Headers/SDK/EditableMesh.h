/********************************************************/
/* FILE: EditableMesh.h                                 */
/* DESCRIPTION: Defines an Editable Mesh Class          */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __EDITABLE_MESH_H__
#define __EDITABLE_MESH_H__

#include <tuple>
#include <list>
#include <vector>
#include <array>

namespace HAGE {

namespace _EditableMeshInternal {

	/*****************************************************************/
	/* template class that implements method to retrieve vertex data */
	/*****************************************************************/

	template<class _VertexType,class _Final> class _CEditableMesh_GetVertexData
	{
	public:
		_VertexType& GetVertexData(const typename _Final::Vertex& vertex)
		{
			return ((_Final*)this)->getInternal(vertex)->data.contents;
		}
		const _VertexType& GetVertexData(const typename _Final::Vertex& vertex) const
		{
			return ((_Final*)this)->getInternal(vertex)->data.contents;
		}
	};

	template<class _Final> class _CEditableMesh_GetVertexData<void,_Final>
	{
	};

	/***************************************************************/
	/* template class that implements method to retrieve edge data */
	/***************************************************************/

	template<class _EdgeType,class _Final> class _CEditableMesh_GetEdgeData
	{
	public:
		_EdgeType& GetEdgeData(const typename _Final::Edge& edge)
		{
			return ((_Final*)this)->getInternal(edge)->data.contents;
		}
		const _EdgeType& GetEdgeData(const typename _Final::Edge& edge) const
		{
			return ((_Final*)this)->getInternal(edge)->data.contents;
		}
	};


	/***************************************************************/
	/* template class that implements method to retrieve face data */
	/***************************************************************/

	template<class _Final> class _CEditableMesh_GetEdgeData<void,_Final>
	{
	};

	template<class _FaceType,class _Final> class _CEditableMesh_GetFaceData
	{
	public:
		_FaceType& GetFaceData(const typename _Final::Face& face)
		{
			return ((_Final*)this)->getInternal(face)->data.contents;
		}
		const _FaceType& GetFaceData(const typename _Final::Face& face) const
		{
			return ((_Final*)this)->getInternal(face)->data.contents;
		}
	};

	template<class _Final> class _CEditableMesh_GetFaceData<void,_Final>
	{
	};

	
	/****************************/
	/* Editable Mesh Base Class */
	/****************************/

	template<class _VertexType, class _FaceType, class _EdgeType,class _Final> class _CEditableMeshBase
	{
	public:

		// Types 

		typedef _VertexType VertexType;
		typedef _FaceType FaceType;
		typedef _EdgeType EdgeType;
		typedef _CEditableMeshBase<_VertexType, _FaceType, _EdgeType, _Final> BaseType;
		typedef _Final ThisType;
		typedef u64 IndexType;

		static const int PolySize = 3;

		typedef enum _RAW_INDEX_FORMAT
		{
			RAW_INDEX_TRIANGLE_LIST = 0,
			RAW_INDEX_TRIANGLE_STRIP = 1,
			RAW_INDEX_TRIANGLE_FAN = 2
		} RAW_INDEX_FORMAT;

		_CEditableMeshBase() : _unusedEdges(0),_unusedVertices(0),_unusedFaces(0) {}
	private:
		template<class _internal,class _data> class _InternalConvert
		{
		public:
			operator const _data&() const
			{
				return ((_internal*)(this))->internal_data->data.contents;
			}

			template<class _target> operator const _target() const
			{
				return (const _target)((_internal*)(this))->internal_data->data.contents;
			}

			_data* operator ->()  const
			{
				return &((_internal*)(this))->internal_data->data.contents;
			}
		};

		template<class _internal> class _InternalConvert<_internal,void>
		{
		public:
		};

		class _HE_edge;
		class _HE_edge_pair;
		class _HE_vert;
		class _HE_face;
	public:
		typedef class _Edge : public _InternalConvert<_Edge,EdgeType>
		{ 
		private:
			_HE_edge_pair* internal_data; 
			_Edge(_HE_edge_pair* val) : internal_data(val) {}
		public:
			IndexType Index() const
			{
				return internal_data->index;
			}

			bool operator == (const _Edge& e) const
			{
				return internal_data == e.internal_data;
			}
			bool operator != (const _Edge& e) const
			{
				return internal_data != e.internal_data;
			}

			friend class BaseType;
		} Edge;

		typedef class _Face : public _InternalConvert<_Face,FaceType>
		{ 
		private:
			_HE_face* internal_data; 
			_Face(_HE_face* val) : internal_data(val) {}
		public:
			IndexType Index() const
			{
				return internal_data->index;
			}

			bool operator == (const _Face& e) const
			{
				return internal_data == e.internal_data;
			}
			bool operator != (const _Face& e) const
			{
				return internal_data != e.internal_data;
			}

			friend class BaseType;
		} Face;
	
		typedef class _Vertex : public _InternalConvert<_Vertex,VertexType>
		{ 
		private:
			_HE_vert* internal_data; 
			_Vertex(_HE_vert* val) : internal_data(val) {}
		public:
			IndexType Index() const
			{
				return internal_data->index;
			}

			bool operator == (const _Vertex& e) const
			{
				return internal_data == e.internal_data;
			}
			bool operator != (const _Vertex& e) const
			{
				return internal_data != e.internal_data;
			}
			friend class BaseType;
		} Vertex;
	
		typedef std::array<Face,2> FacePair;
		typedef std::array<Vertex,2> VertexPair;
		typedef std::array<Vertex,PolySize> VertexTriple;
		typedef std::array<Edge,PolySize> EdgeTriple;
		typedef std::tuple<Vertex,Edge,Face> ElementTriple;

		static const Edge nullEdge;
		static const Vertex nullVertex;
		static const Face nullFace;

		// Methods 

		static FacePair MakePair(const Face& f1,const Face& f2)
		{
			FacePair fp = {{f1,f2}};
			return fp;
		}
		static VertexPair MakePair(const Vertex& v1,const Vertex& v2)
		{
			VertexPair vp = {{v1,v2}};
			return vp;
		}
		static EdgeTriple MakeTriple(const Edge& e1,const Edge& e2,const Edge& e3)
		{
			EdgeTriple et = {{e1,e2,e3}};
			return et;
		}
		static VertexTriple MakeTriple(const Vertex& v1,const Vertex& v2,const Vertex& v3)
		{
			VertexTriple vt = {{v1,v2,v3}};
			return vt;
		}

		IndexType GetNumFaces() const
		{
			return (IndexType)_faces.size() - _unusedFaces;
		}
		IndexType GetNumEdges() const
		{
			return (IndexType)_edges.size() - _unusedEdges;
		}
		IndexType GetNumVertices() const
		{
			return (IndexType)_vertices.size() - _unusedVertices;
		}

		IndexType GetNumFaceIndices() const
		{
			return (IndexType)_faces.size();
		}
		IndexType GetNumEdgeIndices() const
		{
			return (IndexType)_edges.size();
		}
		IndexType GetNumVertexIndices() const
		{
			return (IndexType)_vertices.size();
		}

		Vertex GetVertex(IndexType index) const { if(index >= GetNumVertexIndices()) return nullVertex; else return getExternal(_vertices[(size_t)index]);}
		IndexType GetIndex(const Vertex& v) const { return getInternal(v)->index;}
		Vertex GetVertex(const Vertex& _v, const Edge& _e) const
		{
			const HE_vert* v = getInternal(_v);
			const HE_edge* e = getInternal(_e);

			if(e->end_vertex == v)
				return getExternal(e->pair_edge->end_vertex);
			else if(e->pair_edge->end_vertex == v)
				return getExternal(e->end_vertex);
			else
				return nullVertex;
		}
	
		Edge GetEdge(IndexType index) const { if(index >= GetNumEdgeIndices()) return nullEdge; else return getExternal(_edges[(size_t)index]);}
		IndexType GetIndex(const Edge& e) const { return getInternal(e)->index;}
		Edge GetEdge(const VertexPair& _v) const
		{
			const HE_vert* v1 = getInternal(_v[0]);
			const HE_vert* v2 = getInternal(_v[1]);
			
			if( !isValid(v1) || !isValid(v2))
				return nullEdge;

			const HE_edge* candidate = v1->edge;

			//search ccw
			do
			{
				if(candidate -> end_vertex == v2)
					return getExternal(candidate);

				candidate = candidate->pair_edge->next_edge;
			}
			while(candidate != v1->edge);
			//we rotated a full circle
			return nullEdge;
		}
		Edge GetEdge(const FacePair& _f) const
		{
			const HE_face* f1 = getInternal(_f[0]);
			const HE_face* f2 = getInternal(_f[1]);
			
			if( !isValid(f1) || !isValid(f2))
				return nullEdge;

			
			const HE_edge* candidate = f1->edge;

			for(int i = 0; i < PolySize; ++i)
			{
				if(candidate->pair_edge->face == f2)
					return getExternal(candidate);
				candidate = candidate->next_edge;
			}

			return nullEdge;
		}
	
		Face GetFace(IndexType index) const { assert(index < GetNumFaceIndices()); return getExternal(_faces[(size_t)index]);}
		IndexType GetIndex(const Face& f) const { return getInternal(f)->index;}
		Face GetFace(const VertexTriple& _v) const
		{
			static_assert( PolySize == 3 , "function assumes PolySize == 3" );
			const HE_vert* v1 = getInternal(_v[0]);
			const HE_vert* v2 = getInternal(_v[1]);
			const HE_vert* v3 = getInternal(_v[2]);
			
			if( !isValid(v1) || !isValid(v2) || !isValid(v3))
				return nullFace;

			 HE_edge* current = v1->edge;

			 do
			 {
				 //only if we have a face associated
				 if(current->end_vertex == v2)
				 {
					if(current->next_edge->end_vertex == v3)
						return getExternal(current->face);
					else
						return nullFace;
				 }

				 current = current->pair_edge->next_edge;
			 }
			 while(current != v1->edge);

			 //did full circle
			 return nullFace;

		}
		Face GetFace(const EdgeTriple& _e) const
		{
			static_assert( PolySize == 3 , "function assumes PolySize == 3" );
			const HE_edge* e1 = getInternal(_e[0]);
			const HE_edge* e2 = getInternal(_e[1]);
			const HE_edge* e3 = getInternal(_e[2]);

			if( !isValid(e1) || !isValid(e2) || !isValid(e3))
				return nullFace;
			
			//check this face
			if( (e1->next_edge == e2 && e1->next_edge->next_edge == e3) ||
				(e1->next_edge == e3 && e1->next_edge->next_edge == e2) )
			{
				return getExternal(e1->face);
			}

			e1 = e1->pair_edge;

			//check opposite face
			if( (e1->next_edge == e2 && e1->next_edge->next_edge == e3) ||
				(e1->next_edge == e3 && e1->next_edge->next_edge == e2) )
			{
				return getExternal(e1->face);
			}
				
			//did full circle
			return nullFace;
		}

		EdgeTriple GetFaceEdges(const Face& face) const
		{
			const HE_face* f = getInternal(face);
			EdgeTriple result = {{nullEdge,nullEdge,nullEdge}};
			if(!isValid(f))
				return result;

			const HE_edge* e= f->edge;

			for(int i = 0; i < PolySize; ++i)
			{
				result[i] = getExternal(e);
				e = e->next_edge;
			}

			return result;
		}
		VertexTriple GetFaceVertices(const Face& face) const
		{
			const HE_face* f = getInternal(face);
			VertexTriple result = {{nullVertex,nullVertex,nullVertex}};
			if(!isValid(f))
				return result;

			const HE_edge* e= f->edge;

			for(int i = 0; i < PolySize; ++i)
			{
				result[i] = getExternal(e->end_vertex);
				e = e->next_edge;
			}

			return result;
		}

		VertexPair GetEdgeVertices(const Edge& edge) const
		{
			const HE_edge* e = getInternal(edge);
			VertexPair result = {{nullVertex,nullVertex}};
			if(!isValid(e))
				return result;

			result[0] = getExternal(e->end_vertex);
			result[1] = getExternal(e->pair_edge->end_vertex);

			return result;
		}
		FacePair GetEdgeFaces(const Edge& edge) const
		{
			const HE_edge* e = getInternal(edge);
			FacePair result = {{nullFace,nullFace}};
			if(!isValid(e))
				return result;

			result[0] = getExternal(e->face);
			result[1] = getExternal(e->pair_edge->face);

			return result;
		}

		Edge GetFirstVertexEdge(const Vertex& vertex) const { return GetNextVertexEdge(vertex,nullEdge); }
		Face GetFirstVertexFace(const Vertex& vertex) const { return GetNextVertexFace(vertex,nullFace); }
		ElementTriple GetFirstVertexElementTriple(const Vertex& vertex) const { return GetNextVertexElementTriple(vertex,ElementTriple(nullVertex,nullEdge,nullFace)); }

		Edge GetNextVertexEdge(const Vertex& vertex,const Edge& previous_edge) const
		{
			const HE_vert* v = getInternal(vertex);
			const HE_edge* e = getInternal(previous_edge);
			
			if(!isValid(v) || (v->edge == nullptr) )
				return nullEdge;

			if(!isValid(e))
				return getExternal(v->edge);

			// we might get the wrong one
			if(e->end_vertex != v)
				e = e->pair_edge;

			assert(e->end_vertex == v);

			// return next ccw
			if(e->next_edge == v->edge)
				return nullEdge;
			else
				return getExternal(e->next_edge);
		}

		
		ElementTriple GetNextVertexElementTriple(const Vertex& vertex,const ElementTriple& previous_triple) const
		{
			const HE_vert* v = getInternal(vertex);
			const HE_edge* e = getInternal(std::get<1>(previous_triple));
			
			if(!isValid(v) || (v->edge == nullptr) )
				return ElementTriple(nullVertex,nullEdge,nullFace);

			if(!isValid(e))
				return ElementTriple(getExternal(v->edge->end_vertex),getExternal(v->edge),getExternal(v->edge->face));

			// we might get the wrong one
			if(e->end_vertex != v)
				e = e->pair_edge;

			assert(e->end_vertex == v);

			// return next ccw
			if(e->next_edge == v->edge)
				return ElementTriple(nullVertex,nullEdge,nullFace);
			else
				return ElementTriple(getExternal(e->next_edge->end_vertex),getExternal(e->next_edge),getExternal(e->next_edge->face));
		}

		Face GetNextVertexFace(const Vertex& vertex,const Face& previous_face) const
		{
			const HE_vert* v = getInternal(vertex);
			const HE_face* f = getInternal(previous_face);
			
			if(!isValid(v) || (v->edge == nullptr) )
				return nullFace;

			//depending if inside or outside
			const HE_face* first_face = (v->edge->face) ? (v->edge->face) : (v->edge->pair_edge->face);

			if(!isValid(f))
				return getExternal(first_face);

			// find edge with v as end
			HE_edge* e = f->edge;
			for(int i = 0; i<3;i++)
			{
				if(e->end_vertex == v)
					break;
				e = e->next_edge;
			}
			//end vertex has to be v now or previous_face was not a face around v
			assert(e->end_vertex ==v);

			// return next ccw
			e = e->next_edge->pair_edge;

			//make sure it's not outside
			if(e->face)
			{
				if(e->face ==first_face)
					return nullFace;
				else
					return getExternal(e->face);
			}
			else
			{
				if(e->next_edge->pair_edge->face == first_face)
					return nullFace;
				else
					return getExternal(e->next_edge->pair_edge->face);
			}
		}

		void InvalidateFace(const Face& face)
		{

			HE_face* f = getInternal(face);

			if(!isValid(f))
				return;

			HE_edge* e[PolySize];

			HE_edge* cur = f->edge;
			for(int i = 0 ; i<PolySize;++i)
			{
				e[i] = cur;
				cur->face = nullptr;
				cur = cur->next_edge;
			}

			assert(cur = f->edge);
			f->edge = nullptr;

			internalRemove(f);
			
			//check for degenerate edges
			for(int i = 0 ; i < PolySize ; ++i)
				removeDegenerateEdge(e[i]);
		}

		void InvalidateEdge(const Edge& Edge)
		{
			FacePair p = GetEdgeFaces(Edge);
			InvalidateFace(p[0]);
			InvalidateFace(p[1]);
		}
		void InvalidateVertex(const Vertex& v)
		{
			HE_vertex* _v = getInternal(v);
			if(_v->face == nullptr)
			{
				//orphaned vertex
				internalRemove(v);
				return;
			}

			Face f=GetFirstVertexFace(v);
			do
				InvalidateFace(f);
			while((f =GetFirstVertexFace(v)) != nullFace);
		}

		bool IsValidVertexMerge(const VertexPair& vpair)
		{
			if(vpair[0] == vpair[1])
				return false;

			HE_vert* v1 = getInternal(vpair[0]),* v2 = getInternal(vpair[1]);
				
			//try new test function

			_edgeAdjectencyData data;

			if(!findMergeEdgesEx(v1,v2,data))
				return false;

			return true;
		}

		Vertex MergeVertex(const VertexPair& vpair)
		{
			if(vpair[0] == vpair[1])
				return nullVertex;

			HE_vert* v1 = getInternal(vpair[0]),* v2 = getInternal(vpair[1]);
				
			//try new test function

			_edgeAdjectencyData data;

			if(!findMergeEdgesEx(v1,v2,data))
				return nullVertex;

			bool bEdge = false;
						
			HE_edge* v1_next=data.v0_next,*v1_prev=data.v0_prev;
			HE_edge* v2_next=data.v1_next,*v2_prev=data.v1_prev;

			if(data.edge)
			{
				HE_edge* s = data.edge;

				InvalidateEdge(getExternal(s));
				bEdge = true;

				if(v1_prev)
					assert(v1_prev->next_edge == v1_next);
				else
					assert(v1->edge == nullptr);
				if(v2_prev)
					assert(v2_prev->next_edge == v2_next);
				else
					assert(v2->edge == nullptr);
			}
			else
			{				
				//if we have exclusive edges, fix them
				
				if(v1_prev && v2_prev)
				{
					if(v1_next->end_vertex == v2_prev->pair_edge->end_vertex && v1_next->next_edge != v2_prev)
					{
						if(!twistEdgeConnect(v1_next,v2_prev))
							return nullVertex;

						assert(getPrevEdge(v2_prev) == v1_next);
					}

					if(v2_next->end_vertex == v1_prev->pair_edge->end_vertex && v2_next->next_edge != v1_prev)
					{
						if(!twistEdgeConnect(v2_next,v1_prev))
							return nullVertex;

						assert(getPrevEdge(v1_prev) == v2_next);
					}

					if(v2_prev->next_edge != v2_next)
					{
						if(!twistEdgeConnect(v2_prev,v2_next))
							return nullVertex;
					}

					if(v1_prev->next_edge != v1_next)
					{
						if(!twistEdgeConnect(v1_prev,v1_next))
							return nullVertex;
					}
				
					assert(getPrevEdge(v1_next) == v1_prev);
					assert(getPrevEdge(v2_next) == v2_prev);
				}
			}
				
			//we're now certain so we can start changing stuff
			if(v1->edge)
			{
				HE_edge* cur = v1->edge;
				do
				{
					cur->pair_edge->end_vertex = v2;
					cur = cur->pair_edge->next_edge;
				}
				while(cur != v1->edge);		
			}
			
			if(v1_next && v2_next)
			{

				if(v1_next->next_edge == v2_prev)
					mergeDoubleEdges(v1_next,v2_prev);
				else
					v2_prev->next_edge = v1_next;

				if(v2_next->next_edge == v1_prev)
					mergeDoubleEdges(v2_next,v1_prev);
				else
					v1_prev->next_edge = v2_next;

			}
			else if(!v2->edge)
				v2->edge = v1->edge;

			internalRemove(v1);	

			return getExternal(v2);
		}

		void DebugValidateMesh() const 
		{
			const HE_face* f=nullptr;
			const HE_edge* e =nullptr;
			u32 nEdges = 0;
			for(size_t i = 0; i<_faces.size();++i)
				if(f = _faces[i])
				{
					e = f->edge;
					assert(e);
					for(int ie=0;ie<PolySize;++ie)
					{
						e=e->next_edge;
						assert(e->end_vertex);
						assert(e->pair_edge->pair_edge ==e);
						assert(e->next_edge->pair_edge->end_vertex == e->end_vertex);
						assert(e->face == f);

						const HE_edge* s = e;
						const HE_edge* ps = getPrevEdge(s);
						const HE_edge* psp = getPrevEdge(s->pair_edge);
					
						assert(ps->next_edge == s);
						assert(getPrevEdge(s->next_edge) == s);
						assert(e->end_vertex->edge);
						assert(_vertices[(size_t)e->end_vertex->index]);
						assert(s->next_edge->next_edge->end_vertex != s->end_vertex);
						assert(ps->end_vertex == s->pair_edge->end_vertex);
						assert(psp->end_vertex == s->end_vertex);
						
					
						++nEdges;
					}

					assert(e==f->edge);
				}
			//rest of edges must be outer
			HE_edge_pair* p;
			e = nullptr;
			for(size_t i = 0; i<_edges.size();++i)
				if(p = _edges[i])
					for(int ie=0;ie<2;ie++)
						if(((e = &p->edges[ie])!=nullptr) && (e->face == nullptr))
						{
							nEdges ++;

							const HE_edge* s = e;
							const HE_edge* ps = getPrevEdge(s);
							const HE_edge* psp = getPrevEdge(s->pair_edge);
					
							assert(s->end_vertex);
							assert(ps->next_edge == s);
							assert(getPrevEdge(s->next_edge) == s);
							assert(e->end_vertex->edge);
							assert(_vertices[(size_t)e->end_vertex->index]);
							assert(s->pair_edge->face);
							assert(!s->face);
							assert(s->next_edge->next_edge->end_vertex != s->end_vertex);
							assert(ps->end_vertex == s->pair_edge->end_vertex);
							assert(psp->end_vertex == s->end_vertex);

							s = s->pair_edge->next_edge;
							while(s!=e)
							{
								assert(s->end_vertex != e->end_vertex);
								s = s->pair_edge->next_edge;
							}

							/* takes too long
							for(size_t i2 = 0; i2<_edges.size();++i2)
								if( _edges[i2] && _edges[i2] != s)
									assert(! ( _edges[i2]->end_vertex == e->end_vertex && _edges[i2]->pair_edge->end_vertex == e->pair_edge->end_vertex) );
							*/
						}

			assert(nEdges == GetNumEdges()*2);

			//chec edge loops
			for(size_t i = 0; i<_edges.size();++i)
				if(p = _edges[i])
					for(int ie=0;ie<2;ie++)
					{
						e = &p->edges[ie];
						const HE_edge* s = e->pair_edge->end_vertex->edge;
						bool bFound = false;
						do
						{
							if(s==e)
								bFound=true;
							s = s->pair_edge->next_edge;
						}
						while(s!=e->pair_edge->end_vertex->edge);
						assert(bFound);
					}

			//check vertices
			const HE_vert* v = nullptr;
			for(size_t i = 0; i<_vertices.size();++i)
				if((v = _vertices[i]) && v->edge)
				{
					e = v->edge;
					do
					{
						assert(e->pair_edge->end_vertex == v);
						e = e->pair_edge->next_edge;
					}
					while(e != v->edge);
				}
		}

		Vertex SplitEdge(const Edge& edge)
		{
			if(!isValid(edge))
				return nullVertex;

			HE_edge* e = getInternal(edge);

			HE_edge* pair_prev = getPrevEdge(e->pair_edge);

			//first naively split the edge creating 2 4-gons

			HE_edge* v = internalAllocVert();
			HE_edge* e2 = internalAllocEdge();

			e2->next_edge = e->next_edge;
			e2->end_vertex = e->end_vertex;

			e2->pair_edge->next_edge = e->pair_edge;
			e2->pair_edge->end_vertex = v;

			e->next_edge = e2;
			pair_prev->next_edge = e2->pair_edge;

			e->end_vertex = v;

			v->edge = e2;

			//check if we have faces to split

			if(e->face)
			{
				HE_edge* e_side = internalAllocEdge();
				HE_face* f_side = internalAllocFace();

				f_side->edge = e2;
				e2->face = f_side;

				e_side->face = e->face;
				e_side->pair_edge->face = f_side;
				e2->next_edge->face = f_side;

				e_side->next_edge = e2->next_edge->next_edge;
				e_side->pair_edge->next_edge = e2;

				e_side->end_vertex = e2->next_edge->end_vertex;
				e_side->pair_edge->end_vertex = e->end_vertex;

				e2->next_edge->next_edge = e_side->pair_edge;
				e->next_edge = e_side;
			}
			else
				e2->face = nullptr;

			if(e->pair_edge->face)
			{
				HE_edge* e_side = internalAllocEdge();
				HE_face* f_side = internalAllocFace();

				f_side->edge = e2->pair_edge;
				e2->pair_edge->face = f_side;

				e_side->face = f_side;
				e_side->pair_edge->face = e->pair_edge->face;
				e->pair_edge->next_edge->next_edge->face = f_side;

				e_side->next_edge = e->pair_edge->next_edge->next_edge;
				e_side->pair_edge->next_edge = e->pair_edge;

				e_side->end_vertex = e->pair_edge->next_edge->end_vertex;
				e_side->pair_edge->end_vertex = e->end_vertex;

				e->pair_edge->next_edge = e_side->pair_edge;
				e2->pair_edge->next_edge = e_side;
			}
			else
				e2->pair_edge->face = nullptr;

			return getExternal(v);
		}

		Face InsertFace(const VertexTriple& vertices)
		{
			// find if we're re-using any old edges
			HE_vert* v[PolySize];
			HE_edge* e[PolySize];
			_edgeAdjectencyData adj_data[PolySize];
			memset(adj_data,0,sizeof(_edgeAdjectencyData)*PolySize);
			
			for(int i = 0 ; i < PolySize; ++i)
				v[i] = getInternal(vertices[i]);

			for(int i = 0; i<PolySize; ++i)
				e[i] = getEdge(v[i],v[(i+1)%PolySize]);

			for(int i = 0; i<PolySize; ++i)
			{
				if(e[i])
				{
					if(e[(i+1)%PolySize])
					{
						if(e[i]->next_edge != e[(i+1)%PolySize] && !twistEdgeConnect(e[i],e[(i+1)%PolySize]))
							return false;
						adj_data[i].v1_next = e[(i+1)%PolySize];
						adj_data[i].v1_prev = getPrevEdge(e[i]->pair_edge);
						adj_data[(i+1)%PolySize].v0_next = e[(i+1)%PolySize]->pair_edge->next_edge;
					}
					else
					{
						adj_data[i].v1_next = nullptr;
						adj_data[i].v1_prev = getPrevEdge(e[i]->pair_edge);
						adj_data[(i+1)%PolySize].v0_next = e[i]->next_edge;
					}
					
					adj_data[(i+1)%PolySize].v0_prev = e[i];
					adj_data[i].edge = e[i];
				}
				else
				{
					if(e[(i+1)%PolySize])
					{
						adj_data[i].v1_next = e[(i+1)%PolySize];
						adj_data[i].v1_prev = getPrevEdge(e[(i+1)%PolySize]);
						adj_data[(i+1)%PolySize].v0_next = e[(i+1)%PolySize]->pair_edge->next_edge;
						adj_data[(i+1)%PolySize].v0_prev =  nullptr;
					}
					else
					{
						if(v[(i+1)%PolySize]->edge)
						{
							adj_data[i].v1_prev = getNextFreeInner(v[(i+1)%PolySize]->edge->pair_edge);
							if(adj_data[i].v1_prev== nullptr)
								return false;
							adj_data[i].v1_next = nullptr;
							adj_data[(i+1)%PolySize].v0_next = adj_data[i].v1_prev->next_edge;
							adj_data[(i+1)%PolySize].v0_prev = nullptr;
						}
						else
						{	
							adj_data[i].v1_prev = nullptr;
							adj_data[i].v1_next = nullptr;
							adj_data[(i+1)%PolySize].v0_next = nullptr;
							adj_data[(i+1)%PolySize].v0_prev = nullptr;
						}
					}
					
					adj_data[i].edge = nullptr;
				}
			}
			
			//assuming everything is good now, create edges
			
			for(int i = 0; i<PolySize; ++i)
				if(!adj_data[i].edge)
				{
					HE_edge* e_new = internalAllocEdge();
					e[i] = e_new;
					adj_data[i].edge = e_new;

					if(!v[i]->edge)
						v[i]->edge = e[i];

					e[i]->end_vertex = v[(i+1)%PolySize];
					e[i]->face = nullptr;

					e[i]->pair_edge->face = nullptr;
					e[i]->pair_edge->end_vertex = v[i];

					if(!adj_data[(i-1+PolySize)%PolySize].v1_next)
						adj_data[(i-1+PolySize)%PolySize].v1_next = e[i];
					if(!adj_data[(i-1+PolySize)%PolySize].v1_prev)
						adj_data[(i-1+PolySize)%PolySize].v1_prev = e[i]->pair_edge;
					
					if(!adj_data[(i+1)%PolySize].v0_next)
						adj_data[(i+1)%PolySize].v0_next = e[i]->pair_edge;
					if(!adj_data[(i+1)%PolySize].v0_prev)
						adj_data[(i+1)%PolySize].v0_prev = e[i];
				}
			
			for(int i = 0; i<PolySize; ++i)
			{

				adj_data[i].v0_prev->next_edge = e[i];
				e[i]->next_edge = adj_data[i].v1_next;

				adj_data[i].v1_prev->next_edge = e[i]->pair_edge;
				e[i]->pair_edge->next_edge = adj_data[i].v0_next;
			}
			
			
			HE_face* f = internalAllocFace();
			f->edge = e[0];
			for(int i = 0 ; i < PolySize; ++i)
			{
				//assert(e[i]->face == nullptr);
				e[i]->face = f;
				/*
				assert(e[i]->next_edge == e[(i+1)%PolySize]);

				assert(adj_data[i].v0_prev->next_edge == adj_data[i].edge);
				assert(adj_data[i].edge->next_edge == adj_data[i].v1_next);
				assert(adj_data[i].v1_prev->next_edge == adj_data[i].edge->pair_edge);
				assert(adj_data[i].edge->pair_edge->next_edge == adj_data[i].v0_next);*/
			}
			
			return getExternal(f);
		}

		Vertex InsertVertex()
		{
			HE_vert* vertex = internalAllocVert();
			vertex->edge = nullptr;
			return getExternal(vertex);
		}

		void Compact()
		{
			IndexType w=0;

			//compact edges
			for(size_t r = 0 ; r<_edges.size();++r)
				if(_edges[(size_t)r])
				{
					_edges[(size_t)w] = _edges[r];
					_edges[(size_t)w]->index = (IndexType)w;
					++w;
				}
			_edges.resize((size_t)w);
			_unusedEdges = 0;

			//compact faces
			w=0;
			for(size_t r = 0 ; r<_faces.size();++r)
				if(_faces[(size_t)r])
				{
					_faces[(size_t)w] = _faces[r];
					_faces[(size_t)w]->index = (IndexType)w;
					++w;
				}
			_faces.resize((size_t)w);
			_unusedFaces = 0;

			//compact vertices
			w=0;
			for(size_t r = 0 ; r<_vertices.size();++r)
				if(_vertices[r])
				{
					_vertices[(size_t)w] = _vertices[r];
					_vertices[(size_t)w]->index = (IndexType)w;
					++w;
				}
			_vertices.resize((size_t)w);
			_unusedVertices = 0;
		}

		template<RAW_INDEX_FORMAT _format,class _index_type> void ImportRawIndexData(const size_t& nPrimitives,const _index_type* pData)
		{
			IndexType newVerticesBegin = GetNumVertexIndices();
			IndexType newFacesBegin = GetNumFaceIndices();
			IndexType newEdgesBegin = GetNumEdgeIndices();

			//figure out how many vertices we'll need
			IndexType nNewVertices = 0;
			typedef rawFormatReader<_format,_index_type> reader_type;
			reader_type reader;
			for(size_t i = 0; i<nPrimitives; i++)
			{
				reader_type::Primitive result = reader(i,pData);
				for(auto e = result.begin();e!=result.end();++e)
					if(*e >= nNewVertices)
						nNewVertices = *e+1;
			}

			// we're gonna allocate alot
			_vertices.reserve(newVerticesBegin + nNewVertices);
			_faces.reserve(newFacesBegin + nPrimitives);
			_edges.reserve(newEdgesBegin + 2*nPrimitives); // just a guess, no way to know this

			for(int i = 0 ; i <nNewVertices;++i)
				internalAllocVert()->edge = nullptr;

			//vertices should be there now, create the faces
			for(int i = 0 ; i <nPrimitives; ++i)
			{
				reader_type::Primitive result = reader(i,pData);
				Face f = InsertFace(MakeTriple(
					GetVertex(result[0] + newVerticesBegin),
					GetVertex(result[1] + newVerticesBegin),
					GetVertex(result[2] + newVerticesBegin)
					));
				// something went wrong
				assert(f!=nullFace);
			}
		}

	private:


		template<int _format,class _index_type> class rawFormatReader {
		public:
			typedef std::array<_index_type,3> Primitive;
			inline Primitive operator() (const size_t& ith,const _index_type* pData)
			{
				static_assert(0,"Unknown Index Format");
			}
		};

		template<class _index_type> class rawFormatReader<0,_index_type> {
		public:
			typedef std::array<_index_type,3> Primitive;
			inline Primitive operator() (const size_t& ith,const _index_type* pData)
			{
				Primitive res = {{pData[ith*3+0],pData[ith*3+1],pData[ith*3+2]}};
				return res;
			}
		};
		template<class _index_type> class rawFormatReader<1,_index_type> {
		public:
			typedef std::array<_index_type,3> Primitive;
			inline Primitive operator() (const size_t& ith,const _index_type* pData)
			{
				static_assert(0,"Not yet Implemented");
			}
		};
		template<class _index_type> class rawFormatReader<2,_index_type> {
		public:
			typedef std::array<_index_type,3> Primitive;
			inline Primitive operator() (const size_t& ith,const _index_type* pData)
			{
				static_assert(0,"Not yet Implemented");
			}
		};

		template<class _contents> class voidContainer
		{
		public:
			_contents contents;
		};

		template<> class voidContainer<void>
		{
		};

		struct _HE_edge;
		struct _HE_vert;
		struct _HE_face;

		typedef struct _HE_edge
		{
			_HE_vert*	end_vertex;	//end vertex (null for invalidated)
			_HE_edge*	pair_edge;	//the paired HALF edge
			_HE_face*	face;		//bordering face (null for border)
			_HE_edge*	next_edge;	//next HALF edge around the face if inside, on the circumference if outside
		
		} HE_edge;

		typedef struct _HE_edge_pair
		{
			HE_edge		edges[2];
			IndexType	index;
			voidContainer<EdgeType> data;
		} HE_edge_pair;
	
		typedef struct _HE_vert
		{
			_HE_edge*	edge;

			IndexType	index;

			voidContainer<VertexType> data;
		} HE_vert;
	
		typedef struct _HE_face
		{
			_HE_edge* edge;

			IndexType	index;

			voidContainer<FaceType> data;
		} HE_face;

		// private members

		std::vector<HE_edge_pair*> _edges;
		IndexType	_unusedEdges;
		std::vector<HE_vert*> _vertices;
		IndexType	_unusedVertices;
		std::vector<HE_face*> _faces;
		IndexType	_unusedFaces;


		//private functions

		struct _edgeAdjectencyData
		{
			HE_edge* v0_prev;
			HE_edge* v0_next;
			HE_edge* v1_prev;
			HE_edge* v1_next;
			HE_edge* edge;
		};
		
		HE_edge* getNextFreeInner(HE_edge* e)
		{
			HE_edge* s = e;

			do
			{
				s = s->next_edge->pair_edge;
				if(s->face == nullptr)
					return s;
			}
			while(s!=e);

			return nullptr;

		}

		bool findMergeEdgesEx_withEdge(HE_vert* v0,HE_vert* v1,_edgeAdjectencyData& data_out)
		{
			//we can set all the data out, we're basically just checking for legality
			data_out.v0_prev = getPrevEdge(data_out.edge);
			data_out.v0_next = data_out.edge->pair_edge->next_edge;
				
			data_out.v1_prev = getPrevEdge(data_out.edge->pair_edge);
			data_out.v1_next = data_out.edge->next_edge;

			HE_edge* removed_v0_prev[2] = {nullptr,nullptr};
			HE_edge* removed_v1_prev[2] = {nullptr,nullptr};

			//might point to edges that will be removed so fix them
			
			if(data_out.v0_next->pair_edge == data_out.v0_prev)
			{
				data_out.v0_next = nullptr;
				data_out.v0_prev = nullptr;
			}
			else
			{
				if(!data_out.v0_next->pair_edge->face)
				{
					if(data_out.v0_next->pair_edge->next_edge->pair_edge == data_out.v0_prev)
					{
						data_out.v0_next = nullptr;
						data_out.v0_prev = nullptr;
					}
					else
					{
						removed_v0_prev[0] = data_out.v0_next->pair_edge;
						data_out.v0_next = data_out.v0_next->pair_edge->next_edge;
					}
				}

				if(data_out.v0_prev && !data_out.v0_prev->pair_edge->face)
				{
					removed_v0_prev[1] = data_out.v0_prev;
					data_out.v0_prev = getPrevEdge(data_out.v0_prev->pair_edge);
				}
			}

			
			if(data_out.v1_next->pair_edge == data_out.v1_prev)
			{
				data_out.v1_next = nullptr;
				data_out.v1_prev = nullptr;
			}
			else
			{
				if(!data_out.v1_next->pair_edge->face)
				{
					if(data_out.v1_next->pair_edge->next_edge->pair_edge == data_out.v1_prev)
					{
						data_out.v1_next = nullptr;
						data_out.v1_prev = nullptr;
					}
					else
					{
						removed_v1_prev[0] = data_out.v1_next->pair_edge;
						data_out.v1_next = data_out.v1_next->pair_edge->next_edge;
					}
				}

				if(data_out.v1_prev && !data_out.v1_prev->pair_edge->face)
				{
					removed_v1_prev[1] = data_out.v1_prev;
					data_out.v1_prev = getPrevEdge(data_out.v1_prev->pair_edge);
				}
			}

			if(!data_out.v0_prev || !data_out.v1_prev)
				return true;//nothing to check

			// loop over all combinations of v0/v1 prevs

			HE_edge* v0_prev = v0->edge->pair_edge;
			
			do
			{
				
				if(	v0_prev->pair_edge == data_out.edge )
					continue;//ignore connecting edge
				if(v0_prev == removed_v0_prev[0])
					continue;//this edge will be removed if connecting edge disappears
				if(v0_prev == removed_v0_prev[1])
					continue;//this edge will be removed if connecting edge disappears

				
				HE_edge* v1_prev = v1->edge->pair_edge;
				do
				{
				
					if(v1_prev == data_out.edge)
						continue;//ignore connecting edge
					if(v1_prev == removed_v1_prev[0])
						continue;//this edge will be removed if connecting edge disappears
					if(v1_prev == removed_v1_prev[1])
						continue;//this edge will be removed if connecting edge disappears

					if(v1_prev->pair_edge->end_vertex == v0_prev->pair_edge->end_vertex)
					{
						//since we skip removed edges just check if these match

						if(
							(v0_prev == data_out.v0_prev && v1_prev->pair_edge == data_out.v1_next)
							||
							(v0_prev->pair_edge == data_out.v0_next && v1_prev == data_out.v1_prev)
							)
							continue;


						return false;
					}
					
				}
				while( (v1_prev = v1_prev->next_edge->pair_edge) != v1->edge->pair_edge);
				
			}
			while( (v0_prev = v0_prev->next_edge->pair_edge) != v0->edge->pair_edge);

			return true;
		}

		bool findMergeEdgesEx_withoutEdge(HE_vert* v0,HE_vert* v1,_edgeAdjectencyData& data_out)
		{
			
			bool b12Fixed = false;
			bool b21Fixed = false;
			
			data_out.edge = nullptr;
			data_out.v0_prev = nullptr;
			data_out.v0_next = nullptr;
			data_out.v1_prev = nullptr;
			data_out.v1_next = nullptr;

			// loop over all combinations of v0/v1 prevs

			HE_edge* v0_prev = v0->edge->pair_edge;
			
			do
			{
				v0_prev = v0_prev->next_edge->pair_edge;
				
				HE_edge* v1_prev = v1->edge->pair_edge;
				do
				{
					v1_prev = v1_prev->next_edge->pair_edge;

					if(v1_prev->pair_edge->end_vertex == v0_prev->pair_edge->end_vertex)
					{
						//connecting pair
						if(!v0_prev->face)
						{
							if(v1_prev->pair_edge->face)
								return false; //misaligned face sides

							if(b21Fixed)
								return false;//too many connections
							b21Fixed = true;

							data_out.v0_prev = v0_prev;
							data_out.v1_next = v1_prev->pair_edge;
							
							if(! b12Fixed)
							{
								data_out.v0_next = data_out.v0_prev->next_edge;
								data_out.v1_prev = getPrevEdge(data_out.v1_next);
							}
						}
						else if(!v1_prev->face)
						{
							if(v0_prev->pair_edge->face)
								return false; //misaligned face sides

							if(b12Fixed)
								return false;//too many connections

							b12Fixed = true;

							data_out.v1_prev = v1_prev;
							data_out.v0_next = v0_prev->pair_edge;
							
							if(! b21Fixed)
							{
								data_out.v1_next = data_out.v1_prev->next_edge;
								data_out.v0_prev = getPrevEdge(data_out.v0_next);
							}
						}
						else
							return false; //connection of two  edges with faces? fail
					}

					if(!data_out.v1_prev && !v1_prev->face)
					{
						//try to find open edges if we have no connection at all yet
						
						data_out.v1_prev = v1_prev;
						data_out.v1_next = v1_prev->next_edge;
					}

				}
				while(v1_prev != v1->edge->pair_edge);
				

				if(!data_out.v0_prev && !v0_prev->face)
				{
					//try to find open edges if we have no connection at all yet
						
					data_out.v0_prev = v0_prev;
					data_out.v0_next = v0_prev->next_edge;
				}
			}
			while(v0_prev != v0->edge->pair_edge);

			if(!data_out.v0_prev || !data_out.v1_prev)
				return false;

			return true;
		}

		bool findMergeEdgesEx(HE_vert* v0,HE_vert* v1,_edgeAdjectencyData& data_out)
		{
			if(!v0->edge || !v1->edge)
			{
				//trivial case
				data_out.edge = nullptr;
				data_out.v0_prev = nullptr;
				data_out.v0_next = nullptr;
				data_out.v1_prev = nullptr;
				data_out.v1_next = nullptr;
				return true;
			}

			//trivial cases done

			data_out.edge = getEdge(v0,v1);

			if(data_out.edge)
				return findMergeEdgesEx_withEdge(v0,v1,data_out);
			else
				return findMergeEdgesEx_withoutEdge(v0,v1,data_out);
		}

		bool findMergeEdges(HE_edge*& v1_prev,HE_vert* v1,HE_edge*& v1_next,HE_edge*& v2_prev,HE_vert* v2,HE_edge*& v2_next)
		{
			//bool bExclusiveBorder = false;


			if(v2->edge && v1->edge)
			{
				// 2 edges
				bool b12Fixed = false;
				bool b21Fixed = false;
				bool bOpenL1 = false;
				bool bOpenL2 = false;

				//find open border on v2
				HE_edge* cur = v2->edge;
				do
				{
					if(!cur->pair_edge->face)
						bOpenL1 = true;
					else
						bOpenL1 = false;

					HE_edge* prop_v2_prev = cur->pair_edge;
					HE_edge* prop_v2_next = bOpenL1?prop_v2_prev->next_edge:nullptr;

					//find open edges at v1
					HE_edge* cur_v1 = v1->edge;
					do
					{
						if(!cur_v1->pair_edge->face)
							bOpenL2 = true;
						else
							bOpenL2 = false;

						HE_edge* prop_v1_prev = cur_v1->pair_edge;
						HE_edge* prop_v1_next = bOpenL2?prop_v1_prev->next_edge:nullptr;

						//check if they share vertex, if so they must be chosen
						if(bOpenL1 && bOpenL2)
						{
							if(prop_v1_prev->pair_edge->end_vertex == prop_v2_next->end_vertex)
							{
								if(b21Fixed)
									return false;

								b21Fixed = true;
								v2_next = prop_v2_next;
								v1_prev = prop_v1_prev;

								if(!b12Fixed)
								{
									v1_next = prop_v1_next;
									v2_prev = prop_v2_prev;				
								}

							}
							if(prop_v2_prev->pair_edge->end_vertex == prop_v1_next->end_vertex)
							{
								if(b12Fixed)
									return false;

								b12Fixed = true;
								v1_next = prop_v1_next;
								v2_prev = prop_v2_prev;

								if(!b21Fixed)
								{
									v2_next = prop_v2_next;
									v1_prev = prop_v1_prev;
								}
							}

							if(!b12Fixed && !v1_next)
								v1_next = prop_v1_next;
							if(!b21Fixed && !v1_prev)
								v1_prev = prop_v1_prev;
						}
						else if(!bOpenL1 && !bOpenL2 && prop_v1_prev->pair_edge->end_vertex == prop_v2_prev->pair_edge->end_vertex)
							return false;//if a vertex is shared among non open edges it would degenerate

						cur_v1 = cur_v1->pair_edge->next_edge;
					}
					while(cur_v1 != v1->edge);
							
					if( bOpenL1 && !b21Fixed && !v2_next)
						v2_next = prop_v2_next;
					if( bOpenL1 && !b12Fixed && !v2_prev)
						v2_prev = prop_v2_prev;

					if(bOpenL1)
					{
						assert(v1_prev);
						assert(v1_next);
						assert(v2_prev);
						assert(v2_next);
					}


					cur = cur->pair_edge->next_edge;
				}
				while(cur != v2->edge);

				if(!v2_next || !v1_next)
					return false; // full

				if(!(b21Fixed&&b12Fixed))
				{
					assert(v2_prev->next_edge == v2_next);
					assert(v1_prev->next_edge == v1_next);
				}

			}
			else if(v2->edge)
			{
				//1 edges, find first and take it
				HE_edge* cur = v2->edge;
				do
				{
					if(!cur->pair_edge->face)
					{
						v1_prev = nullptr;
						v1_next = nullptr;
						v2_prev = cur->pair_edge;
						v2_next = v2_prev->next_edge;
						return true;
					}
					cur = cur->pair_edge->next_edge;
				}
				while(cur != v2->edge);
			}
			else if(v1->edge)
			{
				//1 edges, find first and take it
				HE_edge* cur = v1->edge;
				do
				{
					if(!cur->pair_edge->face)
					{
						v1_prev = cur->pair_edge;
						v1_next = v1_prev->next_edge;
						v2_prev = nullptr;
						v2_next = nullptr;
						return true;
					}
					cur = cur->pair_edge->next_edge;
				}
				while(cur != v1->edge);
			}
			else
			{
				// no edges
				v1_prev = nullptr;
				v1_next = nullptr;
				v2_prev = nullptr;
				v2_next = nullptr;
				return true;
			}

			return true;
		}

		bool twistEdgeConnect(HE_edge* i,HE_edge* o)
		{
			HE_edge* freeInner = getNextFreeInner(o->pair_edge);
			HE_edge* p = getPrevEdge(o);

			if(freeInner == i)
				return false; // not connectable

					
			assert(freeInner->end_vertex == i->end_vertex);
			assert(p->end_vertex == i->end_vertex);
			assert(freeInner->face == nullptr);
			assert(p->face == nullptr);

			HE_edge* buffer =freeInner->next_edge;

			freeInner->next_edge = i->next_edge;
			i->next_edge = o;
			p->next_edge = buffer;

			return true;
		}

		void printInnerLoop(HE_edge* e)
		{
			HE_edge* s = e;
			std::cerr << "printing inner loop for" << e->index <<"\n";
			do
			{
				std::cerr << "\t" <<s->index << "("<<s->pair_edge->end_vertex->index <<"-"<<s->end_vertex->index <<")\n";
				s = s->pair_edge->next_edge;
			}
			while(s!=e);
		}

		HE_edge* loopFind(HE_edge* e,HE_vert* v,HE_vert* v_stop = nullptr)
		{
			HE_edge* cur = e;
			do
			{
				if(cur->end_vertex == v)
					return cur;
				if(v_stop && cur->end_vertex == v_stop)
					return nullptr;
				cur=cur->next_edge;
			}
			while(e!=cur);
			return nullptr;
		}

		void mergeDoubleEdges(HE_edge* e1,HE_edge* e2)
		{
			assert(e1->next_edge == e2);
			{
				assert(e1->face == nullptr);
				assert(e2->face == nullptr);
				assert(e1->pair_edge->face);
				assert(e2->pair_edge->face);
				
				//will remove e2 and e2->pair
				HE_edge* e2_prev_pair = getPrevEdge(e2->pair_edge);

				e2_prev_pair->next_edge = e1;

				e1->next_edge = e2->pair_edge->next_edge;
				e1->end_vertex = e2->pair_edge->end_vertex;
				e1->face = e2->pair_edge->face;
				

				if(e2->pair_edge->face->edge == e2->pair_edge)
					e2->pair_edge->face->edge = e1;

				if(e2->end_vertex->edge == e2->pair_edge)
					e2->end_vertex->edge = e1;
				if(e2->pair_edge->end_vertex->edge == e2)
					e2->pair_edge->end_vertex->edge = e1->pair_edge;
						
				//std::cerr << "\te" << e1->index << " + e" << e2->index <<" = e"<<e1->index<<"\n";
				

				internalRemove(e2);
			}
		}
		bool checkDegenerateEdge(HE_vert* v0,HE_vert* v1,_edgeAdjectencyData& data)
		{		
			if(v0->edge)
			{
				HE_edge* prev = nullptr;
				HE_edge* cur =  v0->edge->pair_edge;
				HE_edge* match = nullptr;
				do
				{
					if(!cur->face && !prev && ((data.v0_prev)?(data.v0_prev == cur):true))
						prev=cur;
					if( cur->pair_edge->end_vertex == v1)
						match = cur->pair_edge;
					cur = cur->next_edge->pair_edge;
				}
				while(cur->pair_edge != v0->edge);
				if(!prev)
					return false; //vertex is full
				if(match)
				{
					if(!match->face)
					{
						data.edge =  match;
						return true; // edge already exists
					}
					else
						return false;
				}
				if(data.v0_prev)
					assert(data.v0_prev == prev);

				data.v0_prev = prev;
				data.v0_next = prev->next_edge;
			}
			if(v1->edge && !data.v1_prev)
			{
				HE_edge* prev = v1->edge->pair_edge;
				while(prev->face)
				{
					prev = prev->next_edge->pair_edge;
					if(prev->pair_edge == v1->edge)
						return false; //vertex is full
				}

				data.v1_prev = prev;
				data.v1_next = prev->next_edge;
			}

			return true;
		}

		HE_edge* getEdge(HE_vert* v1,HE_vert* v2)
		{			
			HE_edge* candidate = v1->edge;

			if(!candidate)
				return nullptr;

			//search ccw
			do
			{
				if(candidate -> end_vertex == v2)
					return candidate;

				candidate = candidate->pair_edge->next_edge;
			}
			while(candidate != v1->edge);
			//we rotated a full circle
			return nullptr;
		}

		HE_edge* insertDegenerateEdge(HE_vert* v0,HE_vert* v1,const _edgeAdjectencyData& data)
		{
			if(data.edge)
			{
				return data.edge;
				}

			HE_edge* edge = internalAllocEdge();

			//make it looping
			edge->next_edge = data.v1_next?data.v1_next:edge->pair_edge;
			edge->end_vertex = v1;
			edge->face = nullptr;

			edge->pair_edge->next_edge = data.v0_next?data.v0_next:edge;
			edge->pair_edge->end_vertex = v0;
			edge->pair_edge->face = nullptr;

			if(data.v0_prev)
				data.v0_prev->next_edge = edge;

			if(data.v1_prev)
				data.v1_prev->next_edge = edge->pair_edge;

			if(v0->edge == nullptr)
				v0->edge = edge;
			if(v1->edge == nullptr)
				v1->edge = edge->pair_edge;

			return edge;
		}

		bool removeDegenerateEdge(HE_edge* e)
		{
			if(e->face != nullptr || e->pair_edge->face != nullptr)
				return false;

			//find source of e
			
			//find source of e-pair

			HE_edge* pe = getPrevEdge(e);
			HE_edge* pep = getPrevEdge(e->pair_edge);

			pe->next_edge = e->pair_edge->next_edge;
			pep->next_edge = e->next_edge;

			if(e->end_vertex->edge == e->pair_edge)
			{
				//fix e-end_vertex
				if(e->next_edge == e->pair_edge)
					e->end_vertex->edge = nullptr;//internalRemove(e->end_vertex); // can't fix, remove orphaned vertex
				else
					e->end_vertex->edge = e->next_edge;

				assert(e->end_vertex->edge != e);
				assert(e->end_vertex->edge != e->pair_edge);
			}
			if(e->pair_edge->end_vertex->edge == e)
			{
				//fix pair-end_vertex
				if(e->pair_edge->next_edge == e)
					e->pair_edge->end_vertex->edge = nullptr;//internalRemove(e->pair_edge->end_vertex); // can't fix, remove orphaned vertex
				else
					e->pair_edge->end_vertex->edge = e->pair_edge->next_edge;

				
				assert(e->pair_edge->end_vertex->edge != e);
				assert(e->pair_edge->end_vertex->edge != e->pair_edge);
			}
					
			internalRemove(e);
			return true;
		}

		inline static HE_edge_pair* getPair(HE_edge* e)
		{
			if( e->pair_edge < e)
				return reinterpret_cast<HE_edge_pair*>(e->pair_edge);
			else
				return reinterpret_cast<HE_edge_pair*>(e);
		}
		inline static const HE_edge_pair* getPair(const HE_edge* e)
		{
			if( e->pair_edge < e)
				return reinterpret_cast<const HE_edge_pair*>(e->pair_edge);
			else
				return reinterpret_cast<const HE_edge_pair*>(e);
		}

		inline void internalRemove(HE_edge_pair* e)
		{
			_unusedEdges++;
			_edges[(size_t)e->index] = nullptr;
 			delete e;
		}
		inline void internalRemove(HE_edge* e)
		{
			internalRemove(getPair(e));
		}
		inline void internalRemove(HE_face* f)
		{
			_unusedFaces++;
			_faces[(size_t)f->index] = nullptr;
 			delete f;
		}
		inline void internalRemove(HE_vert* v)
		{
			_unusedVertices++;
			_vertices[(size_t)v->index] = nullptr;
 			delete v;
		}

		inline HE_edge* internalAllocEdge()
		{
			HE_edge_pair* e= new HE_edge_pair;
			e->edges[0].pair_edge = &e->edges[1];
			e->edges[1].pair_edge = &e->edges[0];
			e->index = _edges.size();
			_edges.push_back(e);
			return &e->edges[0];
		}

		inline HE_face* internalAllocFace()
		{
			HE_face* f = new HE_face;
			f->index = _faces.size();			
			_faces.push_back(f);
			return f;
		}
		inline HE_vert* internalAllocVert()
		{
			HE_vert* v = new HE_vert;
			v->index = _vertices.size();			
			_vertices.push_back(v);
			return v;
		}

		inline const HE_edge* getInternal(const Edge& e) const
		{
			return (const HE_edge*)&e.internal_data->edges[0];
		}
		inline const HE_vert* getInternal(const Vertex& v) const
		{
			return (const HE_vert*)v.internal_data;
		}
		inline const HE_face* getInternal(const Face& f) const
		{
			return (const HE_face*)f.internal_data;
		}

		inline HE_edge* getInternal(const Edge& e)
		{
			return ((HE_edge*)&e.internal_data->edges[0]);
		}
		inline HE_vert* getInternal(const Vertex& v)
		{
			return ((HE_vert*)v.internal_data);
		}
		inline HE_face* getInternal(const Face& f)
		{
			return ((HE_face*)f.internal_data);
		}

		inline Edge getExternal(const HE_edge_pair* e) const
		{
			return Edge(const_cast<HE_edge_pair*>(e));
		}
		inline Edge getExternal(const HE_edge* e) const
		{
			return Edge(getPair(const_cast<HE_edge*>(e)));
		}
		inline Vertex getExternal(const HE_vert* v) const
		{
			return Vertex(const_cast<HE_vert*>(v));
		}
		inline Face getExternal(const HE_face* f) const
		{
			return Face(const_cast<HE_face*>(f));
		}

		inline static const HE_edge* getPrevEdge(const HE_edge* e)
		{
			if(e->face)
			{
				const HE_edge* res = e;
				for(int i = 0; i<PolySize-1;++i)
					res = res->next_edge;
				assert(res->next_edge == e);
				return res;
			}
			else
			{
				HE_edge* res = e->pair_edge;
				while(res->next_edge != e)
					res = res->next_edge->pair_edge;
				return res;
			}
		}
		inline static HE_edge* getPrevEdge(HE_edge* e)
		{
			if(e->face)
			{
				HE_edge* res = e;
				for(int i = 0; i<PolySize-1;++i)
					res = res->next_edge;
				assert(res->next_edge == e);
				return res;
			}
			else
			{
				HE_edge* res = e->pair_edge;
				while(res->next_edge != e)
					res = res->next_edge->pair_edge;
				return res;
			}
		}
		
		// for an item to be valid it must be:
		//  not null
		//  not invalidated
		//  inside our vectors
		//  point to the beginning of an element

#ifndef EDITABLE_MESH_SKIP_VALIDATION
		inline bool isValid(const HE_face* f) const
		{
			return 
				(f!=nullptr) && 
				(f->edge!=nullptr) && 
				(f->index < _faces.size()) && 
				(f == _faces[(size_t)f->index]);
		}
		inline bool isValid(const HE_edge* h) const
		{
			const HE_edge_pair* p = h?getPair(h):nullptr;
			return 
				(p!=nullptr) && 
				(h->end_vertex!=nullptr) && 
				(p->index < _edges.size()) && 
				(p == _edges[(size_t)p->index]);
		}
		inline bool isValid(const HE_vert* v,bool ignore_invalidated = false) const
		{
			return 
				(v!=nullptr) && 
				(v->edge!=nullptr || ignore_invalidated) && 
				(v->index < _vertices.size()) && 
				(v == _vertices[(size_t)v->index]);
		}
#else // SKIP VALIDATION
		inline static bool isValid(const HE_face* f)
		{
			return true;
		}
		inline static bool isValid(const HE_edge* e)
		{
			return true;
		}
		inline static bool isValid(const HE_vert* v)
		{
			return true;
		}
#endif


	friend class _EditableMeshInternal::_CEditableMesh_GetFaceData< _FaceType, _EditableMeshInternal::_CEditableMeshBase<_VertexType,_FaceType,_EdgeType, _Final > >;
	friend class _EditableMeshInternal::_CEditableMesh_GetEdgeData< _EdgeType, _EditableMeshInternal::_CEditableMeshBase<_VertexType,_FaceType,_EdgeType, _Final > >;
	friend class _EditableMeshInternal::_CEditableMesh_GetVertexData< _VertexType, _EditableMeshInternal::_CEditableMeshBase<_VertexType,_FaceType,_EdgeType, _Final > >;

	};

	template<class _1, class _2, class _3, class _4> const typename _CEditableMeshBase<_1,_2,_3,_4>::Edge _CEditableMeshBase<_1,_2,_3,_4>::nullEdge = typename _CEditableMeshBase<_1,_2,_3,_4>::Edge(nullptr);
	template<class _1, class _2, class _3, class _4> const typename _CEditableMeshBase<_1,_2,_3,_4>::Vertex _CEditableMeshBase<_1,_2,_3,_4>::nullVertex = typename _CEditableMeshBase<_1,_2,_3,_4>::Vertex(nullptr);
	template<class _1, class _2, class _3, class _4> const typename _CEditableMeshBase<_1,_2,_3,_4>::Face _CEditableMeshBase<_1,_2,_3,_4>::nullFace = typename _CEditableMeshBase<_1,_2,_3,_4>::Face(nullptr);

}

template<class _VertexType = Vector3<>, class _FaceType = void, class _EdgeType = void> class CEditableMesh : 
	public _EditableMeshInternal::_CEditableMesh_GetFaceData< _FaceType, _EditableMeshInternal::_CEditableMeshBase<_VertexType,_FaceType,_EdgeType, CEditableMesh<_VertexType,_FaceType,_EdgeType> > >,
	public _EditableMeshInternal::_CEditableMesh_GetEdgeData< _EdgeType, _EditableMeshInternal::_CEditableMeshBase<_VertexType,_FaceType,_EdgeType, CEditableMesh<_VertexType,_FaceType,_EdgeType> > >,
	public _EditableMeshInternal::_CEditableMesh_GetVertexData< _VertexType, _EditableMeshInternal::_CEditableMeshBase<_VertexType,_FaceType,_EdgeType, CEditableMesh<_VertexType,_FaceType,_EdgeType> > >,
	public _EditableMeshInternal::_CEditableMeshBase<_VertexType,_FaceType,_EdgeType, CEditableMesh<_VertexType,_FaceType,_EdgeType> >
	{
	};


}

#endif