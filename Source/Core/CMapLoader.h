/********************************************************/
/* FILE: CMapLoader.h                                 */
/* DESCRIPTION: Defines various Resource Fromats for    */
/*              usage with the HAGE Engine              */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/


#ifndef __INTERNAL_MAP_LOADER_H__
#define __INTERNAL_MAP_LOADER_H__

namespace HAGE {
	
	struct WMOHeader
	{
		u8 fourcc[4]; // Magic
	};

	struct ChunkHeader
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

	static const u8 WMO_MOHDMagic[4] = {'M','O','H','D'};
	static const u8 WMO_MOTXMagic[4] = {'M','O','T','X'};
	static const u8 WMO_MOMTMagic[4] = {'M','O','M','T'};
	static const u8 WMO_MOGNMagic[4] = {'M','O','G','N'};
	static const u8 WMO_MOGIMagic[4] = {'M','O','G','I'};
	static const u8 WMO_MOSBMagic[4] = {'M','O','S','B'};
	static const u8 WMO_MOPVMagic[4] = {'M','O','P','V'};
	static const u8 WMO_MOPTMagic[4] = {'M','O','P','T'};
	static const u8 WMO_MOPRMagic[4] = {'M','O','P','R'};
	static const u8 WMO_MOVVMagic[4] = {'M','O','V','V'};
	static const u8 WMO_MOVBMagic[4] = {'M','O','V','B'};
	static const u8 WMO_MOLTMagic[4] = {'M','O','L','T'};
	static const u8 WMO_MODSMagic[4] = {'M','O','D','S'};
	static const u8 WMO_MODNMagic[4] = {'M','O','D','N'};
	static const u8 WMO_MODDMagic[4] = {'M','O','D','D'};
	static const u8 WMO_MFOGMagic[4] = {'M','F','O','G'};

	/// WDT are actually more important than WMO

	struct WDT_MPHD
	{
		u32 flags;
		u32 unknown;
		u32 unused[6];
	};

	struct WDT_MAIN
	{
		u32 loaded;
		u32 unused;
	};

	// there's also MWMO and MODF but whatever

	
	static const u8 WDT_MPHDMagic[4] = {'M','P','H','D'};
	static const u8 WDT_MAINMagic[4] = {'M','A','I','N'};

	// ADT stuff

	struct ADT_REVM
	{
		u32 version;
	};

	struct ADT_MVER
	{
		u32 version;
	};

	struct ADT_MHDR
	{
		u32 flags;
		u32 ofMCIN;
		u32 ofMTEX;
		u32 ofMMDX;
		u32 ofMMID;
		u32 ofMWMO;
		u32 ofMWID;
		u32 ofMDDF;
		u32 ofMODF;
		u32 ofMFBO;
		u32 ofH2O;
		u32 ofMTFX;
		u32 unk[4];
	};

	struct ADT_MCIN_Entry
	{
		u32 ofMCNK;
		u32 sizeMCNK;
		u32 unkFlags;
		u32 unkAsyncId;
	};

	struct ADT_MCIN
	{
		ADT_MCIN_Entry entries[256];
	};

	struct ADT_MDDF_Entry
	{
		u32 nameId;
		u32 uniqueId;
		Vector3<f32> pos;
		Vector3<f32> rot;
		u16 scale;
		u16 flags;
	};

	struct ADT_MH2O_HEADER_Entry
	{
		u32 ofInformation;
		u32 layerCount;
		u64 Render;
	};

	struct ADT_MH2O_HEADER
	{
		ADT_MH2O_HEADER_Entry entries[256];
	};

	struct ADT_MH2O_Information
	{
		u16 liquidType;
		u16 flags;
		f32 heightLevel1;
		f32 heightLevel2;
		u8 xOffset;
		u8 yOffset;
		u8 width;
		u8 height;
		u32 ofsMask2;
		u32 ofsHeightmap;
	};

