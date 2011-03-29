/********************************************************/
/* FILE: EMeshIndexed.h                                 */
/* DESCRIPTION: Defines an Indexed Connection Storage   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/
/*

I reconsidered, this class is useless

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif


#ifndef __EMESHINDEXED_H__
#define __EMESHINDEXED_H__

#include "EMeshConnectionStorage.h"

#include <vector>

namespace HAGE {

	class EMeshIndexed : public EMeshConnectionStorage
	{
	public:
		u32 GetNumFaces()
		{
			return _faces.size();
		}
		u32 GetNumEdges()
		{
			return _edges.size();
		}
		u32 GetNumVertices()
		{
			return _nVertices;
		}

		EdgeTriple GetFaceEdges(Face face)
		{
			assert(face.index < _faces.size());
			VertexTriple vertices = GetFaceVertices(face);
			Edge edges[3] = {nullEdge,nullEdge,nullEdge};
			int written = 0;
			for(auto it = _edges.begin();it!=_edges.end();++it)
			{
				if ( contains(vertices,std::get<0>(*it)) )
				{
					if ( contains(vertices,std::get<1>(*it)) );
					else continue;
				}
				else continue;

				Edge e = {it - _edges.begin() };
				edges[written] = e;
				written++;
				if(written == 3)
					break;
			}

			return EdgeTriple(edges[0],edges[1],edges[2]);
		}

		VertexTriple GetFaceVertices(Face face)
		{
			assert(face.index < _faces.size());
			return _faces[face.index];
		}

		VertexPair GetEdgeVertices(Edge edge)
		{
			assert(edge.index < _edges.size());
			return _edges[edge.index];
		}
		FacePair GetEdgeFaces(Edge edge)
		{
			VertexPair vertices = GetEdgeVertices(edge);
			Face faces[2] = {nullFace,nullFace};
			int written = 0;
			for(auto it = _faces.begin(); it!= _faces.end(); ++it)
			{
				if( contains(*it,std::get<0>(vertices)) && contains(*it,std::get<1>(vertices)) )
				{
					Face this_face = {it - _faces.begin()};
					faces[written] = this_face;
					written++;
					if(written == 2)
						break;
				}
			}

			return FacePair(faces[0],faces[1]);
		}
		
		Edge GetFirstVertexEdge(Vertex vertex)
		{
			return GetNextVertexEdge(vertex,nullEdge);
		}
		Face GetFirstVertexFace(Vertex vertex)
		{
			return GetNextVertexFace(vertex,nullFace);
		}

		Edge GetNextVertexEdge(Vertex vertex,Edge previous_edge)
		{
			for(auto it = ((previous_edge == nullEdge) ? (_edges.begin() + previous_edge.index) : _edges.begin() );it != _edges.end(); it++)
			{
				if( contains(*it,vertex) )
				{
					Edge e= {it - _edges.begin()};
					return e;
				}
			}
			return nullEdge;
		}
		Face GetNextVertexFace(Vertex vertex,Face previous_face)
		{
			for(auto it = ((previous_face == nullFace) ? (_faces.begin() + previous_face.index) : _faces.begin() ); it != _faces.end(); ++it)
			{
				if( contains(*it,vertex) )
				{
					Face f= {it - _faces.begin()};
					return f;
				}
			}
			return nullFace;
		}

		void InvalidateFace(Face face)
		{
			assert(face.index <= _faces.size());
			//remove orphaned edges
			EdgeTriple edges = GetFaceEdges(face);
			Edge a_edges[3] = {std::get<0>(edges),std::get<1>(edges),std::get<2>(edges)};
			for(int i = 0; i < 3; i++)
			{
				Edge& edge = a_edges[i];
				FacePair faces = GetEdgeFaces(edge);
				if(contains(faces,nullFace))
				{
					std::get<0>(_edges[edge.index]) = nullVertex;
					std::get<1>(_edges[edge.index]) = nullVertex;
				}
			}

			std::get<0>(_faces[face.index]) = nullVertex;
			std::get<1>(_faces[face.index]) = nullVertex;
			std::get<2>(_faces[face.index]) = nullVertex;
		}

		void InvalidateEdge(Edge edge)
		{
			FacePair faces = GetEdgeFaces(edge);

			InvalidateFace(std::get<0>(faces));
			InvalidateFace(std::get<1>(faces));
		}

		void InvalidateVertex(Vertex v)
		{
			Face f = nullFace;
			while( (f = GetNextVertexFace(v,f)) != nullFace)
				InvalidateFace(f);		
		}

		Face InsertFace(VertexTriple vertices)
		{
		}

		Face InsertFace(EdgeTriple edges)
		{
		}

		void Rebuild() = 0;

		~EMeshIndexed() {};
	private:
		
		std::vector<VertexTriple>	_faces;
		std::vector<VertexPair>		_edges;
		u32							_nVertices;
	};

}

#endif

*/