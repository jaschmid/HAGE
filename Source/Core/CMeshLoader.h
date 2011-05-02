/********************************************************/
/* FILE: CMeshLoader.h                                 */
/* DESCRIPTION: Defines various Resource Fromats for    */
/*              usage with the HAGE Engine              */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/


#ifndef __INTERNAL_MESH_LOADER_H__
#define __INTERNAL_MESH_LOADER_H__

namespace HAGE {
	
	struct WMOHeader
	{
		u8 fourcc[4]; // Magic
	};

	struct WMOChunkHeader
	{
		union {
			u8 fourcc[4]; // Identifies what Chunk it is
			u32 fourcc_code;
		};
		u32 size;
	};

	struct WMOBoundingBox
	{
		Vector3<> Corner1; // Bounding box corner 1
		Vector3<> Corner2; // Bounding box corner 2
	};

	struct WMOChunk_MOHD
	{
		u32 nTextures; // number of materials
		u32 nGroups; // number of WMO groups
		u32 nPortals; // number of portals
		u32 nLights; // number of lights
		u32 nModels; // number of M2 models imported
		u32 nDoodads; // number of dedicated files (*see below this table!) 
		u32 nDoodadSets; // number of doodad sets
		u32 AmbientColor; // ambient color?
		u32 nWMOId; // WMO ID (column 2 in WMOAreaTable.dbc)
		WMOBoundingBox bound;
		u32 LiquidType; // LiquidType related, see below in the MLIQ chunk.
	};

	struct WMO_MatHeader
	{		
		u32 flags;
		u32 specular;
		u32 transparent; // Blending: 0 for opaque, 1 for transparent
		u32 nameStart; // Start position for the first texture filename in the MOTX data block	
		u32 col1; // color
		u32 d3; // flag
		u32 nameEnd; // Start position for the second texture filename in the MOTX data block
		u32 col2; // color
		u32 d4; // flag
		u32 col3;
		f32 f2;
		Vector3<f32> diffColor;
		u32 texture1; // this is the first texture object. of course only in RAM. leave this alone. :D
		u32 texture2; // this is the second texture object.
	};

	struct WMO_GroupInfoHeader
	{
		u32 flags;
		WMOBoundingBox bound;
		u32 ofName;
	};

	struct WMO_PortalRect
	{
		Vector3<f32> Rect[4];
	};

	struct WMO_PortalInformation
	{
		u16 BaseVertexIndex;
		u16 nVertices;
		Vector3<f32> Normal;
		f32	unknown;
	};

	struct PortalIdentifier
	{
		u16 nPortalIndex;
		u16 nGroupIndex;
		i16 iSide;
		u16 filler;
	};

	struct WMO_PortalRelationships
	{
		PortalIdentifier	relationships[2];
	};

	typedef enum _WMO_LIGHT_TYPE
	{
		WMO_OMNI_LGT = 0,
		WMO_SPOT_LGT = 1,
		WMO_DIRECT_LGT = 2,
		WMO_AMBIENT_LGT = 3
	} WMO_LIGHT_TYPE;

	struct WMO_Light
	{
		u8 LightType;
		u8 type;
		u8 useAtten;
		u8 pad;
		u32 color;
		Vector3<f32> position;
		f32 intensity;
		f32 attenStart;
		f32 attenEnd;
		f32 unk[4];//direction?
	};

	struct WMO_DoodadSet
	{
		char name[20];
		u32 firstInstanceIndex;
		u32 numDoodads;
		u32 nulls;
	};

	struct WMO_DoodadInstance
	{
		u32 nameIndex;
		Vector3<f32> pos;
		f32 rot[4];//quaternion
		f32 scale;
		u32 color;
	};
	
	struct WMO_MVER
	{
		u32 version;
	};

	struct WMO_Fog
	{
		u32 flags;
		Vector3<f32> pos;
		f32 rad_min;
		f32 rad_max;
		f32 fog_end;
		f32 fog_start;
		u32 fog_color;
		f32 unk1;
		f32 unk2;
		u32 color2;
	};

