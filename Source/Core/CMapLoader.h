/********************************************************/
/* FILE: CMapLoader.h                                 */
/* DESCRIPTION: Defines various Resource Fromats for    */
/*              usage with the HAGE Engine              */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/


#ifndef __INTERNAL_MAP_LOADER_H__
#define __INTERNAL_MAP_LOADER_H__

namespace HAGE {
	
	struct ChunkHeader
	{
		union {
			u8 fourcc[4]; // Identifies what Chunk it is
			u32 fourcc_code;
		};
		u32 size;
	};

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
	struct ADT_MODF_Entry
	{
		u32 nameId;
		u32 uniqueId;
		Vector3<f32> pos;
		Vector3<f32> rot;
		Vector3<f32> extents[2];
		u16 flags;
		u16 doodadSet;
		u16 nameSet;
		u16 padding;
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
	static const u32 ADT_MODFMagic = ADT_FourCC<'M','O','D','F'>::value;
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
	

	static const f32 fADT_TileXSize = 100.0f/3.0f;
	static const f32 fADT_TileZSize = 100.0f/3.0f;
	static const f32 fADT_CellXSize = fADT_TileXSize / 8.0f;
	static const f32 fADT_CellZSize = fADT_TileZSize / 8.0f;
	static const f32 fADT_ChunkXSize = fADT_TileXSize*16.0f;
	static const f32 fADT_ChunkZSize = fADT_TileZSize*16.0f;

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
			CMapData();
			virtual u32 GetNumVertices() const;
			virtual u32 GetVertexData(const u8** pDataOut,u32& pOutStride,VERTEX_DATA_TYPE type) const;
			virtual u32	GetNumIndices() const;
			virtual u32 GetIndexData(const u8** pDataOut) const;
			virtual const void* GetExtendedData(EXTENDED_DATA_TYPE type) const;
			virtual const char* GetTextureName(u32 index) const;
			virtual const TResourceAccess<IImageData>* GetTexture(u32 index) const;
			
			bool IsValid(){return isValid;}

			bool LoadInit(IDataStream* pADT,std::vector<std::pair<std::string,guid>>& dependanciesOut);
			bool LoadStep(const ResourceAccess* dependanciesIn,std::vector<std::pair<std::string,guid>>& dependanciesInOut);
			bool LoadObj(const TResourceAccess<IRawData>& data,std::vector<std::pair<std::string,guid>>& dependanciesOut);

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

			std::string						_textureName;
			TResourceAccess<IImageData>		_texture;
			
			std::vector<ChildObject>		_childObjects;
			u32								_nChildObjects;

			bool	isValid;
		};
		
		std::vector<std::pair<std::string,guid>> dependancies;
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