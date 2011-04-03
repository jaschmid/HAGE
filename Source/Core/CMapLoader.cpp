#include "HAGE.h"

#include <stdio.h>

#include "CMapLoader.h"

namespace HAGE
{
	IResourceLoader* CMapDataLoader::Initialize(IDataStream* pStream,const IResource* pPrev)
	{
		CMapDataLoader* pLoader = new CMapDataLoader(pStream,pPrev);
		if(!pLoader->IsValid())
		{
			delete pLoader;
			pLoader = nullptr;
		}
		return pLoader;
	}

	CMapDataLoader::CMapDataLoader(IDataStream* pStream,const IResource* pPrev)
		: _pData (pStream)
	{
		_pMapData = new CMapData(pStream);
		if(!_pMapData->IsValid())
		{
			delete _pMapData;
			_pMapData = nullptr;
		}
	}
	CMapDataLoader::~CMapDataLoader()
	{
	}

	IResource* CMapDataLoader::Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
	{
		return _pMapData->Finalize(dependanciesIn,pDependanciesOut,nDependanciesInOut);
	}
	
	inline static void forceRead(IDataStream* pData,u64 size,void* pOut)
	{
		assert(pData->Read(size,(u8*)pOut) == size);
	}

	inline static Vector3<> fixCoordSystem(Vector3<> v)
	{
			return Vector3<>(v.x, v.z, -v.y);
	}
	
