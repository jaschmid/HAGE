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
		
		for(int iLayer = 0; iLayer  < nNumLayers; ++iLayer)
		{
			if(_layerMask & getLayerMask(iLayer))
			{
				assert(_layers[iLayer]);
				if(requested_compression & final_compression && _layers[iLayer]->GetEncodingId() == LAYER_ENCODING_UNCOMPRESSED_RAW)
				{
					ISVTDataLayer* new_layer = ISVTDataLayer::CreateLayer(LAYER_ENCODING_JPEG_XR_RAW);
					new_layer->Empty(Vector2<u32>(_pageSize,_pageSize));
					new_layer->WriteRect(Vector2<i32>(0,0),getLayerMask(iLayer),_layers[iLayer]);
					
					*(u32*)&pOutBytes[pos] = new_layer->GetEncodingId();
					pos+= 4;
					pos+= new_layer->Serialize(maxBytes-pos,&pOutBytes[pos]);

					delete new_layer;
				}
				else
				{
					*(u32*)&pOutBytes[pos] = _layers[iLayer]->GetEncodingId();
					pos+= 4;
					pos+= _layers[iLayer]->Serialize(maxBytes-pos,&pOutBytes[pos]);
				}
			}
		}
		u32 used_data_size = pos;

		if(requested_compression == PAGE_COMPRESSED_DEFLATE)
		{
			deflate_compress_ctx* ctx = deflate_compress_new(DEFLATE_TYPE_BARE);
		
			void* pCompressed;
			int compressed_size;

			deflate_compress_data(ctx,pOutBytes,used_data_size,DEFLATE_END_OF_DATA,&pCompressed,&compressed_size);

			if(compressed_size < pos)
			{
				headerOut->flags |= PAGE_COMPRESSED_DEFLATE;
				memcpy(pOutBytes,pCompressed,compressed_size);
				used_data_size =compressed_size;
			}

			deflate_compress_free(ctx);
		}
		else if(requested_compression == PAGE_COMPRESSED_SNAPPY)
		{
			std::vector<u8> compressed(snappy::MaxCompressedLength(pos));
			size_t compressed_size = compressed.size();
			snappy::RawCompress((const char*)pOutBytes,used_data_size,(char*)compressed.data(),&compressed_size);
			if(compressed_size < used_data_size)
			{
				headerOut->flags |= PAGE_COMPRESSED_SNAPPY;
				memcpy(pOutBytes,compressed.data(),compressed_size);
				used_data_size= compressed_size;
			}
		}
		else if(requested_compression == PAGE_COMPRESSED_LZMA)
		{
			CLzmaEncProps props;
			LzmaEncProps_Init(&props);
			props.numThreads = 1;
			props.lp = 1;//byte size addressing generally better for images
			size_t size_props_encoded = LZMA_PROPS_SIZE;
			std::vector<u8> compressed(LZMA_PROPS_SIZE+used_data_size*2);
			size_t compressed_size = compressed.size();
			SRes result = LzmaEncode(&compressed[LZMA_PROPS_SIZE],&compressed_size,pOutBytes,used_data_size,&props,&compressed[0],&size_props_encoded,true,nullptr,&g_Alloc,&g_Alloc);
			if(result == SZ_OK && (compressed_size + LZMA_PROPS_SIZE < used_data_size))
			{
				headerOut->flags |= PAGE_COMPRESSED_LZMA;
				memcpy(pOutBytes,compressed.data(),compressed_size + LZMA_PROPS_SIZE);
				used_data_size= compressed_size + LZMA_PROPS_SIZE;
			}
		}

		assert(used_data_size < maxBytes);
		return used_data_size;
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
		const u8* pData;
		u32 nData;
		deflate_decompress_ctx* ctx;


		if(header->flags & PAGE_COMPRESSED_DEFLATE)
		{
			ctx = deflate_decompress_new(DEFLATE_TYPE_BARE);
			void* pOut;
			int iOut;
			deflate_decompress_data(ctx,pInBytes,numBytes,&pOut,&iOut);
			pData = (const u8*)pOut;
			nData = (u32)iOut;
		}
		else if(header->flags & PAGE_COMPRESSED_SNAPPY)
		{

			size_t decompressed_size;

			bool bLen = snappy::GetUncompressedLength((const char*)pInBytes,numBytes,&decompressed_size);

			u8* decompressed = new u8[ decompressed_size ];

			bool bDecomp = snappy::RawUncompress((const char*)pInBytes,numBytes,(char*)decompressed);
			
			pData = (const u8*)decompressed;
			nData = (u32)decompressed_size;
		}
		else if(header->flags & PAGE_COMPRESSED_LZMA)
		{
			size_t decompressed_size = (_pageSize*_pageSize*sizeof(u32)+sizeof(u32)) * nNumLayers;
			u8* decompressed = new u8[ decompressed_size ];
			size_t compressed_size = numBytes - LZMA_PROPS_SIZE;

			ELzmaStatus status;

			SRes result = LzmaDecode(decompressed, &decompressed_size, &pInBytes[LZMA_PROPS_SIZE], &compressed_size,
			  pInBytes, LZMA_PROPS_SIZE, LZMA_FINISH_END, &status, &g_Alloc);

			assert(result == SZ_OK);

			pData = (const u8*) decompressed;
			nData = (u32)decompressed_size;
		}
		else
		{
			pData = pInBytes;
			nData = numBytes;
		}

		for(int iLayer = 0; iLayer  < nNumLayers; ++iLayer)
			if(_layerMask & getLayerMask(iLayer))
			{
				u32 encoding = *(u32*)&pData[pos];
				pos+= 4;

				_layers[iLayer] = ISVTDataLayer::CreateLayer(encoding);

				pos+= _layers[iLayer]->Deserialize(Vector2<u32>(_pageSize,_pageSize),nData-pos,&pData[pos],pShared);
			}

		if(header->flags & PAGE_COMPRESSED_DEFLATE)
			deflate_decompress_free(ctx);
		else if(header->flags & PAGE_COMPRESSED_LZMA)
			delete [] (u8*)pData;
		else if(header->flags & PAGE_COMPRESSED_SNAPPY)
			delete [] (u8*)pData;

		return true;
	}

}