#include <HAGE.h>
#include <stdio.h>

#include "dependancies/Deflate/deflate.h"
#include "dependancies/Snappy/snappy.h"
#include "dependancies/LZMA/LzmaEnc.h"
#include "dependancies/LZMA/LzmaDec.h"

namespace HAGE
{

	SVTPage::SVTPage(u32 pageSize) : _pageSize(pageSize),_layerMask(0)
	{
	}

	SVTPage::~SVTPage()
	{
		reset();
	}

	u32 SVTPage::LayerMask()
	{
		return _layerMask;
	}
	
	u32 SVTPage::getLayerIndex(u32 Layer)
	{
		switch(Layer)
		{
		case SVTLAYER_DIFFUSE_COLOR:
			return LAYER_INDEX_DIFFUSE;
		case SVTLAYER_ALPHA:
			return LAYER_INDEX_ALPHA;
		case SVTLAYER_SPECULAR_COLOR:
			return LAYER_INDEX_SPECULAR_COLOR;
		case SVTLAYER_SPECULAR_POWER:
			return LAYER_INDEX_SPECULAR_POWER;
		case SVTLAYER_EMISSIVE_COLOR:
			return LAYER_INDEX_EMISSIVE_COLOR;
		case SVTLAYER_NORMAL_MAP:
			return LAYER_INDEX_NORMAL_MAP;
		default:
			return -1;
		}
	}
	u32 SVTPage::getLayerMask(u32 Index)
	{
		switch(Index)
		{
		case LAYER_INDEX_DIFFUSE:
			return SVTLAYER_DIFFUSE_COLOR;
		case LAYER_INDEX_ALPHA:
			return SVTLAYER_ALPHA;
		case LAYER_INDEX_SPECULAR_COLOR:
			return SVTLAYER_SPECULAR_COLOR;
		case LAYER_INDEX_SPECULAR_POWER:
			return SVTLAYER_SPECULAR_POWER;
		case LAYER_INDEX_EMISSIVE_COLOR:
			return SVTLAYER_EMISSIVE_COLOR;
		case LAYER_INDEX_NORMAL_MAP:
			return SVTLAYER_NORMAL_MAP;
		default:
			return 0;
		}
	}

	const ISVTDataLayer* SVTPage::GetLayer(u32 Layer) const
	{
		if(_layerMask & Layer)
			return _layers[getLayerIndex(Layer)];
		else
			return nullptr;
	}

	bool SVTPage::UpdateLayer(u32 Layer,ISVTDataLayer* data)
	{
		u32 index= getLayerIndex(Layer);
		
		if(_layerMask&Layer)
		{
			assert(_layers[index]);
			delete _layers[index];
		}
		_layers[index] = data;

		return true;
	}

	
	static void *SzAlloc(void *p, size_t size) { p = p; return new u8[size]; }
	static void SzFree(void *p, void *address) {  p = p; delete [] address; }
	static ISzAlloc g_Alloc = { SzAlloc, SzFree };

	const static SVTPage::PAGE_FLAGS final_compression = SVTPage::PAGE_COMPRESSED_LZMA;
	
	u32 SVTPage::Serialize(u32 maxBytes,u8* pOutBytes,SVTPageHeader* headerOut,PAGE_FLAGS requested_compression) const
	{
		u32 pos = 0;
		if(_layerMask == 0)
			return 0;
		assert(_layerMask <= 0xffff);
		headerOut->layers = (u16)_layerMask;
		headerOut->flags = 0;

		std::array<u8,nMaxStackBuffer>	stackBuffer;
		std::vector<u8>					heapBuffer;

		u8* pBuffer = nullptr;
		u32  nBuffer = ISVTDataLayer::MaxLayerSerializedSizeFactor * _pageSize * _pageSize;

		if( nBuffer > stackBuffer.size() )
		{
			heapBuffer.resize(nBuffer);
			pBuffer = heapBuffer.data();
		}
		else
		{
			pBuffer = stackBuffer.data();
			nBuffer = stackBuffer.size();
		}
		
		for(int iLayer = 0; iLayer  < nNumLayers; ++iLayer)
		{
			if(_layerMask & getLayerMask(iLayer))
			{
				assert(_layers[iLayer]);
				LayerHeader header;
				header.layerEncoding = (u16)_layers[iLayer]->GetEncodingId();

				bool bWritten = false;

				if(!(requested_compression & final_compression &&  header.layerEncoding == LAYER_ENCODING_UNCOMPRESSED_RAW))
				{
					header.layerSize = _layers[iLayer]->Serialize(nBuffer,pBuffer);

					if(header.layerSize <= ISVTDataLayer::MaxLayerSerializedSizeFactor * _pageSize * _pageSize)
					{
						//layer is accepted, see if we can compress it now
						bWritten = true;

						u8* layerOut = &pOutBytes[pos+sizeof(LayerHeader)];
						u32 layerOutMax = maxBytes - (pos+sizeof(LayerHeader));

						u32 uncompressed_size = header.layerSize;

						header.layerSize = compress_layer(pBuffer,uncompressed_size,layerOut,layerOutMax,requested_compression);

						if(header.layerSize < uncompressed_size)
						{
							header.layerFlags = requested_compression;
						}
						else
						{
							header.layerSize=uncompressed_size;
							memcpy(layerOut,pBuffer,uncompressed_size);
							header.layerFlags = 0;
						}

						*(LayerHeader*)&pOutBytes[pos] = header;
						pos+= header.layerSize + sizeof(LayerHeader);
					}
				}

				if(!bWritten)
				{
					//convert to jpeg XR if header was rejected before for any reason
					ISVTDataLayer* new_layer = ISVTDataLayer::CreateLayer(LAYER_ENCODING_JPEG_XR_RAW);
					new_layer->Empty(Vector2<u32>(_pageSize,_pageSize));
					new_layer->WriteRect(Vector2<i32>(0,0),getLayerMask(iLayer),_layers[iLayer]);
					
					header.layerEncoding = LAYER_ENCODING_JPEG_XR_RAW;
					header.layerSize =  new_layer->Serialize(maxBytes-pos-sizeof(LayerHeader),&pOutBytes[pos+sizeof(LayerHeader)]);
					header.layerFlags = 0;
					
					*(LayerHeader*)&pOutBytes[pos] = header;

					pos+= header.layerSize + sizeof(LayerHeader);

					delete new_layer;
				}

				//printf("Header info: %08x - %04hx - %04hx\n",header.layerSize,header.layerEncoding,header.layerFlags);
			}
		}
		u32 used_data_size = pos;

		assert(used_data_size < maxBytes);
		return used_data_size;
	}
	