	struct ADT_MCNK
	{
		u32 flags;
		u32 indexX;
		u32 indexY;
		u32 nLayers;
		u32 nDoodadRefs;
		u32 ofHeight;
		u32 ofNormal;
		u32 ofLayer;
		u32 ofRefs;
		u32 ofAlpha;
		u32 sizeAlpha;
		u32 ofShadow;
		u32 sizeShadow;
		u32 areaId;
		u32 nMapObjRefs;
		u32 holes;
		u64 RLQTM[2];
		u32 predTex;
		u32 noEffectDoodad;
		u32 ofsSndEmitters;
		u32 nSndEmitters;
		u32 ofLiquid;
		u32 sizeLiquid;
		Vector3<> position;
		u32 ofMCCV;
		u32 ofMCLV;
		u32 unused;
	};

	struct ADT_MCVT
	{
		f32 heights[9*9+8*8];
	};

	struct ADT_MCLV
	{
		u32 colors[9*9+8*8];
	};

	struct ADT_MCCV
	{
		u32 colors[9*9+8*8];
	};
	
	struct ADT_MCLY
	{
		static const u32 FLAG_ALPHA_MAP = 0x100;
		static const u32 FLAG_COMPRESSED_LAYER = 0x200;
		u32 textureId;
		u32 flags;
		u32 offsetInMCAL;
		u16 effectId;
		u16 padding;
	};

	template <u8 a, u8 b, u8 c, u8 d>
	struct ADT_FourCC
	{
		static const u32 value = (((((a << 8) | b) << 8) | c) << 8) | d;
	};
	
	
	static const u32 ADT_MVERMagic = ADT_FourCC<'M','V','E','R'>::value;
	static const u32 ADT_MHDRMagic = ADT_FourCC<'M','H','D','R'>::value;
	static const u32 ADT_MAMPMagic = ADT_FourCC<'M','A','M','P'>::value;
	static const u32 ADT_MFBOMagic = ADT_FourCC<'M','F','B','O'>::value;
	static const u32 ADT_MTXFMagic = ADT_FourCC<'M','T','X','F'>::value;
	static const u32 ADT_MCINMagic = ADT_FourCC<'M','C','I','N'>::value;
	static const u32 ADT_MTEXMagic = ADT_FourCC<'M','T','E','X'>::value;
	static const u32 ADT_MMDXMagic = ADT_FourCC<'M','M','D','X'>::value;
	static const u32 ADT_MMIDMagic = ADT_FourCC<'M','M','I','D'>::value;
	static const u32 ADT_MWMOMagic = ADT_FourCC<'M','W','M','O'>::value;
	static const u32 ADT_MWIDMagic = ADT_FourCC<'M','W','I','D'>::value;
	static const u32 ADT_MDDFMagic = ADT_FourCC<'M','D','D','F'>::value;
	static const u32 ADT_MH2OMagic = ADT_FourCC<'M','H','2','O'>::value;
	static const u32 ADT_MCNKMagic = ADT_FourCC<'M','C','N','K'>::value;
	static const u32 ADT_MCCVMagic = ADT_FourCC<'M','C','C','V'>::value;//sub
	static const u32 ADT_MCVTMagic = ADT_FourCC<'M','C','V','T'>::value;//sub
	static const u32 ADT_MCLVMagic = ADT_FourCC<'M','C','L','V'>::value;//sub
	static const u32 ADT_MCLYMagic = ADT_FourCC<'M','C','L','Y'>::value;//sub
	static const u32 ADT_MCNRMagic = ADT_FourCC<'M','C','N','R'>::value;//sub
	static const u32 ADT_MCRFMagic = ADT_FourCC<'M','C','R','F'>::value;//sub
	static const u32 ADT_MCSHMagic = ADT_FourCC<'M','C','S','H'>::value;//sub
	static const u32 ADT_MCALMagic = ADT_FourCC<'M','C','A','L'>::value;//sub
	static const u32 ADT_MCSEMagic = ADT_FourCC<'M','C','S','E'>::value;//sub
	static const u32 ADT_MCMTMagic = ADT_FourCC<'M','C','M','T'>::value;//sub
	

