#include "header.h"
#include "VirtualTextureGenerator.h"
#include "GenerationDomain.h"
#include "../Source/Core/SVTPageEncoding.h"

namespace HAGE {



		GenerationDomain::GenerationDomain() : _dataProc(xBegin,xEnd,yBegin,yEnd)
		{
			printf("Init Generation\n");
			
			FreeImage_Initialise();
		}

		bool GenerationDomain::MessageProc(const Message* m)
		{
			return SharedDomainBase::MessageProc(m);
		}		

		void GenerationDomain::DomainStep(t64 time)
		{/*
			static bool test = true;

			if(test)
			{
				const int page_size = 128;
				SVTPage pageTest(128);
				SVTPage pageTestL(1024);
				SVTPage::SVTPageHeader header;

				FreeImage_Initialise();
				FIBITMAP* pBitmap = FreeImage_Load(FIF_PNG,"MipMap_partial5.png",0);
				SVTDataLayer_PNGish rawImage;
				SVTDataLayer_JpegXR pngImage;
				ImageData<R8G8B8A8> image_data(1024,1024);
				for(int y = 0; y < 1024; ++y)
					for(int x = 0; x < 1024; ++x)
					{
						RGBQUAD color;
						FreeImage_GetPixelColor(pBitmap,x,y,&color);
						image_data(x,y).SetRed(color.rgbRed);
						image_data(x,y).SetBlue(color.rgbBlue);
						image_data(x,y).SetGreen(color.rgbGreen);
					}
				rawImage.Initialize(image_data);
				pngImage.Initialize(image_data);
								
				std::vector<u8> buffer(1024*1024*20);

				for(int y = 0; y < 1024/page_size; ++y)
					for(int x = 0; x < 1024/page_size; ++x)
					{
						pageTest.Empty();
						pageTest.WriteRect(Vector2<i32>(-x * page_size,-y * page_size),SVTLAYER_DIFFUSE_COLOR,&pngImage);
						u32 bytes = pageTest.Serialize(buffer.size(),buffer.data(),&header,SVTPage::PAGE_FLAGS::PAGE_COMPRESSED_NONE);

						pageTest.Deserialize(128,&header,bytes,buffer.data(),nullptr);
						pageTest.GetLayer(SVTLAYER_DIFFUSE_COLOR)->GetImageData(Vector2<u32>(0,0),SVTLAYER_DIFFUSE_COLOR,image_data.GetRect(Vector4<u32>(x * page_size,y * page_size,(x+1) * page_size,(y+1) * page_size)));
					}

				
				for(int y = 0; y < 1024; ++y)
					for(int x = 0; x < 1024; ++x)
					{
						RGBQUAD color;
						color.rgbRed = image_data(x,y).Red();
						color.rgbBlue = image_data(x,y).Blue();
						color.rgbGreen = image_data(x,y).Green();
						FreeImage_SetPixelColor(pBitmap,x,y,&color);
					}

				FreeImage_Save(FIF_PNG, pBitmap, "TestOut.png");
				FreeImage_Unload(pBitmap);



				const u32 compression_methods = 4;

				for(u32 method = 0; method < compression_methods; method ++)
				{
					u32 totalSize = 0;
					u32 totalSizePNG = 0;
					for(int y = 0; y < 1024/page_size; ++y)
						for(int x = 0; x < 1024/page_size; ++x)
						{
							pageTest.Empty();
							pageTest.WriteRect(Vector2<i32>(-x * page_size,-y * page_size),SVTLAYER_DIFFUSE_COLOR,&rawImage);
							totalSize += pageTest.Serialize(buffer.size(),buffer.data(),&header,(SVTPage::PAGE_FLAGS)(method?(1<<(method-1)):0));
							pageTest.Empty();
							pageTest.WriteRect(Vector2<i32>(-x * page_size,-y * page_size),SVTLAYER_DIFFUSE_COLOR,&pngImage);
							totalSizePNG += pageTest.Serialize(buffer.size(),buffer.data(),&header,(SVTPage::PAGE_FLAGS)(method?(1<<(method-1)):0));
						}
						
					pageTestL.Empty();
					pageTestL.WriteRect(Vector2<i32>(0,0),SVTLAYER_DIFFUSE_COLOR,&rawImage);
					u32 largeSize = pageTestL.Serialize(buffer.size(),buffer.data(),&header,(SVTPage::PAGE_FLAGS)(method?(1<<(method-1)):0));
					pageTestL.Empty();
					pageTestL.WriteRect(Vector2<i32>(0,0),SVTLAYER_DIFFUSE_COLOR,&pngImage);
					u32 largeSizePNG = pageTestL.Serialize(buffer.size(),buffer.data(),&header,(SVTPage::PAGE_FLAGS)(method?(1<<(method-1)):0));

					printf("Method %u, %u to %u\n",method,totalSize/1024,totalSizePNG/1024);
					printf("Method %uL, %u to %u\n",method,largeSize/1024,largeSizePNG/1024);
				}

				test = false;
			}
			else
				GetTask().Shutdown();
			*/
			if(_dataProc.Process())
				GetTask().Shutdown();
		}

		GenerationDomain::~GenerationDomain()
		{
			FreeImage_DeInitialise();
			printf("Destroy GenerationDomain\n");
		}
}