	u32 SVTPage::compress_layer(const void* pInData,u32 pInDataSize,void* pOutData,u32 maxOutSize,PAGE_FLAGS compression)
	{
		if(compression == PAGE_COMPRESSED_DEFLATE)
		{
			deflate_compress_ctx* ctx = deflate_compress_new(DEFLATE_TYPE_BARE);
		
			void* pCompressed;
			int compressed_size;

			deflate_compress_data(ctx,pInData,pInDataSize,DEFLATE_END_OF_DATA,&pCompressed,&compressed_size);
			
			if(compressed_size < maxOutSize)
				memcpy(pOutData,pCompressed,compressed_size);

			deflate_compress_free(ctx);

			return compressed_size;
		}
		else if(compression == PAGE_COMPRESSED_SNAPPY)
		{
			if( snappy::MaxCompressedLength(pInDataSize) > maxOutSize )
				return -1;
			size_t compressed_size = maxOutSize;
			snappy::RawCompress((const char*)pInData,pInDataSize,(char*)pOutData,&compressed_size);
			return compressed_size;
		}
		else if(compression == PAGE_COMPRESSED_LZMA)
		{
			CLzmaEncProps props;
			LzmaEncProps_Init(&props);
			props.numThreads = 1;
			props.lp = 1;//byte size addressing generally better for images
			size_t size_props_encoded = LZMA_PROPS_SIZE;
			u8* pOut = (u8*)pOutData;
			size_t compressed_size = maxOutSize;
			SRes result = LzmaEncode(&pOut[LZMA_PROPS_SIZE],&compressed_size,(const Byte*)pInData,pInDataSize,&props,&pOut[0],&size_props_encoded,true,nullptr,&g_Alloc,&g_Alloc);
			if(result == SZ_OK && (compressed_size + LZMA_PROPS_SIZE < pInDataSize))
				return compressed_size;
			else
				return -1;
		}
	}

	u32 SVTPage::decompress_layer(const void* pInData,u32 pInDataSize,void* pOutData,u32 maxOutSize,PAGE_FLAGS compression)
	{
		if(compression & PAGE_COMPRESSED_DEFLATE)
		{
			deflate_decompress_ctx* ctx = deflate_decompress_new(DEFLATE_TYPE_BARE);
			void* pOut;
			int iOut;
			deflate_decompress_data(ctx,pInData,pInDataSize,&pOut,&iOut);
			assert(iOut < maxOutSize);
			memcpy(pOutData,pOut,iOut);
			deflate_decompress_free(ctx);
			return (u32)iOut;
		}
		else if(compression & PAGE_COMPRESSED_SNAPPY)
		{

			size_t decompressed_size;

			bool bLen = snappy::GetUncompressedLength((const char*)pInData,pInDataSize,&decompressed_size);

			assert(decompressed_size<maxOutSize);

			bool bDecomp = snappy::RawUncompress((const char*)pInData,pInDataSize,(char*)pOutData);
			
			return (u32)decompressed_size;
		}
		else if(compression & PAGE_COMPRESSED_LZMA)
		{
			size_t decompressed_size = maxOutSize;
			size_t compressed_size = pInDataSize - LZMA_PROPS_SIZE;

			ELzmaStatus status;

			SRes result = LzmaDecode((Byte*)pOutData, &decompressed_size, &((u8*)pInDataSize)[LZMA_PROPS_SIZE], &compressed_size,
			  ((u8*)pInDataSize), LZMA_PROPS_SIZE, LZMA_FINISH_END, &status, &g_Alloc);

			assert(result == SZ_OK);

			return (u32)decompressed_size;
		}
	}

