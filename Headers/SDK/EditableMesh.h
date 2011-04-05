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
	private:
		template<class _internal,class _data> class _InternalConvert
		{
		public:
			operator const _data&() const
			{
				return ((_internal*)(this))->internal_data->data.contents;
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
		class _HE_vert;
		class _HE_face;
	public:
		typedef class _Edge : public _InternalConvert<_Edge,EdgeType>
		{ 
		private:
			_HE_edge* internal_data; 
			_Edge(_HE_edge* val) : internal_data(val) {}
		public:
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
			return (IndexType)_faces.size() - _facesFree.size();
		}
		IndexType GetNumEdges() const
		{
			return (IndexType)(_edges.size() - _edgesFree.size())/2;
		}
		IndexType GetNumVertices() const
		{
			return (IndexType)_vertices.size() - _verticesFree.size();
		}

		IndexType GetNumFaceIndices() const
		{
			return (IndexType)_faces.size();
		}
		IndexType GetNumEdgeIndices() const
		{
			return (IndexType)_edges.size()/2;
		}
		IndexType GetNumVertexIndices() const
		{
			return (IndexType)_vertices.size();
		}

		Vertex GetVertex(IndexType index) const { if(index >= GetNumVertexIndices()) return nullVertex; else return getExternal(_vertices[(size_t)index]);}
		IndexType GetIndex(const Vertex& v) const { return getInternal(v)->index;}
	
		Edge GetEdge(IndexType index) const { if(index >= GetNumEdgeIndices()) return nullEdge; else return getExternal(_edges[(size_t)index*2]);}
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
		Edge GetNextVertexEdge(const Vertex& vertex,const Edge& previous_edge) const
		{
			const HE_vert* v = getInternal(vertex);
			const HE_edge* e = getInternal(previous_edge);
			
			if(!isValid(v))
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
		Face GetNextVertexFace(const Vertex& vertex,const Face& previous_face) const
		{
			const HE_vert* v = getInternal(vertex);
			const HE_face* f = getInternal(previous_face);
			
			if(!isValid(v))
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
				cur = cur->next_edge;
				cur->face = nullptr;
			}

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

		bool MergeVertex(const VertexPair& vpair)
		{
			if(vpair[0] == vpair[1])
				return false;
			bool bEdge = false;
			Edge e = GetEdge(vpair);
			if(e!=nullEdge)
			{
				InvalidateEdge(e);
				bEdge = true;
			}
			
			{
				HE_vert* v1 = getInternal(vpair[0]),* v2 = getInternal(vpair[1]);
				
				HE_edge* v1_next=nullptr,*v1_prev=nullptr;
				HE_edge* v2_next=nullptr,*v2_prev=nullptr;
				
				if(v2->edge && v1->edge)
				{
					//find open border on v2
					HE_edge* cur = v2->edge;
					do
					{
						if(!cur->pair_edge->face)
						{
							v2_prev = cur->pair_edge;
							v2_next = v2_prev->next_edge;
							break;
						}
						cur = cur->pair_edge->next_edge;
					}
					while(cur != v2->edge);

					if(!v2_next)
					{
						if(bEdge)
							assert(!"Vertex 2 Full");
						else return false;
					}
				}

				if(v1->edge)
				{
					//find open border on v1
					HE_edge* cur = v1->edge;
					do
					{
						if(!cur->pair_edge->face && !v1_prev)
						{
							v1_prev = cur->pair_edge;
							v1_next = v1_prev->next_edge;
						}
						cur = cur->pair_edge->next_edge;
					}
					while(cur != v1->edge);

					if(!v1_next && v2->edge)
					{
						if(bEdge)
							assert(!"Vertex 2 Full");
						else 
							return false;
					}

					//we're now certain so we can start changing stuff
					cur = v1->edge;
					do
					{
						cur->pair_edge->end_vertex = v2;
						cur = cur->pair_edge->next_edge;
					}
					while(cur != v1->edge);


					if(!v2->edge)
					{
						v2->edge = v1->edge;
					}
				}

				//we can get rid of v1 now, no more edges reference it
				internalRemove(v1);

				if(v1_next && v2_next)
				{
					v1_prev->next_edge = v2_next;
					v2_prev->next_edge = v1_next;

					// theoretically we're all linked up now but the edges might be broken
					if(v1_next->next_edge == v2_prev)
					{
						//this part is broken
						//we will remove v1_next and v1_next->pair_edge
						v2_prev->next_edge = v1_next->pair_edge->next_edge;
						v2_prev->end_vertex = v1_next->pair_edge->end_vertex;
						v2_prev->face = v1_next->pair_edge->face;
						getPrevEdge(v1_next->pair_edge)->next_edge = v2_prev;

						if(v2_prev->face->edge == v1_next->pair_edge)
							v2_prev->face->edge = v2_prev;
						if(v1_next->end_vertex->edge == v1_next->pair_edge)
							v1_next->end_vertex->edge = v2_prev;
						if(v1_next->pair_edge->end_vertex->edge == v1_next)
							v1_next->pair_edge->end_vertex->edge = v1_prev->pair_edge;
						internalRemove(v1_next);
						v1_next = nullptr;
					}

					// theoretically we're all linked up now but the edges might be broken
					if(v2_next->next_edge == v1_prev)
					{
						//this part is broken
						//we will remove v1_next and v1_next->pair_edge
						v1_prev->next_edge = v2_next->pair_edge->next_edge;
						v1_prev->end_vertex = v2_next->pair_edge->end_vertex;
						v1_prev->face = v2_next->pair_edge->face;
						getPrevEdge(v2_next->pair_edge)->next_edge = v1_prev;

						if(v1_prev->face->edge == v2_next->pair_edge)
							v1_prev->face->edge = v1_prev;
						if(v2_next->end_vertex->edge == v2_next->pair_edge)
							v2_next->end_vertex->edge = v1_prev;
						if(v2_next->pair_edge->end_vertex->edge == v2_next)
							v2_next->pair_edge->end_vertex->edge = v1_prev->pair_edge;
						internalRemove(v2_next);
						v2_next = nullptr;
					}
				}
			}
			return true;
		}

		void DebugValidateMesh() const 
		{
			const HE_face* f=nullptr;
			const HE_edge* e =nullptr;
			u32 nEdges = 0;
			for(int i = 0; i<_faces.size();++i)
				if(f = _faces[i])
				{
					e = f->edge;
					for(int ie=0;ie<PolySize;++ie)
					{
						e=e->next_edge;
						assert(e->pair_edge->pair_edge ==e);
						assert(e->next_edge->pair_edge->end_vertex == e->end_vertex);
						assert(e->face == f);
						++nEdges;
					}

					assert(e==f->edge);
				}
			//rest of edges must be outer
			e = nullptr;
			for(int i = 0; i<_edges.size();++i)
				if((e = _edges[i]) && e->face == nullptr)
				{
					nEdges ++;
					const HE_edge* s = e;
					do
					{
						assert(s->pair_edge->face);
						assert(!s->face);
					}
					while(s!=e);
				}
			assert(nEdges/2 == GetNumEdges());

			//check vertices
			const HE_vert* v = nullptr;
			for(int i = 0; i<_vertices.size();++i)
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

		Face InsertFace(const VertexTriple& vertices)
		{
			// find if we're re-using any old edges
			HE_vert* v[PolySize];
			HE_edge* e[PolySize];
			
			for(int i = 0 ; i < PolySize; ++i)
				v[i] = getInternal(vertices[i]);
			

			for(int i = 0 ; i < PolySize; ++i)
			{
				_edgeAdjectencyData data = {nullptr,nullptr,nullptr,nullptr,nullptr};
				if(i != 0 )
				{
					data.v0_prev = e[i-1];
					data.v0_next = e[i-1]->next_edge;
				}
				if(i==2)
				{
					data.v1_prev = getPrevEdge(e[0]);
					data.v1_next = e[0];
				}
				if(!checkDegenerateEdge(v[i],v[(i+1)%PolySize],data))
				{
					//clean up
					for(int i2 = i-1; i2 >= 0; --i2)
						removeDegenerateEdge(e[i2]);
					return nullFace;
				}
				e[i] = insertDegenerateEdge(v[i],v[(i+1)%PolySize],data);
			}
			
			
			HE_face* f = internalAllocFace();
			f->edge = e[0];
			for(int i = 0 ; i < PolySize; ++i)
				e[i]->face = f;

	
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
			for(int r = 0 ; r<_edges.size();++r)
				if(_edges[r])
				{
					_edges[w] = _edges[r];
					_edges[w]->index = w;
					++w;
				}
			_edges.resize(w);
			_edgesFree.clear();

			//compact faces
			w=0;
			for(int r = 0 ; r<_faces.size();++r)
				if(_faces[r])
				{
					_faces[w] = _faces[r];
					_faces[w]->index = w;
					++w;
				}
			_faces.resize(w);
			_facesFree.clear();

			//compact vertices
			w=0;
			for(int r = 0 ; r<_vertices.size();++r)
				if(_vertices[r])
				{
					_vertices[w] = _vertices[r];
					_vertices[w]->index = w;
					++w;
				}
			_vertices.resize(w);
			_verticesFree.clear();
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
			_edges.reserve(newEdgesBegin + 4*nPrimitives); // just a guess, no way to know this

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
			
		/*
		template<size_t _index> struct contains_impl
		{
			template<class _tuple,class _element> bool operator()( const _tuple& tuple, const _element& element)
			{	
				if( std::get<_index>(tuple) == element)
					return true;
				contains_impl<_index-1> contains;
				return contains(tuple,element);
			}
		};

		template<> struct contains_impl<0>
		{
			template<class _tuple,class _element> bool operator()( const _tuple& tuple, const _element& element)
			{	
				if( std::get<0>(tuple) == element)
					return true;
				return false;
			}
		};

		template<class _tuple,class _element> bool contains( const _tuple& tuple, const _element& element)
		{
			contains_impl<std::tuple_size<_tuple>::value -1> contains;
			return contains(tuple,element);
		}
		*/

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
		
			IndexType	index;

			voidContainer<EdgeType> data;
		} HE_edge;
	
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

		std::vector<HE_edge*> _edges;
		std::list<IndexType>	_edgesFree;
		std::vector<HE_vert*> _vertices;
		std::list<IndexType>	_verticesFree;
		std::vector<HE_face*> _faces;
		std::list<IndexType>	_facesFree;


		//private functions

		struct _edgeAdjectencyData
		{
			HE_edge* v0_prev;
			HE_edge* v0_next;
			HE_edge* v1_prev;
			HE_edge* v1_next;
			HE_edge* edge;
		};
		
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

		HE_edge* insertDegenerateEdge(HE_vert* v0,HE_vert* v1,const _edgeAdjectencyData& data)
		{
			if(data.edge)
				return data.edge;

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
			HE_edge* cur = e->pair_edge;

			while(cur->next_edge != e)
				cur = cur->next_edge->pair_edge;

			cur->next_edge = e->pair_edge->next_edge;

			//find source of e-pair
			cur = e;

			while(cur->next_edge != e->pair_edge)
				cur = cur->next_edge->pair_edge;

			cur->next_edge = e->next_edge;

			if(e->end_vertex->edge == e->pair_edge)
			{
				//fix e-end_vertex
				if(e->next_edge == e->pair_edge)
					e->end_vertex->edge = nullptr;//internalRemove(e->end_vertex); // can't fix, remove orphaned vertex
				else
					e->end_vertex->edge = e->next_edge;
			}
			if(e->pair_edge->end_vertex->edge == e)
			{
				//fix pair-end_vertex
				if(e->pair_edge->next_edge == e)
					e->pair_edge->end_vertex->edge = nullptr;//internalRemove(e->pair_edge->end_vertex); // can't fix, remove orphaned vertex
				else
					e->pair_edge->end_vertex->edge = e->pair_edge->next_edge;
			}
					
			internalRemove(e);
			return true;
		}

		bool internalMergeDoubleEdge(HE_edge* e1,HE_edge* e2)
		{
			return false;
		}

		inline void internalRemove(HE_edge* e)
		{
			_edgesFree.push_back(e->index);
			_edgesFree.push_back(e->pair_edge->index);
			_edges[e->index] = nullptr;
			_edges[e->pair_edge->index] = nullptr;
 			delete [] ( (e<e->pair_edge) ? e : (e->pair_edge) );
		}
		inline void internalRemove(HE_face* f)
		{
			_facesFree.push_back(f->index);
			_faces[f->index] = nullptr;
 			delete f;
		}
		inline void internalRemove(HE_vert* v)
		{
			_verticesFree.push_back(v->index);
			_vertices[v->index] = nullptr;
 			delete v;
		}

		inline HE_edge* internalAllocEdge()
		{
			HE_edge* e= new HE_edge[2];
			e->pair_edge = &e[1];
			e->pair_edge->pair_edge = e;
			if(!_edgesFree.empty())
			{
				e->index = _edgesFree.front();
				_edges[e->index] = e;
				_edgesFree.pop_front();
				e->pair_edge->index = _edgesFree.front();
				_edges[e->pair_edge->index] = e->pair_edge;
				_edgesFree.pop_front();
			}
			else
			{
				e->index = _edges.size();
				e->pair_edge->index = _edges.size() +1;
				_edges.push_back(e);
				_edges.push_back(e->pair_edge);
			}
			return e;
		}

		inline HE_face* internalAllocFace()
		{
			HE_face* f = new HE_face;
			if(!_edgesFree.empty())
			{
				f->index = _facesFree.front();
				_faces[f->index] = f;
				_facesFree.pop_front();
			}
			else
			{
				f->index = _faces.size();			
				_faces.push_back(f);
			}
			return f;
		}
		inline HE_vert* internalAllocVert()
		{
			HE_vert* v = new HE_vert;
			if(!_verticesFree.empty())
			{
				v->index = _verticesFree.front();
				_vertices[v->index] = v;
				_verticesFree.pop_front();
			}
			else
			{
				v->index = _vertices.size();			
				_vertices.push_back(v);
			}
			return v;
		}

		inline const HE_edge* getInternal(const Edge& e) const
		{
			return (const HE_edge*)e.internal_data;
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
			return ((HE_edge*)e.internal_data);
		}
		inline HE_vert* getInternal(const Vertex& v)
		{
			return ((HE_vert*)v.internal_data);
		}
		inline HE_face* getInternal(const Face& f)
		{
			return ((HE_face*)f.internal_data);
		}

		inline Edge getExternal(const HE_edge* e) const
		{
			if(e==nullptr)
				return nullEdge;
			return e<e->pair_edge?Edge(const_cast<HE_edge*>(e)):Edge(const_cast<HE_edge*>(e->pair_edge));
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
				(f == _faces[f->index]);
		}
		inline bool isValid(const HE_edge* e) const
		{
			return 
				(e!=nullptr) && 
				(e->end_vertex!=nullptr) && 
				(e->index < _edges.size()) && 
				(e == _edges[e->index]);
		}
		inline bool isValid(const HE_vert* v,bool ignore_invalidated = false) const
		{
			return 
				(v!=nullptr) && 
				(v->edge!=nullptr || ignore_invalidated) && 
				(v->index < _vertices.size()) && 
				(v == _vertices[v->index]);
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
/*
			HE_vert* vp[PolySize][2];
			HE_edge* potential_edges[PolySize];
			for(int i = 0 ; i < PolySize; ++i)
			{
				if(!isValid(v[i],true))
					return nullFace;
				vp[i][0] = v[i];
				vp[i][1] = v[(i+1)%PolySize];
				potential_edges[i] = getInternal(GetEdge(MakePair(vertices[i],vertices[(i+1)%PolySize])));
			}
			
			int num_existing_edges = 0;

			//check orientation of used edges
			for(int i = 0 ; i < PolySize; ++i)
				if(potential_edges[i])
				{
					if(potential_edges[i]->face)
						potential_edges[i] = potential_edges[i]->pair_edge;
		
					if(potential_edges[i]->face)//both sides in use
						return nullFace;
							
					if(potential_edges[i]->pair_edge->end_vertex != vp[i][0] || potential_edges[i]->end_vertex != vp[i][1])
						return nullFace; // wrong orientation

					num_existing_edges ++;
				}

			//everything ok add face
			HE_face* f = internalAllocFace();
			f->edge = nullptr;

			//everything ok now add the edges
			for(int i = 0 ; i < PolySize; ++i)
			{
				if(potential_edges[i])
				{
					potential_edges[i] -> face = f;
					if(f->edge == nullptr)
						f->edge = potential_edges[i];
				}
				else
				{
					//add new edges
					potential_edges[i] = internalAllocEdge();
					potential_edges[i]->end_vertex = vp[i][1];
					potential_edges[i]->face = f;
					potential_edges[i]->next_edge = nullptr;

					potential_edges[i]->pair_edge->end_vertex = vp[i][0];
					potential_edges[i]->pair_edge->face = f;
					potential_edges[i]->pair_edge->next_edge = nullptr;
				}
			}

			// fix outer next_edges
			for(int i = 0 ; i < PolySize; ++i)
				if(potential_edges[i]->pair_edge->next_edge == nullptr)
				{
					if(potential_edges[i]->pair_edge->end_vertex->edge == nullptr)
					{
						potential_edges[i]->pair_edge->end_vertex->edge = potential_edges[i];
						potential_edges[i]->pair_edge->next_edge = potential_edges[(i+2)%PolySize]->pair_edge;
					}
					else
					{
						HE_edge* outer = potential_edges[i]->pair_edge->end_vertex->edge;
		
						//find edge poitning away from this vertex who's pair is an outer
						while(outer->pair_edge->face != nullptr)
							outer = outer->pair_edge->next_edge;

						potential_edges[i]->pair_edge->next_edge = outer->pair_edge->next_edge;
						outer->pair_edge->next_edge = potential_edges[i]->pair_edge;
					}
				}

			// fix inner edges
			for(int i = 0 ; i < PolySize; ++i)
				potential_edges[i]->next_edge = potential_edges[(i+1)%PolySize];
*/

#endif