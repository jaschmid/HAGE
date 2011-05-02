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
		_pMapData = new CMapData;
		if(!_pMapData->LoadInit(pStream,dependancies))
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
		if(dependanciesIn == nullptr)
		{
			if(dependancies.size() != 0)
			{
				*pDependanciesOut = dependancies.data();
				nDependanciesInOut = dependancies.size();
				return nullptr;
			}
			else
				return static_cast<IResource*>(_pMapData);
		}
		else
		{
			_pMapData->LoadStep(dependanciesIn,dependancies);
			
			if(dependancies.size() != 0)
			{
				*pDependanciesOut = dependancies.data();
				nDependanciesInOut = dependancies.size();
				return nullptr;
			}
			else
				return static_cast<IResource*>(_pMapData);
		}
	}
	
	inline static void forceRead(IDataStream* pData,u64 size,void* pOut)
	{
		assert(pData->Read(size,(u8*)pOut) == size);
	}

	inline static Vector3<> fixCoordSystem(Vector3<> v)
	{
			return Vector3<>(v.x, v.z, -v.y);
	}
	
	CMapDataLoader::CMapData::CMapData() :isValid(false)
	{
	}
	
	template<class _C> static bool getFromData(const _C*& target,u64& pos,const u8* data,const u64& size)
	{
		if(pos+sizeof(_C) > size)
		{
			target = nullptr;
			return false;
		}
		else
		{
			target = reinterpret_cast<const _C*>(&data[pos]);
			pos+=sizeof(_C);
			return true;
		}
	}

	bool CMapDataLoader::CMapData::LoadObj(const TResourceAccess<IRawData>& data,std::vector<std::pair<std::string,guid>>& dependanciesOut)
	{
		const u8* pData;
		u64 size = data->GetData(&pData);
		u64 pos = 0;

		const static bool doNotLoadM2 = true;
		
		std::vector<char> m2FilenamesData;
		std::vector<char> wmoFilenamesData;
		std::vector<char*> m2Filenames;
		std::vector<char*> wmoFilenames;

		const ChunkHeader* cheader;
		const ADT_MVER* mver;
		bool bLoadFail = false;
		do
		{
			if(!getFromData(cheader,pos,pData,size))
				break;
			switch(cheader->fourcc_code)
			{
			case ADT_MVERMagic:
				{
					if(!getFromData(mver,pos,pData,size))
					{
						bLoadFail = true;
						break;
					}
					assert(cheader->size == sizeof(ADT_MVER));
					printf("MVer_Obj Chunk version: %08x\n",mver->version);
				}
				break;
			case ADT_MMDXMagic:
				{
					m2FilenamesData.resize(cheader->size);
					memcpy(m2FilenamesData.data(),&pData[pos],cheader->size);
					pos += cheader->size;
				}
				break;
			case ADT_MMIDMagic:
				{
					m2Filenames.resize(cheader->size / sizeof(u32) );
					for(int i = 0; i < m2Filenames.size(); ++i)
					{
						const u32* loc;
						if(!getFromData(loc,pos,pData,size))
						{
							bLoadFail = true;
							break;
						}
						m2Filenames[i] = &m2FilenamesData[*loc];
					}
				}
				break;
			case ADT_MWMOMagic:
				{
					wmoFilenamesData.resize(cheader->size);
					memcpy(wmoFilenamesData.data(),&pData[pos],cheader->size);
					pos += cheader->size;
				}
				break;
			case ADT_MWIDMagic:
				{
					wmoFilenames.resize(cheader->size / sizeof(u32) );
					for(int i = 0; i < wmoFilenames.size(); ++i)
					{
						const u32* loc;
						if(!getFromData(loc,pos,pData,size))
						{
							bLoadFail = true;
							break;
						}
						wmoFilenames[i] = &wmoFilenamesData[*loc];
					}
				}
				break;
			case ADT_MDDFMagic:
				{
					u32 nM2Models = cheader->size / sizeof(ADT_MDDF_Entry);
					const ADT_MDDF_Entry* pEntries = (const ADT_MDDF_Entry*)&pData[pos];
					if(pos + nM2Models*sizeof(ADT_MDDF_Entry) > size)
					{
						bLoadFail = true;
						break;
					}
					else
						pos += nM2Models*sizeof(ADT_MDDF_Entry);

					if(!doNotLoadM2)
						for(int i = 0; i < nM2Models; ++i)
						{
							ChildObject o;
							const ADT_MDDF_Entry& current = pEntries[i];
							f32 scaleFactor = ((f32)pEntries[i].scale)/1024.0f;
							Vector3<> pos = Vector3<>(-(pEntries[i].pos.z - 17066.6666666f) , pEntries[i].pos.y, (pEntries[i].pos.x - 17066.6666666f));
							o.transformation = Matrix4<>::Translate( pos ) * Matrix4<>::Scale( Vector3<>(scaleFactor,scaleFactor,scaleFactor));
							_childObjects.push_back(o);
							std::string name = std::string("@art.mpq\\") +std::string(m2Filenames[ pEntries[i].nameId ] );
							dependanciesOut.push_back( std::make_pair( name, guid_of<IMeshData>::Get() ) );
						}
				}
				break;
			case ADT_MODFMagic:
				{
					u32 nWMOModels = cheader->size / sizeof(ADT_MODF_Entry);
					const ADT_MODF_Entry* pEntries = (const ADT_MODF_Entry*)&pData[pos];
					if(pos + nWMOModels*sizeof(ADT_MODF_Entry) > size)
					{
						bLoadFail = true;
						break;
					}
					else
						pos += nWMOModels*sizeof(ADT_MODF_Entry);

					for(int i = 0; i < nWMOModels; ++i)
					{
						ChildObject o;
						const ADT_MODF_Entry& current = pEntries[i];
						f32 scaleFactor = 1.0f;
						Matrix4<> rotation = Matrix4<>::AngleRotation(Vector3<>(0.0f,1.0f,0.0f),current.rot.y / 360.0f * 2* 3.14159265);
						Vector3<> pos = Vector3<>(-(pEntries[i].pos.z - 17066.6666666f) , pEntries[i].pos.y, (pEntries[i].pos.x - 17066.6666666f));
						o.transformation = Matrix4<>::Translate( pos ) * (rotation*Matrix4<>::Scale( Vector3<>(scaleFactor,scaleFactor,scaleFactor)));
						_childObjects.push_back(o);
						std::string name = std::string("@world.mpq\\") +std::string(wmoFilenames[ pEntries[i].nameId ] );
						dependanciesOut.push_back( std::make_pair( name, guid_of<IMeshData>::Get() ) );
					}
				}
				break;
			case ADT_MCNKMagic:
				pos += cheader->size;
				break;
			}
		}
		while(!bLoadFail);

		if(bLoadFail)
			return false;

		//
		// debug
		//

		printf("Obj dependancies:\n");
		for(int i = 0; i < dependanciesOut.size(); ++i)
			printf("\t%s",dependanciesOut[i].first.c_str());

		return true;
	}

	bool CMapDataLoader::CMapData::LoadStep(const ResourceAccess* dependanciesIn,std::vector<std::pair<std::string,guid>>& dependanciesInOut)
	{
		if(dependanciesInOut[0].second == guid_of<IRawData>::Get())
		{
			dependanciesInOut.clear();
			_texture = TResourceAccess<IImageData>(dependanciesIn[1]);
			return LoadObj(TResourceAccess<IRawData>(dependanciesIn[0]),dependanciesInOut);
		}
		else if(dependanciesInOut[0].second == guid_of<IMeshData>::Get())
		{
			for(int i = 0; i < dependanciesInOut.size(); ++i)
				_childObjects[i].childMesh = TResourceAccess<IMeshData>(dependanciesIn[i]);
			dependanciesInOut.clear();
			_nChildObjects = _childObjects.size();
			return true;
		}
		else
			assert(!"recieved unexpected dependancy data");
		return true;
	}

	bool CMapDataLoader::CMapData::LoadInit(IDataStream* pData,std::vector<std::pair<std::string,guid>>& dependanciesOut)
	{
		if(!pData->IsValid())
			return false;

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
												position.x +(nTileYCells-iy)*fADT_CellXSize,
												buffer[iy*(2*nTileXCells+1)+ix]+position.y,
												position.z +ix*fADT_CellZSize
												);
									}
								// centers
								for(int iy = 0;iy<nTileYCells;++iy)
									for(int ix = 0;ix<nTileXCells;++ix)
									{
										tile.Positions[iy*(2*nTileXCells+1)+(nTileXCells+1)+ix]=
											Vector3<f32>(
												position.x +(nTileYCells-iy-1)*fADT_CellXSize+fADT_CellXSize/2.0f,
												buffer[iy*(2*nTileXCells+1)+(nTileXCells+1)+ix]+position.y,
												position.z +ix*fADT_CellZSize+fADT_CellZSize/2.0f
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
			return false;
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

		//generate our dependancy

		const std::string& adtName = pData->GetIdentifierString();

		std::string objName = adtName.substr(0,adtName.length()-4) + std::string("_obj0.adt");

		dependanciesOut.push_back(std::make_pair(objName,guid_of<IRawData>::Get()));

		std::string texName = adtName.substr(0,adtName.length()-4) + std::string("_tex0.adt");

		dependanciesOut.push_back(std::make_pair(texName,guid_of<IImageData>::Get()));

		_textureName = texName;

		isValid = true;

		return true;
	}

	CMapDataLoader::CMapData::~CMapData()
	{
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
		if(index == 0)
			return _textureName.c_str();
		else
			return nullptr;
	}
	const TResourceAccess<IImageData>* CMapDataLoader::CMapData::GetTexture(u32 index) const
	{
		if(index == 0)
			return &_texture;
		else
			return nullptr;
	}
	
	const void* CMapDataLoader::CMapData::GetExtendedData(EXTENDED_DATA_TYPE type) const
	{
		if(type == CHILD_COUNT)
			return &_nChildObjects;
		else if(type == CHILD_LIST)
			return _childObjects.data();
		else
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