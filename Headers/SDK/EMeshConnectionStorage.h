#include <types.h>
#include <tuple>

namespace HAGE {

class EMeshConnectionStorage
{
public:
	virtual u32 GetNumFaces() = 0;
	virtual u32 GetNumEdges() = 0;
	virtual u32 GetNumVertices() = 0;

	typedef struct { u32 index; } Edge;
	typedef struct { u32 index; } Vertex;
	typedef struct { u32 index; } Face;

	virtual Edge GetEdge(u32 index) { assert(index < GetNumVertices()); Edge e = {index}; return e;}
	virtual Vertex GetVertex(u32 index) { assert(index < GetNumEdges()); Vertex v = {index}; return v;}
	virtual Face GetFace(u32 index) { assert(index < GetNumFaces()); Face f = {index}; return f;}

	typedef std::tuple<Face,Face> FacePair;
	typedef std::tuple<Vertex,Vertex> VertexPair;
	
	typedef std::tuple<Vertex,Vertex,Vertex> VertexTriple;
	typedef std::tuple<Edge,Edge,Edge> EdgeTriple;

	virtual EdgeTriple GetFaceEdges(Face face) = 0;
	virtual VertexTriple GetFaceVertices(Face face) = 0;

	virtual VertexPair GetEdgeVertices(Edge edge) = 0;
	virtual FacePair GetEdgeFaces(Edge edge) = 0;

	virtual Edge GetNumVertexEdges(Vertex vertex) = 0;
	virtual Face GetNumVertexFaces(Vertex vertex) = 0;
	virtual Edge GetVertexEdge(Vertex vertex,u32 edgeN) = 0;
	virtual Face GetVertexFace(Vertex vertex,u32 faceN) = 0;

	virtual ~EMeshConnectionStorage() {};
private:
};

}