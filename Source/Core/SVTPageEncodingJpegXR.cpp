#include <HAGE.h>
#include <stdio.h>

#include "SVTPageEncoding.h"

#include "Dependancies/JpegXR/jpegxr.h"

namespace HAGE
{

static const u8 xr_base_quant = 20;

u8 getDerivQuant(u8 idx)
{
	if (idx < 16) 
        return idx + ((idx + 2) >> 2);
	else if (idx <= 48) 
		return idx + 4;
	else
        return idx + 4 + 2;
}

static u8 xr_quant[3] = {xr_base_quant , getDerivQuant(xr_base_quant), getDerivQuant(xr_base_quant)};

void SVTDataLayer_JpegXR::static_jxr_read_block(void* image_data, int mx, int my, int*data)
{
	jxr_image_t jxr = (jxr_image_t)image_data;
	const SVTDataLayer_JpegXR* user_data = (SVTDataLayer_JpegXR*)jxr_get_user_data(jxr);
	user_data->jxr_read_block(image_data,mx,my,data);
}
void SVTDataLayer_JpegXR::static_jxr_write_block(void* image_data, int mx, int my, int*data)
{
	jxr_image_t jxr = (jxr_image_t)image_data;
	SVTDataLayer_JpegXR* user_data = (SVTDataLayer_JpegXR*)jxr_get_user_data(jxr);
	user_data->jxr_write_block(image_data,mx,my,data);
}

void SVTDataLayer_JpegXR::jxr_read_block(void* image_data, int mx, int my, int*data) const
{
	jxr_image_t jxr = (jxr_image_t)image_data;

	const static u32 macro_size = 16;
	
	for(u32 y = 0; y < macro_size; ++y)
		for(u32 x = 0; x < macro_size; ++x)
		{
			((u32*)data)[(y*macro_size+x)*3+0] = (*_imageData)(x + mx*macro_size,y + my*macro_size).Red();
			((u32*)data)[(y*macro_size+x)*3+1] = (*_imageData)(x + mx*macro_size,y + my*macro_size).Green();
			((u32*)data)[(y*macro_size+x)*3+2] = (*_imageData)(x + mx*macro_size,y + my*macro_size).Blue();
		}
}
void SVTDataLayer_JpegXR::jxr_write_block(void* image_data, int mx, int my, int*data)
{
	jxr_image_t jxr = (jxr_image_t)image_data;
	
	const static u32 macro_size = 16;
	
	for(u32 y = 0; y < macro_size; ++y)
		for(u32 x = 0; x < macro_size; ++x)
		{
			(*_imageData)(x + mx*macro_size,y + my*macro_size).SetRed(((u32*)data)[(y*macro_size+x)*3+0]);
			(*_imageData)(x + mx*macro_size,y + my*macro_size).SetGreen(((u32*)data)[(y*macro_size+x)*3+1]);
			(*_imageData)(x + mx*macro_size,y + my*macro_size).SetBlue(((u32*)data)[(y*macro_size+x)*3+2]);
		}
}

u32 SVTDataLayer_JpegXR::Serialize(u32 maxBytes,u8* pOutBytes) const
{
	if(!_imageData)
		return 0;

	unsigned char windowing[5] = {0,0,0,0,0};
	jxr_image_t jxr = jxr_create_image(_size.x,_size.y,windowing);

	jxr_set_INTERNAL_CLR_FMT(jxr,JXR_YUV420,3);
	jxr_set_OUTPUT_CLR_FMT(jxr,JXR_OCF_RGB);
	jxr_set_OUTPUT_BITDEPTH(jxr,JXR_BD8);
	jxr_set_BANDS_PRESENT(jxr,JXR_BP_ALL);
	jxr_set_QP_INDEPENDENT(jxr,xr_quant);
	jxr_set_TILING_FLAG(jxr,false);
	//jxr_set_LONG_WORD_FLAG(jxr,true);
	unsigned width[1] = { _size.x /16};
	unsigned height[1] = { _size.y /16};
    jxr_set_NUM_HOR_TILES_MINUS1(jxr, 1);
	jxr_set_TILE_WIDTH_IN_MB(jxr, width);
    jxr_set_NUM_VER_TILES_MINUS1(jxr, 1);
	jxr_set_TILE_HEIGHT_IN_MB(jxr, height);
	

	jxr_set_block_input(jxr,(block_fun_t)static_jxr_read_block);
	jxr_set_pixel_format(jxr,JXRC_FMT_32bppBGR);
	jxr_set_user_data(jxr,const_cast<void*>((const void*)this));
	int written = jxr_write_image_bitstream(jxr,pOutBytes,maxBytes);

	jxr_destroy(jxr);

	assert(written > 0);

	return (u32) written;
}

u32 SVTDataLayer_JpegXR::Deserialize(const Vector2<u32>& size,u32 numBytes,const u8* pInBytes,const ISVTSharedDataStorage* pShared)
{
	reset();
	_size = size;
	_imageData = new ImageData<R8G8B8A8>(_size.x,_size.y);

	jxr_image_t jxr = jxr_create_input();
	jxr_set_INTERNAL_CLR_FMT(jxr,JXR_YUV420,3);
	jxr_set_OUTPUT_CLR_FMT(jxr,JXR_OCF_RGB);
	jxr_set_OUTPUT_BITDEPTH(jxr,JXR_BD8);
	jxr_set_BANDS_PRESENT(jxr,JXR_BP_ALL);
	jxr_set_TILING_FLAG(jxr,false);
	//jxr_set_LONG_WORD_FLAG(jxr,true);

	jxr_set_block_output(jxr,(block_fun_t)static_jxr_write_block);
	jxr_set_pixel_format(jxr,JXRC_FMT_32bppBGR);
	jxr_set_container_parameters(jxr,JXRC_FMT_32bppBGR,_size.x,_size.y,0,0,0,0);
	jxr_set_user_data(jxr,this);
	int read = jxr_read_image_bitstream(jxr,pInBytes,numBytes);

	jxr_destroy(jxr);

	assert( read > 0);

	return (u32) read;
}

}