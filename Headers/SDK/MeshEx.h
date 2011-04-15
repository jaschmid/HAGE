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
#include <boost/mpl/set.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/iter_fold.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/erase_key.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/inherit_linearly.hpp>

namespace HAGE {
	
	using boost::mpl::set;

	namespace _MeshExInternal
	{

		using boost::mpl::front; 
		using boost::mpl::erase_key;
		using boost::mpl::set0;
		using boost::mpl::insert;
		using boost::mpl::iter_fold;
		using boost::mpl::deref;
		using boost::mpl::_1;
		using boost::mpl::_2;
		using boost::mpl::inherit_linearly;
		using boost::mpl::inherit;
		
		template<class _Feature,class _Final, class _Features> struct get_implementation
		{
			typedef typename _Feature::Implementation<_Final,_Features>::type type;
		};

		template<class _Features,class _Final> struct generate_mesh_implementation	
		{
			typedef typename inherit_linearly<
				typename _Features::Features,
				inherit< _1, get_implementation<_2,typename _Final,typename _Features> >,
				typename _Features::BaseMesh
			>::type type;
		};

		template<class _Sequence> class set_inherit : 
			public front<_Sequence>::type, 
			public set_inherit< typename erase_key<_Sequence, typename front<_Sequence>::type>::type >
		{
		public:
		};

		template<> struct set_inherit<set0<>> 
		{
		};	

		template<class _Sequence> struct generate_data_from_requirement
		{
			typedef set_inherit<_Sequence> type;
		};
		template<> struct generate_data_from_requirement<set0<>>
		{
			typedef void type;
		};

		template<class _Set1,class _Set2> struct set_union
		{
			typedef 
				typename iter_fold< 
					typename _Set2, 
					typename _Set1 , 
					insert<		
						_1 , 
						deref<_2> 
					> 
				>::type
					type;
		};
		template<class _Set1> struct set_union<_Set1,void>
		{
			typedef _Set1 type;
		};
		
		template<class _Set1,class _FeatureSet> struct vertex_union_wrapper
		{
			typedef 
				typename set_union<_Set1,typename _FeatureSet::VertexRequirements>::type type;
		};
		template<class _Set1,class _FeatureSet> struct edge_union_wrapper
		{
			typedef 
				typename set_union<_Set1,typename _FeatureSet::EdgeRequirements>::type type;
		};
		template<class _Set1,class _FeatureSet> struct face_union_wrapper
		{
			typedef 
				typename set_union<_Set1,typename _FeatureSet::FaceRequirements>::type type;
		};

		template<class _Set> struct generate_vertex_req_set {
			typedef typename iter_fold< typename _Set, typename set0<>::type , 
				vertex_union_wrapper< 	_1 , deref<_2> >
			>::type	type;
		};
		template<class _Set> struct generate_edge_req_set {
			typedef typename iter_fold< typename _Set, typename set0<>::type , 
				edge_union_wrapper< 	_1 , deref<_2> >
			>::type	type;
		};
		template<class _Set> struct generate_face_req_set {
			typedef typename iter_fold< typename _Set, typename set0<>::type , 
				face_union_wrapper< 	_1 , deref<_2> >
			>::type	type;
		};


	}

	template<class _Features>
	struct MinVertexType
	{
		typedef typename _MeshExInternal::generate_data_from_requirement<typename _MeshExInternal::generate_vertex_req_set<_Features>::type>::type type;
	};

	template<class _Features>
	struct MinEdgeType
	{
		typedef typename _MeshExInternal::generate_data_from_requirement<typename _MeshExInternal::generate_edge_req_set<_Features>::type>::type type;
	};

	template<class _Features>
	struct MinFaceType
	{
		typedef typename _MeshExInternal::generate_data_from_requirement<typename _MeshExInternal::generate_face_req_set<_Features>::type>::type type;
	};

	
	template<
		class _Features, 
		class _VertexType,
		class _FaceType,
		class _EdgeType
	> struct MeshFeatureSet
	{
		typedef _Features Features;
		typedef _VertexType VertexType;
		typedef _EdgeType EdgeType;
		typedef _FaceType FaceType;
		typedef CEditableMesh<typename _VertexType,typename _FaceType,typename _EdgeType> BaseMesh;
	};
	
	template<
		class _Features,
		class _VertexType = typename MinVertexType<_Features>::type,
		class _FaceType = typename MinFaceType<_Features>::type,
		class _EdgeType = typename MinEdgeType<_Features>::type
		> class HageMeshEx : 
		public _MeshExInternal::generate_mesh_implementation<MeshFeatureSet<_Features,_VertexType,_FaceType,_EdgeType>, HageMeshEx<_Features,_VertexType,_FaceType,_EdgeType> >::type
	{
	public:

		typedef MeshFeatureSet<_Features,_VertexType,_FaceType,_EdgeType> FeatureSet;
		typedef HageMeshEx<FeatureSet> Type;
		typedef CEditableMesh<_VertexType,_FaceType,_EdgeType> BaseType;

		using BaseType::Vertex;
		using BaseType::Edge;
		using BaseType::Face;
		using BaseType::IndexType;

	private:
	};


}

#include "MeshExGeometry.h"
#include "MeshExDecimator.h"
#include "MeshExCurvature.h"
#include "MeshExSubdivision.h"

#endif