	static const f32 fADT_CellXSize = 1.0f/64.0f;
	static const f32 fADT_CellZSize = 1.0f/64.0f;
	static const f32 fADT_TileXSize = fADT_CellXSize*8;
	static const f32 fADT_TileZSize = fADT_CellZSize*8;
	static const f32 fADT_ChunkXSize = fADT_TileXSize*16;
	static const f32 fADT_ChunkZSize = fADT_TileZSize*16;

	class CMapDataLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CMapDataLoader(IDataStream* pStream,const IResource* pPrev);
		~CMapDataLoader();
		IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);

		bool IsValid(){return _pMapData?true:false;}
		
		IDataStream* _pData;
	private:

		class CMapData : public IMeshData
		{
		public:
			virtual ~CMapData();
			CMapData(IDataStream* pData);
			virtual u32 GetNumVertices() const;
			virtual u32 GetVertexData(const u8** pDataOut,u32& pOutStride,VERTEX_DATA_TYPE type) const;
			virtual u32	GetNumIndices() const;
			virtual u32 GetIndexData(const u8** pDataOut) const;
			virtual const void* GetExtendedData(EXTENDED_DATA_TYPE type) const;
			virtual const char* GetTextureName(u32 index) const;
			
			IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
			bool IsValid(){return isValid;}
		private:
			static const u32 nCellTriangles = 4;
			
			static const u32 nTileXCells = 8;
			static const u32 nTileYCells = 8;
			static const u32 nXTiles = 16;
			static const u32 nYTiles = 16;

			static const u32 nTileIndices = nTileXCells*nTileYCells*nCellTriangles*3;
			static const u32 nTileVertices = (nTileXCells+1)*(nTileYCells+1)+nTileXCells*nTileYCells;
			static const u32 nTiles = nXTiles*nYTiles;
			static const std::array<u32,nTileIndices> TileIndex;
			static const u32 nVertices = nTiles*nTileVertices;
			static const u32 nIndices = nTiles*nTileIndices;
			
			struct MapTile
			{
				Vector3<>	Positions[nTileVertices];
				Vector3<>	Normals[nTileVertices];
				Vector3<>	VertexColors[nTileVertices];
			};

			// heightmap Data
			std::array<Vector3<>,nVertices> _positionData;
			std::array<Vector3<>,nVertices> _normalData;
			std::array<Vector2<>,nVertices> _texcoordData;
			std::array<Vector3<>,nVertices> _vertexColorData;
			std::array<u32,nIndices>		_indexData;

			bool	isValid;
		};

		CMapData* _pMapData;
	};

	class CMapDataImageLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CMapDataImageLoader(IDataStream* pStream,const IResource* pPrev);
		~CMapDataImageLoader();
		IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);

		bool IsValid(){return _pMapData?true:false;}
		
		IDataStream* _pData;
	private:

		class CMapDataImage : public IImageData
		{
		public:
			virtual ~CMapDataImage();
			CMapDataImage(IDataStream* pData);

			virtual u32 GetImageWidth() const;
			virtual u32 GetImageHeight() const;
			virtual u32 GetImageFormat() const;
			virtual u32 GetImageLevels() const;
			virtual const void*	GetImageData() const;
			
			IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
			bool IsValid(){return isValid;}
		private:
			struct MapTile
			{
				ADT_MCNK mcnk;
				u32 nLayers;
				ADT_MCLY layers[4];
				u8 AlphaMap[4096*3];
				u32 AlphaBytes;
				u8 ShadowMap[8*64]; // 1 bit per field
			};

			static const u32 nCellTriangles = 4;
			
			static const u32 nTileXCells = 8;
			static const u32 nTileYCells = 8;
			static const u32 nTileXAlphas = 64;
			static const u32 nTileYAlphas = 64;
			static const u32 nXTiles = 16;
			static const u32 nYTiles = 16;

			static const u32 nTiles = nXTiles*nYTiles;
			
			u32		_Width;
			u32		_Height;
			u32		_Levels;
			u32		_Format;
			void*	_Data;

			MapTile*	_pTiles;
			std::pair<std::string,guid>* pDependancies;
			std::vector<std::string> _texNames;

			bool	isValid;
		};

		CMapDataImage* _pMapData;
	};

}

#endif