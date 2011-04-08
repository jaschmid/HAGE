/********************************************************/
/* FILE: MeshUtils.h                                    */
/* DESCRIPTION: Extensions for Mesh Utils               */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __MESH_UTILS_H__
#define __MESH_UTILS_H__

#include <tuple>
#include <list>
#include <vector>
#include <array>

namespace HAGE {

	template< class _Processor , class _SubFeature > class MeshFeatureSet : public _SubFeature
	{
	};

	template<
		class _FeatureSet, 
		class _VertexType = typename _FeatureSet::BaseVertexType, 
		class _EdgeType = typename _FeatureSet::BaseEdgeType, 
		class _FaceType = typename _FeatureSet::BaseFaceType
		> class HageMeshEx : public typename _FeatureSet::MeshType
	{
	public:
	private:
	};

	class MeshGeometryPositionData
	{
	public:
		Vector3<>	Position;
	};
	
	class MeshGeometryNormalData
	{
	public:
		Vector3<>	Normal;
	};

	class MeshGeometryFeature {};

	class M

	template<class _MeshType> class MeshGeometryProcessor
	{
		MeshGeometryProcessor() : _mesh(static_cast<_MeshType*>(this))
		{
			static_assert( std::is_base_of<MeshGeometryPositionData,typename _MeshType::VertexType>::value , "MeshGeometryProcessor requires Vertices to inherit from MeshGeometryPositionData");
			static_assert( std::is_base_of<MeshGeometryNormalData,typename _MeshType::VertexType>::value , "MeshGeometryProcessor requires Vertices to inherit from MeshGeometryNormalData");
			static_assert( std::is_base_of<MeshGeometryNormalData,typename _MeshType::FaceType>::value , "MeshGeometryProcessor requires Vertices to inherit from MeshGeometryNormalData");
		}
		~MeshGeometryProcessor()
		{
		}

	private:
		_MeshType* const		_mesh;
	};

	class MeshDecimatorQData
	{
	public:
		Matrix4<>	Q;
	};



}

#endif
