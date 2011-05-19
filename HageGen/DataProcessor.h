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

		const static bool bProcessMesh = true;
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
			u32 GetX() const{return _x;}
			u32 GetY() const{return _y;}
		private:
			bool _bDone;
			bool _bLoading;
			TResourceAccess<IMeshData> _mesh;
			u32	_x,_y;
		};
		
		void Finalize();
		void ProcessMesh(const DataItem& mesh);
		void ProcessTexture(const DataItem& texture);

		bool	_bDone;
		std::list<DataItem> _loadingItems;
		std::list<DataItem> _queuedItems;

		u32 _xBegin,_xEnd,_yBegin,_yEnd;
		
		typedef HAGE::set<HAGE::MeshGeometryFeature,HAGE::MeshDecimatorFeature<f64>> MeshFeatures;

		struct VertexData : public HAGE::MinVertexType<MeshFeatures>::type
		{
			Vector2<f32>	TexCoord;
			u32				Material;
			operator Vector3<>&()
			{
				return Position;
			}
			operator const Vector3<>&() const
			{
				return Position;
			}
		};

		const static u32 TempMaterialMarker = 0x80000000;

		std::map<u32,SparseVirtualTextureGenerator::TextureReference> materials;

		typedef HAGE::HageMeshEx< MeshFeatures , VertexData> MeshType;
		
		void loadMesh(MeshType& mesh,const TResourceAccess<IMeshData>& data,const Matrix4<>&);
		void processMeshTextures(MeshType& mesh,const TResourceAccess<IMeshData>& data,SparseVirtualTextureGenerator::RelationArray& arr);

		std::array<Vector2<>,2> packTexture(u32 material_index,Vector2<> mincoord,Vector2<> maxcoord,u32 xSize,u32 ySize,const u32* pData,const SparseVirtualTextureGenerator::RelationArray& arr);

		static MeshType::VertexType DecimateUpdate(const MeshType::VertexPair& vp,const MeshType::Edge& e);

		MeshType	_mesh;
		SparseVirtualTextureGenerator hsvt;
		
		void writeOutputMesh();
		static void mergeMeshVertices(MeshType& mesh);
	};

}

#endif