	struct WMO_GroupHeader 
	{
		i32 nameStart, nameStart2, flags; 
		f32 box1[3], box2[3]; 
		u16 portalStart, portalCount;
		u16 batches[4];
		u8 fogs[4];
		u32 unk1, id, unk2, unk3;
	};

	template <u8 a, u8 b, u8 c, u8 d>
	struct WMO_FourCC
	{
		static const u32 value = (((((a << 8) | b) << 8) | c) << 8) | d;
	};
	
	static const u32 WMO_MVERMagic = WMO_FourCC<'M','V','E','R'>::value;
	static const u32 WMO_MOHDMagic = WMO_FourCC<'M','O','H','D'>::value;
	static const u32 WMO_MOTXMagic = WMO_FourCC<'M','O','T','X'>::value;
	static const u32 WMO_MOMTMagic = WMO_FourCC<'M','O','M','T'>::value;
	static const u32 WMO_MOGNMagic = WMO_FourCC<'M','O','G','N'>::value;
	static const u32 WMO_MOGIMagic = WMO_FourCC<'M','O','G','I'>::value;
	static const u32 WMO_MOSBMagic = WMO_FourCC<'M','O','S','B'>::value;
	static const u32 WMO_MOPVMagic = WMO_FourCC<'M','O','P','V'>::value;
	static const u32 WMO_MOPTMagic = WMO_FourCC<'M','O','P','T'>::value;
	static const u32 WMO_MOPRMagic = WMO_FourCC<'M','O','P','R'>::value;
	static const u32 WMO_MOVVMagic = WMO_FourCC<'M','O','V','V'>::value;
	static const u32 WMO_MOVBMagic = WMO_FourCC<'M','O','V','B'>::value;
	static const u32 WMO_MOLTMagic = WMO_FourCC<'M','O','L','T'>::value;
	static const u32 WMO_MODSMagic = WMO_FourCC<'M','O','D','S'>::value;
	static const u32 WMO_MODNMagic = WMO_FourCC<'M','O','D','N'>::value;
	static const u32 WMO_MODDMagic = WMO_FourCC<'M','O','D','D'>::value;
	static const u32 WMO_MFOGMagic = WMO_FourCC<'M','F','O','G'>::value;
	
	static const u32 WMO_MOGPMagic = WMO_FourCC<'M','O','G','P'>::value;
	static const u32 WMO_MOPYMagic = WMO_FourCC<'M','O','P','Y'>::value;
	static const u32 WMO_MOVIMagic = WMO_FourCC<'M','O','V','I'>::value;
	static const u32 WMO_MOVTMagic = WMO_FourCC<'M','O','V','T'>::value;
	static const u32 WMO_MONRMagic = WMO_FourCC<'M','O','N','R'>::value;
	static const u32 WMO_MOTVMagic = WMO_FourCC<'M','O','T','V'>::value;
	static const u32 WMO_MOBAMagic = WMO_FourCC<'M','O','B','A'>::value;
	static const u32 WMO_MOLRMagic = WMO_FourCC<'M','O','L','R'>::value;
	static const u32 WMO_MOBSMagic = WMO_FourCC<'M','O','B','S'>::value;
	static const u32 WMO_MODRMagic = WMO_FourCC<'M','O','D','R'>::value;
	static const u32 WMO_MOBNMagic = WMO_FourCC<'M','O','B','N'>::value;
	static const u32 WMO_MOBRMagic = WMO_FourCC<'M','O','B','R'>::value;
	static const u32 WMO_MOBVMagic = WMO_FourCC<'M','O','B','V'>::value;
	static const u32 WMO_MOBPMagic = WMO_FourCC<'M','O','B','P'>::value;
	static const u32 WMO_MOBIMagic = WMO_FourCC<'M','O','B','I'>::value;
	static const u32 WMO_MOBGMagic = WMO_FourCC<'M','O','B','G'>::value;
	static const u32 WMO_MOCVMagic = WMO_FourCC<'M','O','C','V'>::value;
	static const u32 WMO_MLIQMagic = WMO_FourCC<'M','L','I','Q'>::value;
	static const u32 WMO_MORIMagic = WMO_FourCC<'M','O','R','I'>::value;
	static const u32 WMO_MORBMagic = WMO_FourCC<'M','O','R','B'>::value;

