#ifndef DATA_PROCESSOR__INCLUDED
#define DATA_PROCESSOR__INCLUDED

#include "header.h"
class GenerationDomain;
#include "VirtualTextureGenerator.h"

#include <list>

namespace HAGE
{

	class DataProcessor : public DomainMember<GenerationDomain>
	{
	public:
		DataProcessor(u32 xStart,u32 xEnd,u32 yStart,u32 yEnd);
		~DataProcessor();

		const static bool bProcessMesh = false;
		const static bool bProcessTexture = true;

		bool Process();
	private:

		
		class DataItem
		{
		public:
			DataItem(u32 x,u32 y);
			~DataItem();
			bool IsReady() const {return _bDone;}
			void TryLoad();
			const TResourceAccess<IMeshData>& GetMeshData() const {return _mesh;}
			const TResourceAccess<IImageData>& GetTextureData() const{return _texture;}
			u32 GetX() const{return _x;}
			u32 GetY() const{return _y;}
		private:
			bool _bDone;
			bool _bLoading;
			TResourceAccess<IMeshData> _mesh;
			TResourceAccess<IImageData> _texture;
			u32	_x,_y;
		};
		
		void Finalize();
		void ProcessMesh(const DataItem& mesh);
		void ProcessTexture(const DataItem& texture);

		bool	_bDone;
		std::list<DataItem> _loadingItems;
		std::list<DataItem> _queuedItems;

		u32 _xBegin,_xEnd,_yBegin,_yEnd;
		
		typedef HAGE::set<HAGE::MeshGeometryFeature,HAGE::MeshDecimatorFeature<f32>> MeshFeatures;

		
		struct VertexData : public HAGE::MinVertexType<MeshFeatures>::type
		{
			operator Vector3<>&()
			{
				return Position;
			}
			operator const Vector3<>&() const
			{
				return Position;
			}
		};

		typedef HAGE::HageMeshEx< MeshFeatures , VertexData> MeshType;

		MeshType	_mesh;
		SparseVirtualTextureGenerator hsvt;
		Vector3<> min,max;//extents of the mesh
		
		void writeOutputMesh();
		void mergeMeshVertices();
	};

}

#endif