	CMapDataLoader::CMapData::CMapData(IDataStream* pData) :isValid(false)
	{
		if(!pData->IsValid())
			return;

		ChunkHeader cheader;
		ADT_MVER mver;
		ADT_MHDR mhdr;
		MapTile* tiles = nullptr;
		bool bLoadFail = false;
		do
		{
			//read chunk header
			if(pData->Read(sizeof(cheader),(u8*)&cheader)!=sizeof(cheader))
				break;
			switch(cheader.fourcc_code)
			{
			case ADT_MVERMagic:
				{
					forceRead(pData,sizeof(ADT_MVER),&mver);
					assert(cheader.size == sizeof(ADT_MVER));
					printf("MVer Chunk version: %08x\n",mver.version);
				}
				break;
			case ADT_MHDRMagic:
				{
					forceRead(pData,sizeof(ADT_MHDR),&mhdr);
					assert(cheader.size == sizeof(ADT_MHDR));
				}
				break;
			case ADT_MCNKMagic:
				{
					if(!tiles)
						tiles = new MapTile[16*16]; 
					ADT_MCNK mcnk;
					MapTile tile;
					forceRead(pData,sizeof(ADT_MCNK),&mcnk);
					u32 remaining_size = cheader.size-sizeof(ADT_MCNK);
					ChunkHeader sub_header;
					Vector3<f32> position = fixCoordSystem(mcnk.position);
					
					for(int i = 0;i<9*9+8*8;++i)
						tile.VertexColors[i] = Vector3<f32>(1.0f,1.0f,1.0f);

					ADT_MCLY layers[4];
					u32	nLayers;
					while(remaining_size)
					{
						forceRead(pData,sizeof(ChunkHeader),&sub_header);
						remaining_size -= sub_header.size + sizeof(ChunkHeader);
						switch(sub_header.fourcc_code)
						{
						case ADT_MCVTMagic:
							{
								f32 buffer[nTileVertices];
								forceRead(pData,sizeof(f32)*(nTileVertices),buffer);
								// edges
								for(int iy = 0;iy<nTileYCells+1;++iy)
									for(int ix = 0;ix<nTileXCells+1;++ix)
									{
										tile.Positions[iy*(2*nTileXCells+1)+ix]=
											Vector3<f32>(
												-fADT_ChunkXSize/2.0f + (f32)mcnk.indexX*fADT_TileXSize+ix*fADT_CellXSize,
												(buffer[iy*(2*nTileXCells+1)+ix]+position.y)/500.0f-1.5f,
												-fADT_ChunkZSize/2.0f + (f32)mcnk.indexY*fADT_TileZSize+iy*fADT_CellZSize
												);
									}
								// centers
								for(int iy = 0;iy<nTileYCells;++iy)
									for(int ix = 0;ix<nTileXCells;++ix)
									{
										tile.Positions[iy*(2*nTileXCells+1)+(nTileXCells+1)+ix]=
											Vector3<f32>(
												-fADT_ChunkXSize/2.0f + (f32)mcnk.indexX*fADT_TileXSize+ix*fADT_CellXSize+fADT_CellXSize/2.0f,
												(buffer[iy*(2*nTileXCells+1)+(nTileXCells+1)+ix]+position.y)/500.0f-1.5f,
												-fADT_ChunkZSize/2.0f + (f32)mcnk.indexY*fADT_TileZSize+iy*fADT_CellZSize+fADT_CellXSize/2.0f
												);
									}
							}
							break;
						case ADT_MCNRMagic:
							{
								Vector3<i8> buffer[nTileVertices];
								forceRead(pData,sizeof(Vector3<i8>)*(nTileVertices),buffer);
								// skip unknown data
								pData->Seek(13,IDataStream::ORIGIN_CURRENT);
								
								for(int i = 0;i<nTileVertices;++i)
									tile.Normals[i] = fixCoordSystem(Vector3<f32>((f32)buffer[i].x /127.0f,(f32)buffer[i].y /127.0f,(f32)buffer[i].z /127.0f)).normalize();
								
							}
							break;
						case ADT_MCSEMagic:
							// not interested in sound emitters
							pData->Seek(sub_header.size,IDataStream::ORIGIN_CURRENT);
							break;
						case ADT_MCCVMagic:
							{			
								Vector4<u8> buffer[9*9+8*8];
								int read = sizeof(Vector4<u8>)*(9*9+8*8);
								forceRead(pData,sizeof(Vector4<u8>)*(9*9+8*8),buffer);
								for(int i = 0;i<9*9+8*8;++i)
									tile.VertexColors[i] = Vector3<f32>((f32)buffer[i].x /127.0f,(f32)buffer[i].y /127.0f,(f32)buffer[i].z /127.0f);
							}
							break;
						case ADT_MCLVMagic:
							{
								// unknown so skip silently
								pData->Seek(sub_header.size,IDataStream::ORIGIN_CURRENT);
							}
							break;
						case ADT_MCRFMagic:
						case ADT_MCSHMagic:
						case ADT_MCALMagic:
							// skip these currently
							printf("%c%c%c%c in MCNK\n",sub_header.fourcc[3],sub_header.fourcc[2],sub_header.fourcc[1],sub_header.fourcc[0]);
							pData->Seek(sub_header.size,IDataStream::ORIGIN_CURRENT);
							break;
						default:
							printf("Unknown chunk: %c%c%c%c in MCNK\n",sub_header.fourcc[3],sub_header.fourcc[2],sub_header.fourcc[1],sub_header.fourcc[0]);
							pData->Seek(sub_header.size,IDataStream::ORIGIN_CURRENT);
							break;
						}
					}

					tiles[mcnk.indexY*nXTiles+mcnk.indexX] = tile;
				}
				break;
			case ADT_MH2OMagic:
			case ADT_MFBOMagic:
				pData->Seek(cheader.size,IDataStream::ORIGIN_CURRENT);
				break;
			default:
				pData->Seek(cheader.size,IDataStream::ORIGIN_CURRENT);
				bLoadFail = true;
				break;
			}
		}
		while(!bLoadFail);

		if(bLoadFail)
		{
			if(tiles)
				delete [] tiles;
			return;
		}

		//assemble tiles

		for(int iy = 0; iy<nYTiles;iy++)
			for(int ix = 0; ix<nXTiles;ix++)
			{
				const u32 nTileOffset = iy*nXTiles+ix;
				const u32 nTileSize = nTileVertices*sizeof(Vector3<>);
				memcpy(&_positionData[nTileOffset*nTileVertices],tiles[nTileOffset].Positions,nTileSize);
				memcpy(&_normalData[nTileOffset*nTileVertices],tiles[nTileOffset].Normals,nTileSize);
				memcpy(&_vertexColorData[nTileOffset*nTileVertices],tiles[nTileOffset].VertexColors,nTileSize);

				//generate index data
				for(int ii = 0; ii < nTileIndices; ii++)
				{
					_indexData[nTileOffset*nTileIndices+ii] = TileIndex[ii]+nTileOffset*nTileVertices;
				}

				//generate texcoord data
				const u32 nRowStride = nTileXCells + (nTileXCells +1);
				const u32 nCenterOffset = (nTileXCells +1);
				const f32 texTileSizeX = 1.0f/(f32)nXTiles;
				const f32 texTileSizeY = 1.0f/(f32)nYTiles;
				const f32 texCellSizeX = texTileSizeX/(f32)nTileXCells;
				const f32 texCellSizeY = texTileSizeY/(f32)nTileYCells;
				// edges
				for(int iyt = 0; iyt < nTileYCells + 1 ; ++ iyt)
					for(int ixt = 0; ixt < nTileXCells + 1 ; ++ ixt)
					{
						_texcoordData[nTileOffset*nTileVertices + iyt*nRowStride + ixt].x = (f32)ix*texTileSizeX+(f32)ixt*texCellSizeX;
						_texcoordData[nTileOffset*nTileVertices + iyt*nRowStride + ixt].y = (f32)iy*texTileSizeX+(f32)iyt*texCellSizeX;
					}
				// centers
				for(int iyt = 0; iyt < nTileYCells ; ++ iyt)
					for(int ixt = 0; ixt < nTileXCells ; ++ ixt)
					{
						_texcoordData[nTileOffset*nTileVertices + iyt*nRowStride + nCenterOffset + ixt].x = (f32)ix*texTileSizeX+(f32)ixt*texCellSizeX+0.5f*texCellSizeX;
						_texcoordData[nTileOffset*nTileVertices + iyt*nRowStride + nCenterOffset + ixt].y = (f32)iy*texTileSizeX+(f32)iyt*texCellSizeX+0.5f*texCellSizeY;
					}
			}
		delete [] tiles;


		isValid = true;
	}