	//initialization functions
	void SVTPage::Empty()
	{
		reset();
	}

	void SVTPage::GenerateMipmap(const std::array<const SVTPage*,16>& parents,u32 borderSize)
	{
		reset();

		std::array<const ISVTDataLayer*,16> layerParents;

		for(int iLayer = 0; iLayer < nNumLayers; ++iLayer)
		{
			bool bMixed = false;
			i32 encoding = -1;
			u32 nParents = 0;
			for(int iParent = 0; iParent < 16; iParent++)
			{
				if(parents[iParent]->_layerMask & getLayerMask(iLayer))
				{
					++nParents;
					layerParents[iParent] = parents[iParent]->_layers[iLayer];
					if(encoding == -1)
						encoding = (i32)layerParents[iParent]->GetEncodingId();
					else if(!bMixed && encoding != (i32)layerParents[iParent]->GetEncodingId())
						bMixed = true;
				}
				else
					layerParents[iParent] = nullptr;
			}

			if(nParents == 0)
			{
				_layers[iLayer] = nullptr;
				continue;
			}
			else
			{
				if(bMixed)
					encoding = defaultMixedEncoding;

				ISVTDataLayer* new_data = ISVTDataLayer::CreateLayer(encoding);

				assert(new_data);

				new_data->GenerateMipmap(Vector2<u32>(_pageSize,_pageSize),layerParents,borderSize);

				_layers[iLayer] = new_data;

				_layerMask |= getLayerMask(iLayer);
			}
		}
	}

	void SVTPage::WriteRect(Vector2<i32> offset,u32 layer,const ISVTDataLayer* data)
	{
		u32 iLayer = getLayerIndex(layer);
		if(!(_layerMask&layer))
		{
			_layers[iLayer] = ISVTDataLayer::CreateLayer(data->GetEncodingId());
			assert(_layers[iLayer]);
			_layers[iLayer]->Empty(Vector2<u32>(_pageSize,_pageSize));
			_layerMask |=layer;
		}
		else if(_layers[iLayer]->GetEncodingId() != defaultMixedEncoding && _layers[iLayer]->GetEncodingId() != data->GetEncodingId())
		{
			 ISVTDataLayer* new_data = ISVTDataLayer::CreateLayer(defaultMixedEncoding);
			 assert(new_data);
			 new_data->Empty(Vector2<u32>(_pageSize,_pageSize));
			 new_data->WriteRect(Vector2<i32>(0,0),layer,_layers[iLayer]);
			 delete _layers[iLayer];
			 _layers[iLayer] = new_data;
		}
		
		ISVTDataLayer* new_data = _layers[iLayer]->WriteRect(offset,layer,data);

		assert(new_data == _layers[iLayer]);

		return;
	}

	void SVTPage::reset()
	{
		if(_layerMask)
		{
			for(auto it = _layers.begin(); it != _layers.end(); ++it)
				if(_layerMask & getLayerMask(it - _layers.begin()))
					delete *it;
			_layerMask=0;
		}
	}

	bool SVTPage::Deserialize(u32 pageSize,const SVTPageHeader* header, u32 numBytes,const u8* pInBytes,const ISVTSharedDataStorage* pShared)
	{
		reset();
		u32 pos = 0;
		_layerMask = (u32)header->layers;
		

		std::array<u8,nMaxStackBuffer>	stackBuffer;
		std::vector<u8>					heapBuffer;

		u8* pBuffer = nullptr;
		u32  nBuffer = ISVTDataLayer::MaxLayerSerializedSizeFactor * _pageSize * _pageSize;

		if( nBuffer > stackBuffer.size() )
		{
			heapBuffer.resize(nBuffer);
			pBuffer = heapBuffer.data();
		}
		else
		{
			pBuffer = stackBuffer.data();
			nBuffer = stackBuffer.size();
		}

		for(int iLayer = 0; iLayer  < nNumLayers; ++iLayer)
			if(_layerMask & getLayerMask(iLayer))
			{
				LayerHeader* h = (LayerHeader*)&pInBytes[pos];
				u32 encoding = h->layerEncoding;

				const u8* pLayer;
				u32 nLayer;

				if(h->layerFlags)
				{
					nLayer = decompress_layer(&pInBytes[pos+sizeof(LayerHeader)],h->layerSize,pBuffer,nBuffer,(PAGE_FLAGS)h->layerFlags);
					pLayer = pBuffer;
					pos+= sizeof(LayerHeader) + h->layerSize;
				}
				else
				{
					nLayer = h->layerSize;
					pLayer = &pInBytes[pos+sizeof(LayerHeader)];
					pos+= sizeof(LayerHeader) + h->layerSize;
				}

				_layers[iLayer] = ISVTDataLayer::CreateLayer(encoding);

				pos+= _layers[iLayer]->Deserialize(Vector2<u32>(_pageSize,_pageSize),nLayer,pLayer,pShared);
			}


		return true;
	}

}