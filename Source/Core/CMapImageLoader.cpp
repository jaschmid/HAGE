#include "HAGE.h"

#include <stdio.h>

#include "CMapLoader.h"

namespace HAGE
{
	IResourceLoader* CMapDataImageLoader::Initialize(IDataStream* pStream,const IResource* pPrev)
	{
		CMapDataImageLoader* pLoader = new CMapDataImageLoader(pStream,pPrev);
		if(!pLoader->IsValid())
		{
			delete pLoader;
			pLoader = nullptr;
		}
		return pLoader;
	}

	CMapDataImageLoader::CMapDataImageLoader(IDataStream* pStream,const IResource* pPrev)
		: _pData (pStream)
	{
		_pMapData = new CMapDataImage(pStream);
		if(!_pMapData->IsValid())
		{
			delete _pMapData;
			_pMapData = nullptr;
		}
	}
	CMapDataImageLoader::~CMapDataImageLoader()
	{
	}

	IResource* CMapDataImageLoader::Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
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

		
	CMapDataImageLoader::CMapDataImage::~CMapDataImage()
	{
		if(_Data)
			delete [] _Data;
	}
	
	CMapDataImageLoader::CMapDataImage::CMapDataImage(IDataStream* pData) 
		:isValid(false),pDependancies(nullptr),_pTiles(nullptr),_Data(nullptr)
	{
		if(!pData->IsValid())
			return;

		ChunkHeader cheader;
		ADT_MVER mver;
		bool bVerRead = false;
		bool bLoadFail = false;
		u32 tile_index =0;

		do
		{
			//read chunk header
			if(pData->Read(sizeof(cheader),(u8*)&cheader)!=sizeof(cheader))
				break;

			if(!bVerRead)
			{
				if(cheader.fourcc_code != ADT_MVERMagic)
					return;
				forceRead(pData,sizeof(ADT_MVER),&mver);
				printf("MVer Chunk version: %08x\n",mver.version);
				bVerRead = true;

				continue;
			}

			switch(cheader.fourcc_code)
			{
			//don't know what this does
			case ADT_MAMPMagic:
				pData->Seek(cheader.size,IDataStream::ORIGIN_CURRENT);
				break;
			case ADT_MTXFMagic:
				{
					u32* temp = new u32[cheader.size/4];
					forceRead(pData,cheader.size,temp);
					for(int i = 0 ; i< _texNames.size();++i)
						if(temp[i])
							_texNames[i] = "Null";
					delete [] temp;
				}
				//pData->Seek(cheader.size,IDataStream::ORIGIN_CURRENT);
				break;
			case ADT_MTEXMagic:
				{
					char* pFilenames=nullptr;
					pFilenames = new char[cheader.size];
					forceRead(pData,cheader.size,pFilenames);
					int i = 0;
					while(i<cheader.size)
					{
						if(pFilenames[i])
						{
							_texNames.push_back(std::string("@art.mpq\\")+std::string(&pFilenames[i]));
							i+=strlen(&pFilenames[i]);
						}
						else
							++i;
					}
					delete [] pFilenames;
				}
				break;
			case ADT_MCNKMagic:
				{
					if(!_pTiles)
						_pTiles = new MapTile[nXTiles*nYTiles];
					MapTile tile;
					memset(&tile,0,sizeof(MapTile));
					//forceRead(pData,sizeof(ADT_MCNK),&tile.mcnk);
					u32 remaining_size = cheader.size;
					ChunkHeader sub_header;
					while(remaining_size && ! bLoadFail)
					{
						forceRead(pData,sizeof(ChunkHeader),&sub_header);
						remaining_size -= sub_header.size + sizeof(ChunkHeader);
						switch(sub_header.fourcc_code)
						{
						case ADT_MCLYMagic:
							{
								assert(sub_header.size <= sizeof(ADT_MCLY)*4);
								tile.nLayers = sub_header.size / sizeof(ADT_MCLY);
								forceRead(pData,sub_header.size,tile.layers);
							}
							break;
						case ADT_MCALMagic:
							{
								forceRead(pData,sub_header.size,tile.AlphaMap);
								tile.AlphaBytes = sub_header.size;
							}
							break;
						case ADT_MCSHMagic:
							{
								forceRead(pData,(nTileXCells*8)*(nTileYCells*8)/8,tile.ShadowMap);
							}
							break;
						case ADT_MCMTMagic:
							// skip this chunk unknown
							pData->Seek(sub_header.size,IDataStream::ORIGIN_CURRENT);
							break;
						default:
							printf("Unknown chunk: %c%c%c%c in MCNK\n",sub_header.fourcc[3],sub_header.fourcc[2],sub_header.fourcc[1],sub_header.fourcc[0]);
							pData->Seek(sub_header.size,IDataStream::ORIGIN_CURRENT);
							bLoadFail = true;
							break;
						}
					}

					_pTiles[tile_index] = tile;

					u32 outputLocation = 0;

					for(int i = 0;i<tile.nLayers;i++)
					{
						if(tile.layers[i].flags & ADT_MCLY::FLAG_ALPHA_MAP)
						{
							u32 nLayerSize = 0;
							if(i<tile.nLayers - 1)
								nLayerSize = tile.layers[i+1].offsetInMCAL -  tile.layers[i].offsetInMCAL;
							else
								nLayerSize = tile.AlphaBytes - tile.layers[i].offsetInMCAL;

							if(tile.layers[i].flags & ADT_MCLY::FLAG_COMPRESSED_LAYER)
							{
								//compressed
								// 21-10-2008 by Flow
								unsigned offO = 0;
								unsigned offI = 0; //offset IN buffer
								u8* buffIn = &tile.AlphaMap[tile.layers[i].offsetInMCAL]; ; // pointer to data in adt file
								u8* buffOut = &_pTiles[tile_index].AlphaMap[outputLocation]; // the resulting alpha map

								while( offO < 4096 )
								{
									// fill or copy mode
									bool fill = buffIn[offI] & 0x80;
									unsigned n = buffIn[offI] & 0x7F;
									offI++;
									for( unsigned k = 0; k < n; k++ )
									{
										if (offO == 4096) break;
										buffOut[offO] = buffIn[offI];
										offO++;
										if( !fill )
										offI++;
									}
									if( fill ) offI++;
								}

								outputLocation += 4096;
							}
							else if(nLayerSize == 4096)
							{
								//uncompressed just copy
								memcpy(&_pTiles[tile_index].AlphaMap[outputLocation],&tile.AlphaMap[tile.layers[i].offsetInMCAL],4096);
								outputLocation += 4096;
							}
							else if(nLayerSize == 2048)
							{
								//half byte compressed
								unsigned offI = 0; //offset IN buffer
								u8* buffIn = &tile.AlphaMap[tile.layers[i].offsetInMCAL]; ; // pointer to data in adt file
								for(int i = 0;i<2096;i++)
								{
									_pTiles[tile_index].AlphaMap[outputLocation +1]= (buffIn[offI]&0xf0 >> 4)*16; 
									_pTiles[tile_index].AlphaMap[outputLocation +0]= (buffIn[offI]&0x0f)*16;
									outputLocation += 2;
									offI ++;
								}
							}
							else
								assert(!"Unknown Alpha Data");
						}
					}

					assert(outputLocation == (tile.nLayers-1)*4096);
					++tile_index;
				}
				break;
			default:
				printf("Unknown chunk: %c%c%c%c in texture ADT\n",cheader.fourcc[3],cheader.fourcc[2],cheader.fourcc[1],cheader.fourcc[0]);
				pData->Seek(cheader.size,IDataStream::ORIGIN_CURRENT);
				bLoadFail = true;
				break;
			}
		}
		while(!bLoadFail);

		assert(tile_index == 256);

		if(bLoadFail)
		{
			if(_pTiles)
				delete [] _pTiles;
			return;
		}

		isValid = true;
	}
	