	CMapDataLoader::CMapData::~CMapData()
	{
	}

	IResource* CMapDataLoader::CMapData::Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
	{
		return (IResource*)this;
	}

	u32 CMapDataLoader::CMapData::GetNumVertices() const
	{
		return nVertices;
	}

	u32 CMapDataLoader::CMapData::GetVertexData(const u8** pDataOut,u32& pOutStride,VERTEX_DATA_TYPE type) const
	{
		switch(type)
		{
		case POSITION:
			*pDataOut = (const u8*)_positionData.data();
			pOutStride = sizeof(Vector3<>);
			return nVertices;
			break;
		case TEXCOORD0:
			*pDataOut = (const u8*)_texcoordData.data();
			pOutStride = sizeof(Vector2<>);
			return nVertices;
			break;
		case NORMAL:
			*pDataOut = (const u8*)_normalData.data();
			pOutStride = sizeof(Vector3<>);
			return nVertices;
			break;
		case COLOR:
			*pDataOut = (const u8*)_vertexColorData.data();
			pOutStride = sizeof(Vector3<>);
			return nVertices;
			break;
		default:
			return 0;
		}

		pOutStride = 0;
	}
	u32	CMapDataLoader::CMapData::GetNumIndices() const
	{
		return nIndices;
	}
	u32 CMapDataLoader::CMapData::GetIndexData(const u8** pDataOut) const
	{
		if(pDataOut)
		{
			*pDataOut = (const u8*)_indexData.data();
			return nIndices;
		}
		else
		{
			return nIndices;
		}
	}
	const char* CMapDataLoader::CMapData::GetTextureName(u32 index) const
	{
		return nullptr;
	}

	const void* CMapDataLoader::CMapData::GetExtendedData(EXTENDED_DATA_TYPE type) const
	{
		return nullptr;
	}

	template<int x_cells,int y_cells> std::array<u32,x_cells*y_cells*4*3> GenerateTileIndices()
	{
		std::array<u32,x_cells*y_cells*4*3> result;
		u32 row_stride = x_cells+(x_cells+1);
		u32 center_offset = x_cells+1;
		
		for(int iy = 0; iy<y_cells;iy++)
			for(int ix = 0; ix<x_cells;ix++)
			{
				const u32 top_left_vertex = iy*row_stride + ix;
				const u32 top_right_vertex = iy*row_stride + ix + 1;
				const u32 center_vertex = iy*row_stride + center_offset + ix;
				const u32 bottom_left_vertex = (iy+1)*row_stride + ix;
				const u32 bottom_right_vertex = (iy+1)*row_stride + ix + 1;

				result[((iy*x_cells+ix)*4+0)*3+0] = center_vertex;
				result[((iy*x_cells+ix)*4+0)*3+1] = top_right_vertex;
				result[((iy*x_cells+ix)*4+0)*3+2] = top_left_vertex;

				result[((iy*x_cells+ix)*4+1)*3+0] = bottom_right_vertex;
				result[((iy*x_cells+ix)*4+1)*3+1] = top_right_vertex;
				result[((iy*x_cells+ix)*4+1)*3+2] = center_vertex;

				result[((iy*x_cells+ix)*4+2)*3+0] = center_vertex;
				result[((iy*x_cells+ix)*4+2)*3+1] = bottom_left_vertex;
				result[((iy*x_cells+ix)*4+2)*3+2] = bottom_right_vertex;

				result[((iy*x_cells+ix)*4+3)*3+0] = top_left_vertex;
				result[((iy*x_cells+ix)*4+3)*3+1] = bottom_left_vertex;
				result[((iy*x_cells+ix)*4+3)*3+2] = center_vertex;
			}

		return result;
	}

	const std::array<u32,CMapDataLoader::CMapData::nTileIndices> CMapDataLoader::CMapData::TileIndex = GenerateTileIndices<nTileXCells,nTileYCells>();

}