	class CDrawableMeshLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CDrawableMeshLoader(IDataStream* pStream,const IResource* pPrev);
		~CDrawableMeshLoader();
		IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
	private:

		class CDrawableMesh : public IDrawableMesh
		{
		public:
			void Draw(const Vector3<>& position, const Matrix4<>& view,const Matrix4<>& inv_view, const Matrix4<>& proj,const Vector3<>& lPosition, const Vector3<>& lColor)  const;
			const APIWVertexArray* GetVertexArray() const{return _pVertexArray;}
			const APIWTexture* GetTexture(u32 index) const{return (index<_nTextures)?_Textures[index]->GetTexture():nullptr;}
			virtual ~CDrawableMesh();
			CDrawableMesh(const TResourceAccess<IMeshData>& pData);

			IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
		private:

			APIWVertexBuffer*							_pVertexBuffer;
			APIWVertexArray*							_pVertexArray;
			APIWConstantBuffer*							_pConstants;
			APIWEffect*									_pEffect;
			u32											_nTextures;
			std::vector<TResourceAccess<ITextureImage>>	_Textures;
			std::pair<std::string,guid>*				_pTextureNames;
		};

		std::pair<std::string,guid>*				_pDependancies;

		CDrawableMesh* _pDrawableMesh;
	};

	class CMeshDataLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CMeshDataLoader(IDataStream* pStream,const IResource* pPrev);
		~CMeshDataLoader();
		IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);

		bool IsValid(){return _pMeshData?true:false;}
	private:

		class CMeshData : public IMeshData
		{
		public:				
			virtual u32 GetNumVertices() const;
			virtual u32 GetVertexData(const u8** pDataOut,u32& pOutStride,VERTEX_DATA_TYPE type) const;
			virtual u32	GetNumIndices() const;
			virtual u32 GetIndexData(const u8** pDataOut) const;
			virtual const void* GetExtendedData(EXTENDED_DATA_TYPE type) const;
			virtual const char* GetTextureName(u32 index) const;
			virtual const TResourceAccess<IImageData>* GetTexture(u32 index) const;

			virtual ~CMeshData();
			CMeshData(IDataStream* pData);

			bool IsValid(){return _bValid;}

			IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
		private:
			
			void GenerateNormals();
			bool TryLoadHGEO(IDataStream* pData);
			bool TryLoadM2(IDataStream* pData);
			bool TryLoadWMO(IDataStream* pData);
			void FinalizeLoadM2(const ResourceAccess* dependanciesIn,u32 nDependanciesIn);
			void FinalizeLoadWMO(const ResourceAccess* dependanciesIn,u32 nDependanciesIn);
			bool	_MD2;
			bool	_WMO;
			IDataStream* _pData;

			std::vector<std::pair<std::string,guid>>	_depencancies;
			std::vector<std::string>					_textureNames;
			std::vector<TResourceAccess<IImageData>>	_textures;

			u32		_vertexSize;
			u32		_nVertices;
			u32		_nIndices;
			u8*		_pVertexData;
			u8*		_pIndexData;
			u8*		_pMaterialData;

			bool	_bValid;
		};

		CMeshData* _pMeshData;
	};
		
	class CRawDataLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CRawDataLoader(IDataStream* pStream,const IResource* pPrev);
		~CRawDataLoader();
		IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
	private:

		class CRawData : public IRawData
		{
		public:		
			virtual u64 GetSize() const;
			virtual u64 GetData(const u8** pDataOut) const;
			virtual ~CRawData();
			CRawData(IDataStream* pData);
		private:
			u64		_Size;
			void*	_Data;
		};

		CRawData* _pRawData;
	};
}

#endif