	// unsigned long PackRGBA(): Helper method that packs RGBA channels into a single 4 byte pixel.
	//
	// unsigned char r:		red channel.
	// unsigned char g:		green channel.
	// unsigned char b:		blue channel.
	// unsigned char a:		alpha channel.
 
	unsigned long PackRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		return ((r << 24) | (g << 16) | (b << 8) | a);
	}
 
	// void DecompressBlockDXT1(): Decompresses one block of a DXT1 texture and stores the resulting pixels at the appropriate offset in 'image'.
	//
	// unsigned long x:						x-coordinate of the first pixel in the block.
	// unsigned long y:						y-coordinate of the first pixel in the block.
	// unsigned long width: 				width of the texture being decompressed.
	// unsigned long height:				height of the texture being decompressed.
	// const unsigned char *blockStorage:	pointer to the block to decompress.
	// unsigned long *image:				pointer to image where the decompressed pixel data should be stored.
 
	void DecompressBlockDXT1(unsigned long x, unsigned long y, unsigned long width, const unsigned char *blockStorage, unsigned long *image)
	{
		unsigned short color0 = *reinterpret_cast<const unsigned short *>(blockStorage);
		unsigned short color1 = *reinterpret_cast<const unsigned short *>(blockStorage + 2);
 
		unsigned long temp;
 
		temp = (color0 >> 11) * 255 + 16;
		unsigned char r0 = (unsigned char)((temp/32 + temp)/32);
		temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
		unsigned char g0 = (unsigned char)((temp/64 + temp)/64);
		temp = (color0 & 0x001F) * 255 + 16;
		unsigned char b0 = (unsigned char)((temp/32 + temp)/32);
 
		temp = (color1 >> 11) * 255 + 16;
		unsigned char r1 = (unsigned char)((temp/32 + temp)/32);
		temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
		unsigned char g1 = (unsigned char)((temp/64 + temp)/64);
		temp = (color1 & 0x001F) * 255 + 16;
		unsigned char b1 = (unsigned char)((temp/32 + temp)/32);
 
		unsigned long code = *reinterpret_cast<const unsigned long *>(blockStorage + 4);
 
		for (int j=0; j < 4; j++)
		{
			for (int i=0; i < 4; i++)
			{
				unsigned long finalColor = 0;
				unsigned char positionCode = (code >>  2*(4*j+i)) & 0x03;
 
				if (color0 > color1)
				{
					switch (positionCode)
					{
						case 0:
							finalColor = PackRGBA(r0, g0, b0, 255);
							break;
						case 1:
							finalColor = PackRGBA(r1, g1, b1, 255);
							break;
						case 2:
							finalColor = PackRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, 255);
							break;
						case 3:
							finalColor = PackRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, 255);
							break;
					}
				}
				else
				{
					switch (positionCode)
					{
						case 0:
							finalColor = PackRGBA(r0, g0, b0, 255);
							break;
						case 1:
							finalColor = PackRGBA(r1, g1, b1, 255);
							break;
						case 2:
							finalColor = PackRGBA((r0+r1)/2, (g0+g1)/2, (b0+b1)/2, 255);
							break;
						case 3:
							finalColor = PackRGBA(0, 0, 0, 255);
							break;
					}
				}
 
				if (x + i < width)
					image[(y + j)*width + (x + i)] = finalColor;
			}
		}
	}
 
	// void BlockDecompressImageDXT1(): Decompresses all the blocks of a DXT1 compressed texture and stores the resulting pixels in 'image'.
	//
	// unsigned long width:					Texture width.
	// unsigned long height:				Texture height.
	// const unsigned char *blockStorage:	pointer to compressed DXT1 blocks.
	// unsigned long *image:				pointer to the image where the decompressed pixels will be stored.
 
	void BlockDecompressImageDXT1(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned long *image)
	{
		unsigned long blockCountX = (width + 3) / 4;
		unsigned long blockCountY = (height + 3) / 4;
		unsigned long blockWidth = (width < 4) ? width : 4;
		unsigned long blockHeight = (height < 4) ? height : 4;
 
		for (unsigned long j = 0; j < blockCountY; j++)
		{
			for (unsigned long i = 0; i < blockCountX; i++) DecompressBlockDXT1(i*4, j*4, width, blockStorage + i * 8, image);
			blockStorage += blockCountX * 8;
		}
	}

	const int max_dependancies = 0xffff;

	IResource* CMapDataImageLoader::CMapDataImage::Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
	{
		if(dependanciesIn == nullptr )
		{
			assert(_Data == nullptr);
			// need to load files
			if(_texNames.size() > max_dependancies)
				_texNames.resize(max_dependancies);
			pDependancies = new std::pair<std::string,guid>[_texNames.size()];
			for(int i = 0;i<_texNames.size();++i)
			{
				pDependancies[i].first = _texNames[i];
				pDependancies[i].second = guid_of<IImageData>::Get();
			}
			*pDependanciesOut = pDependancies;
			nDependanciesInOut = _texNames.size();
			return nullptr;
		}
		else
		{
			assert(nDependanciesInOut == _texNames.size());
			//finish loading

			//get largest dependancy size
			u32 largest_source_width = 0;
			u32 largest_source_height = 0;

			u32* source_widths = new u32[nDependanciesInOut];
			u32* source_heights = new u32[nDependanciesInOut];
			u32** source_images = new u32*[nDependanciesInOut];

			for(int i =0;i<nDependanciesInOut;++i)
			{
				TResourceAccess<IImageData> dep = dependanciesIn[i];
				u32 w=dep->GetImageWidth();
				u32 h=dep->GetImageHeight();
				if(w>largest_source_width);
					largest_source_width = w;
				if(h>largest_source_height);
					largest_source_height = h;
				source_widths[i] = w;
				source_heights[i] = h;
				source_images[i] = new u32[w*h];
				if(dep->GetImageFormat() == IImageData::DXTC1)
				{
					BlockDecompressImageDXT1(w,h,(u8*)dep->GetImageData(),source_images[i]);
				}
				else if(dep->GetImageFormat() == IImageData::R8G8B8A8)
				{
					memcpy(source_images[i],dep->GetImageData(),w*h*sizeof(u32));
				}
				else
				{
					printf("Unknown Format for source image %s",_texNames[i].c_str());
					memset(source_images[i],0xff,w*h*sizeof(u32));
				}
			}

			_Width = largest_source_width*nXTiles;
			_Height = largest_source_height*nYTiles;

			_Levels =1;
			_Format = IImageData::R8G8B8A8;

			_Data = new u32[_Width*_Height];

			for(int ix = 0;ix<_Width;ix++)
				for(int iy=0;iy<_Height;iy++)
				{
					int ixTile = ix/largest_source_width;
					int ixCell = ix%largest_source_width*nTileXCells/largest_source_width;
					int ixAlpha = ix%largest_source_width*nTileXAlphas/largest_source_width;
					int iyTile = iy/largest_source_height;
					int iyCell = iy%largest_source_height*nTileYCells/largest_source_height;
					int iyAlpha = iy%largest_source_height*nTileYAlphas/largest_source_height;
					
					int iTile = iyTile*nXTiles+ixTile;

					u32 BlendWeights[4];
					
					u32 base = 255;

					for(int iLayer = 1;iLayer<_pTiles[iTile].nLayers;++iLayer)
					{
						BlendWeights[iLayer] = _pTiles[iTile].AlphaMap[iyAlpha*nTileXAlphas+ixAlpha+(iLayer-1)*4096];
						base -= BlendWeights[iLayer];
					}

					BlendWeights[0] = base;

					u32 red=0,blue=0,green=0,alpha=0;

					for(int iLayer = 0;iLayer<_pTiles[iTile].nLayers;++iLayer)
					{
						int iTexture = _pTiles[iTile].layers[iLayer].textureId;

						if(iTexture>=max_dependancies)
							iTexture = max_dependancies-1;

						int iTextureWidth = source_widths[iTexture];
						int iTextureHeight = source_heights[iTexture];
						int ixPixel = ix%largest_source_width*iTextureWidth/largest_source_width;
						int iyPixel = iy%largest_source_height*iTextureHeight/largest_source_height;

						u32 color = source_images[iTexture][iyPixel*iTextureWidth+ixPixel];
						red += ((color&0xff000000)>>24) * BlendWeights[iLayer];
						blue += ((color&0x00ff0000)>>16) * BlendWeights[iLayer];
						green += ((color&0x0000ff00)>>8) * BlendWeights[iLayer];
						alpha += ((color&0x000000ff)>>0) * BlendWeights[iLayer];
					}

					red/=255;
					blue/=255;
					green/=255;
					alpha/=255;

					((u32*)_Data)[iy*_Width+ix] = (red<<0) | (blue <<8) | (green<<16) | (alpha<<24);
				}
				
			for(int i =0;i<nDependanciesInOut;++i)
			{
				delete [] source_images[i];
			}
			delete [] source_widths;
			delete [] source_heights;
			delete [] source_images;
			delete [] pDependancies;
			delete [] _pTiles;
			pDependancies=nullptr;
			_pTiles=nullptr;
			return (IResource*)this;
		}
	}

	const void* CMapDataImageLoader::CMapDataImage::GetImageData() const
	{
		return _Data;
	}

	u32 CMapDataImageLoader::CMapDataImage::GetImageFormat() const
	{
		return _Format;
	}

	u32 CMapDataImageLoader::CMapDataImage::GetImageHeight() const
	{
		return _Height;
	}

	u32 CMapDataImageLoader::CMapDataImage::GetImageWidth() const
	{
		return _Width;
	}

	u32 CMapDataImageLoader::CMapDataImage::GetImageLevels() const
	{
		return _Levels;
	}
}