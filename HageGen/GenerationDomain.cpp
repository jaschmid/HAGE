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
		{
			static bool test = true;

			if(test)
			{
				const int page_size = 128;
				SVTPage pageTest(128);
				SVTPage pageTestL(1024);
				FreeImage_Initialise();
				FIBITMAP* pBitmap = FreeImage_Load(FIF_PNG,"SampleTex.png",0);
				SVTDataLayer_Raw rawImage;
				SVTDataLayer_PNGish pngImage;
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
				FreeImage_Unload(pBitmap);
				
				std::vector<u8> buffer(1024*1024*20);

				SVTPage::SVTPageHeader header;

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

					printf("Method %u, %u.%03u to %u.%03u\n",method,totalSize/1000,totalSize%1000,totalSizePNG/1000,totalSizePNG%1000);
					printf("Method %uL, %u.%03u to %u.%03u\n",method,largeSize/1000,largeSize%1000,largeSizePNG/1000,largeSizePNG%1000);
				}

				test = false;
			}
			else
				GetTask().Shutdown();
			/*
			if(_dataProc.Process())
				GetTask().Shutdown();*/
		}

		GenerationDomain::~GenerationDomain()
		{
			FreeImage_DeInitialise();
			printf("Destroy GenerationDomain\n");
		}
}
