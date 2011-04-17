#ifndef VIRTUA_TEXTURE_GENERATOR__INCLUDED
#define VIRTUA_TEXTURE_GENERATOR__INCLUDED

#include "header.h"

#define FREEIMAGE_LIB
#include <FreeImage.h>
namespace HAGE {


	class SparseVirtualTextureGenerator : public SparseVirtualTextureFile
	{
	public:

		SparseVirtualTextureGenerator() 
		{
		}
		
		void Commit()
		{
			//make mipmaps
			FinalizeDepthLayer(0);

			//debug stuff
			// without borders
			for(int i = 0; i< std::min((int)GetNumLayers(),(int)5);++i)
			{
				std::vector<u32> buffer;
				u32 xSize = GetPageInnerSize()*GetNumXPagesAtDepth(i);
				buffer.resize(xSize*xSize);

				MergePagesToImage(
							0,GetNumXPagesAtDepth(i),
							0,GetNumXPagesAtDepth(i),
							i,(Color*)buffer.data(),0);

				char temp[256];
				sprintf(temp,"MipLevel%i.png",i);
				
				OutputImage(temp,(Color*)buffer.data(),xSize,xSize);
			}

			//with borders
			for(int i = 0; i< std::min((int)GetNumLayers(),(int)5);++i)
			{
				std::vector<u32> buffer;
				u32 xSize = GetPageOuterSize()*GetNumXPagesAtDepth(i);
				buffer.resize(xSize*xSize);

				MergePagesToImage(
							0,GetNumXPagesAtDepth(i),
							0,GetNumXPagesAtDepth(i),
							i,(Color*)buffer.data(),nullptr,SparseVirtualTextureFile::PAGE_FINAL);

				char temp[256];
				sprintf(temp,"MipLevel_borders_%i.png",i);

				OutputImage(temp,(Color*)buffer.data(),xSize,xSize);
			}

			//test image
			{
				std::vector<Color> buffer;
				u32 xSize = 4096;
				buffer.resize(xSize*xSize);

				ReadImageFromVirtualTexture(0,0,xSize,xSize,buffer.data(),6);

				OutputImage("Landscape.png",(Color*)buffer.data(),xSize,xSize);
			}

			SparseVirtualTextureFile::Commit();
		}

		
		struct Rectangle
		{
			u32 xBegin,xSize;
			u32 yBegin,ySize;
		};

		class PlacedTexture
		{
		public:
			f32 GetUBegin() const{return (f32)location.xBegin / (float)base->GetSize();}
			f32 GetUSize() const{return (f32)location.xSize / (float)base->GetSize();}
			f32 GetVBegin() const{return (f32)location.yBegin / (float)base->GetSize();}
			f32 GetVSize() const{return (f32)location.ySize / (float)base->GetSize();}
		private:
			PlacedTexture(const SparseVirtualTextureGenerator* parent,const Rectangle& rect,u32 i) : 
			   base(parent),location(rect),index(i) {}

			u32 index;
			const SparseVirtualTextureGenerator* base;
			Rectangle location;
		};

		typedef const PlacedTexture& TextureReference;
		typedef std::vector<std::pair<TextureReference,f32>> RelationArray;

		TextureReference PlaceTexture(u32 xSize,u32 ySize,u32* pImageData,RelationArray relationArray);
		
	private:

		class FreeSquare : public Rectangle
		{
		public:
			bool operator < (const FreeSquare& sq) const
			{
			}

		};


		struct Color
		{
			u8 r,g,b,a;
		};
		

		static void OutputImage(char* filename,const Color* pData,u32 xSize,u32 ySize)
		{
			FIBITMAP* b =FreeImage_Allocate(xSize,ySize,24);

			for(u32 iy = 0; iy < ySize;iy++)
				for(u32 ix = 0; ix < xSize;ix++)
					FreeImage_SetPixelColor(b,ix,iy,(RGBQUAD*)&pData[iy*xSize+ix]);

			BOOL res = FreeImage_Save(FIF_PNG,b,filename);
			FreeImage_Unload(b);
		}

		class BorderedImageReader
		{
		public:
			BorderedImageReader(const Color* pData,const std::vector<bool>& PageStatus,u32 xInputPages,u32 yInputPages,u32 PageOuterSize,u32 PageInnerSize) :
				_data(pData),
				_pageStatus(PageStatus),
				_xInputPages(xInputPages),
				_yInputPages(yInputPages),
				_pageOuterSize(PageOuterSize),
				_pageInnerSize(PageInnerSize),
				_pageBorderSize((PageOuterSize - PageInnerSize)/2),
				_rowStride(xInputPages*PageOuterSize)
			{
			}

			const Color& GetPixelFromInner(u32 absX,u32 absY) const
			{
				u32 xPage = absX/_pageInnerSize;
				u32 yPage = absY/_pageInnerSize;
				u32 xPixel = absX%_pageInnerSize;
				u32 yPixel = absY%_pageInnerSize;
				return GetPixelFromInner(xPage,yPage,xPixel,yPixel);
			}

			const Color& GetPixelFromInner(u32 xPage,u32 yPage,u32 xPixel,u32 yPixel) const
			{
				static const Color Black = {0,0,0,0};

				assert(xPage < _xInputPages);
				assert(yPage < _yInputPages);
				assert(xPixel < _pageInnerSize);
				assert(yPixel < _pageInnerSize);

				if(_pageStatus[_xInputPages*yPage + xPage])
				{
					u32 xAbsOut = xPage * _pageOuterSize + xPixel + _pageBorderSize;
					u32 yAbsOut = yPage * _pageOuterSize + yPixel + _pageBorderSize;
					return _data[yAbsOut*_rowStride + xAbsOut];
				}
				else
					return Black;
			}

			const Color& GetPixelFromOuter(u32 absX,u32 absY) const
			{
				u32 xPage = absX/_pageOuterSize;
				u32 yPage = absY/_pageOuterSize;
				u32 xPixel = absX%_pageOuterSize;
				u32 yPixel = absY%_pageOuterSize;
				return GetPixelFromOuter(xPage,yPage,xPixel,yPixel);
			}

			const Color& GetPixelFromOuter(u32 xPage,u32 yPage,u32 xPixel,u32 yPixel) const
			{
				u32 yInPixel = (_pageInnerSize + yPixel - _pageBorderSize)% _pageInnerSize;
				u32 xInPixel = (_pageInnerSize + xPixel - _pageBorderSize)% _pageInnerSize;
				u32 yInPage = yPage + (_pageInnerSize + yPixel - _pageBorderSize)/ _pageInnerSize - 1;
				u32 xInPage = xPage +(_pageInnerSize + xPixel - _pageBorderSize)/ _pageInnerSize - 1;
				return GetPixelFromInner(xInPage,yInPage,xInPixel,yInPixel);
			}

			u32 GetInnerX() const {return _xInputPages*_pageInnerSize;}
			u32 GetInnerY() const {return _yInputPages*_pageInnerSize;}
			u32 GetOuterX() const {return _xInputPages*_pageOuterSize;}
			u32 GetOuterY() const {return _yInputPages*_pageOuterSize;}
			u32 GetPageOuter() const{return _pageOuterSize;}
			u32 GetPageBorder() const{return _pageBorderSize;}
			u32 GetPageInner() const{return _pageInnerSize;}
			u32 GetPagesX() const{return _xInputPages;}
			u32 GetPagesY() const{return _yInputPages;}
			u64 GetInnerTotal() const{return (u64)GetInnerX()*(u64)GetInnerY();}
			u64 GetOuterTotal() const{return (u64)GetOuterX()*(u64)GetOuterY();}
		private:
			const Color* _data;
			const std::vector<bool>& _pageStatus;
			const u32 _xInputPages;
			const u32 _yInputPages;
			const u32 _rowStride;
			const u32 _pageOuterSize;
			const u32 _pageBorderSize;
			const u32 _pageInnerSize;
		};

		static void applyBoxFilter(const BorderedImageReader& source,u32 InputBorder,
			Color* pOutput,u32 xOutputSize,u32 yOutputSize)
		{
			static const u32 FilterSize = 2;
			static const float base = 1.0f/4.0f;
			static const float base_filter[FilterSize][FilterSize] = 
			{
				base,base,
				base,base,
			};

			float trapez_filter[FilterSize+1][FilterSize+1];
			u32 trapez_size;

			//our trapez filter only supports even output sizes
			assert(xOutputSize%2 == 0);
			assert(yOutputSize%2 == 0);
				
			if(FilterSize%2 == 1)
			{
				trapez_size = FilterSize+1;
				for(u32 fy = 0; fy < trapez_size; ++fy)
					for(u32 fx = 0; fx < trapez_size; ++fx)
						trapez_filter[fy][fx] = 0.0f;

				for(u32 fy = 0; fy < FilterSize; ++fy)
					for(u32 fx = 0; fx < FilterSize; ++fx)
					{
						trapez_filter[fy][fx] += 0.25f*base_filter[fy][fx];
						trapez_filter[fy+1][fx] += 0.25f*base_filter[fy][fx];
						trapez_filter[fy][fx+1] += 0.25f*base_filter[fy][fx];
						trapez_filter[fy+1][fx+1] += 0.25f*base_filter[fy][fx];
					}
			}
			else
			{
				trapez_size = FilterSize;
				for(u32 fy = 0; fy < trapez_size; ++fy)
					for(u32 fx = 0; fx < trapez_size; ++fx)
						trapez_filter[fy][fx] = base_filter[fy][fx];
			}
			
			/*
			float Filter[xFilterSize][yFilterSize] = {
				0.25f,0.25f,
				0.25f,1.0f/9.0f,1.0f/9.0f,
				base/2.0f,base,base,base/2.0f

			};*/

			assert( source.GetInnerX() == 2*InputBorder + 2*xOutputSize );
			assert( source.GetInnerY() == 2*InputBorder + 2*yOutputSize );

			for(u32 iy = 0; iy < yOutputSize; iy++)
				for(u32 ix = 0; ix < xOutputSize; ix++)
				{
					u32 xInput = InputBorder - (trapez_size-1)/2 + ix*2;
					u32 yInput = InputBorder - (trapez_size-1)/2 + iy*2;
					float red=0.0f,green=0.0f,blue=0.0f,alpha=0.0f;
					for(u32 fy = 0; fy < trapez_size; ++fy)
						for(u32 fx = 0; fx < trapez_size; ++fx)
						{
							const Color& c = source.GetPixelFromInner(xInput+fx,yInput+fy);
							float f= trapez_filter[fy][fx];
							red += f*(float)c.r;
							green += f*(float)c.g;
							blue += f*(float)c.b;
							alpha += f*(float)c.a;
						}

					Color out;
					out.r = (u8)std::max(std::min(red,255.0f),0.0f);
					out.g = (u8)std::max(std::min(green,255.0f),0.0f);
					out.b = (u8)std::max(std::min(blue,255.0f),0.0f);
					out.a = (u8)std::max(std::min(alpha,255.0f),0.0f);
					pOutput[iy*xOutputSize + ix] = out;
				}
		}


		void SplitPagesFromImage(i64 xPageBegin,u64 xPageEnd,u64 yPageBegin,i64 yPageEnd,u32 depth,const Color* pIn,const std::vector<bool>* pageStatus,u32 flags = 0,u64 inputOffset =0, u64 inputStride = 0)
		{
			i64 nPageSize = GetPageInnerSize();
			if(flags & PAGE_FINAL)
				nPageSize = GetPageOuterSize();


			i64 nXPages = xPageEnd-xPageBegin;
			i64 nYPages = yPageEnd-yPageBegin;
			i64 nXPixels = nXPages*nPageSize;
			if(inputStride)
				nXPixels = inputStride;
			i64 NumXPages =		GetNumXPagesAtDepth(depth);

			std::vector<Color> tempBuffer;
			tempBuffer.resize(nPageSize*nPageSize);
			
			for(i64 uYPage = 0; uYPage < nYPages; uYPage++)
				for(i64 uXPage = 0; uXPage < nXPages; uXPage++)
					if(pageStatus == nullptr || (*pageStatus)[uYPage*nXPages+uXPage])
					{
						i64 iXAbsolute = (i64)(uXPage + xPageBegin) ;
						i64 iYAbsolute = (i64)(uYPage + yPageBegin) ;
					
						//copy tempBuffer to inputBuffer

						for(int iYPixel = 0; iYPixel < (int)nPageSize; iYPixel++)
							for(int iXPixel = 0; iXPixel < (int)nPageSize; iXPixel++)
								tempBuffer[iYPixel * nPageSize + iXPixel] = pIn[inputOffset+(iYPixel+uYPage*nPageSize)*nXPixels + iXPixel + uXPage*nPageSize];
					
						WritePage(GetPageIndex(iXAbsolute,iYAbsolute,depth),tempBuffer.data(),flags);
					}
		}

		bool MergePagesToImage(i32 xPageBegin,i32 xPageEnd,i32 yPageBegin,i32 yPageEnd,u32 depth,Color* pOut,std::vector<bool>* pageStatus,u32 flags = 0)
		{
			bool result = false;
			i64 nPageSize = GetPageInnerSize();
			if(flags & PAGE_FINAL)
				nPageSize = GetPageOuterSize();
			i64 nPagePixels = nPageSize*nPageSize;
			i64 nXPages = xPageEnd-xPageBegin;
			i64 nYPages = yPageEnd-yPageBegin;
			i64 nXPixels = nXPages*nPageSize;
			i64 TotalXPages =		GetNumXPagesAtDepth(depth);

			std::vector<Color> tempBuffer;
			tempBuffer.resize(nPagePixels);
			
			//read input pages
			for(i64 uYPage = 0; uYPage < nYPages; uYPage++)
				for(i64 uXPage = 0; uXPage < nXPages; uXPage++)
				{
					i64 iXAbsolute = (i64)(uXPage + xPageBegin) ;
					i64 iYAbsolute = (i64)(uYPage + yPageBegin) ;

					//magic wrap around

					iXAbsolute = (iXAbsolute+TotalXPages)%TotalXPages;
					iYAbsolute = (iYAbsolute+TotalXPages)%TotalXPages;

					bool status=ReadPage(GetPageIndex(iXAbsolute,iYAbsolute,depth),tempBuffer.data(),flags);
					
					if(pageStatus)
						(*pageStatus)[uYPage*nXPages+uXPage] = status;

					//copy tempBuffer to inputBuffer
					if(status)
					{
						result = true;
						for(int iXPixel = 0; iXPixel < (int)nPageSize; iXPixel++)
							for(int iYPixel = 0; iYPixel < (int)nPageSize; iYPixel++)
								pOut[(iYPixel+uYPage*nPageSize)*nXPixels + iXPixel + uXPage*nPageSize] =
									tempBuffer[iYPixel * nPageSize + iXPixel];
					}
				}
			return result;
		}

		bool FinalizeDepthLayer(u32 depth) 
		{
			if(depth >= GetNumLayers())
				return true;

			
			FinalizeDepthLayer(depth+1);
			
			printf("Finalizing Depth Layer %u ",depth);

			static const u64 maxMemoryForFinalizing = 128*1024*1024; //512 MB for finalization
			const u32 maxInputPages = (maxMemoryForFinalizing*4/5)/GetPageOuterPixelCount()/sizeof(Color)-1;
			const u32 maxInputBlockDimension = ((u64)sqrt((float)maxInputPages))/2*2;
			const u32 maxInputBlockInnerDimension = maxInputBlockDimension-2;
			const u32 maxOutputBlockDimension = (maxInputBlockDimension-2)/2;
			const u32 maxOutputPages = maxOutputBlockDimension * maxOutputBlockDimension;
			const u32 InputDimension = GetNumXPagesAtDepth(depth);
			const u32 OutputDimension = (depth != 0)?GetNumXPagesAtDepth(depth-1):0;
			const u32 nXInputBlocks = ((InputDimension-1)/maxInputBlockDimension) +1;
			const u32 nYInputBlocks = ((InputDimension-1)/maxInputBlockDimension) +1;

			
			const u64 nBlocksForTick= nYInputBlocks*nXInputBlocks/10+1;
			u64 nCompletedBlocks = 0;
			
			std::vector<Color> inputBuffer;
			std::vector<Color> outputBuffer;
			std::vector<bool> inputPageStatus;
			std::vector<bool> outputPageStatus;
			inputBuffer.resize(maxInputPages*GetPageOuterPixelCount());
			outputBuffer.resize(maxOutputPages*GetPageInnerPixelCount());
			inputPageStatus.resize(maxInputPages);
			outputPageStatus.resize(maxOutputPages);

			
			for(u32 uYBlock = 0; uYBlock < nYInputBlocks; uYBlock++)
				for(u32 uXBlock = 0; uXBlock < nXInputBlocks; uXBlock++)
				{
					nCompletedBlocks++;
					if(nCompletedBlocks % nBlocksForTick == 0)
						printf(".");


					const u32 nYInputPages = std::min((InputDimension - uYBlock*maxInputBlockInnerDimension),maxInputBlockInnerDimension) + 2;
					const u32 nXInputPages = std::min((InputDimension - uXBlock*maxInputBlockInnerDimension),maxInputBlockInnerDimension) + 2;
					const u32 nYOutputPages = (nYInputPages-2)/2;
					const u32 nXOutputPages = (nXInputPages-2)/2;
					const u32 nInputPages = nYInputPages*nXInputPages;
					const u32 nOutputPages = nYOutputPages*nXOutputPages;
					const u32 nYInputPixels = nYInputPages*GetPageOuterSize();
					const u32 nXInputPixels = nXInputPages*GetPageOuterSize();
					const u32 nYOutputPixels = nYOutputPages*GetPageInnerSize();
					const u32 nXOutputPixels = nXOutputPages*GetPageInnerSize();
					
					//read input pages
					i32 iXBegin = (i32)(uXBlock*maxInputBlockInnerDimension) - 1;
					i32 iYBegin = (i32)(uYBlock*maxInputBlockInnerDimension) - 1;
					if(MergePagesToImage(
						iXBegin,iXBegin+nXInputPages,
						iYBegin,iYBegin+nYInputPages,
						depth,inputBuffer.data(),&inputPageStatus,PAGE_FINAL))
					{
						//char temp[256];
						//sprintf(temp,"Input_%i_%i_%i.png",depth,uXBlock,uYBlock);
						//OutputImage(temp,inputBuffer.data(),nXInputPages*GetPageOuterSize(),nYInputPages*GetPageOuterSize());


						BorderedImageReader imageReader(inputBuffer.data(),inputPageStatus,nXInputPages,nYInputPages,GetPageOuterSize(),GetPageInnerSize());
					
						if(nOutputPages)
						{
							//downsample pageStatus
							bool bHasOne = false;
							for(u32 iy = 0; iy < nYOutputPages;iy++)
								for(u32 ix = 0; ix < nXOutputPages;ix++)
								{
									outputPageStatus[iy*nXOutputPages+ix] = 
										(inputPageStatus[(2*iy+0 +1)*nXInputPages + (2*ix+0 +1)]) ||
										(inputPageStatus[(2*iy+0 +1)*nXInputPages + (2*ix+1 +1)]) ||
										(inputPageStatus[(2*iy+1 +1)*nXInputPages + (2*ix+0 +1)]) ||
										(inputPageStatus[(2*iy+1 +1)*nXInputPages + (2*ix+1 +1)]);
									if(outputPageStatus[iy*nXOutputPages+ix])
										bHasOne = true;
								}

							//process mipmaps
							if(bHasOne)
							{
								applyBoxFilter(
									imageReader,GetPageInnerSize(),
									outputBuffer.data(),nXOutputPages*GetPageInnerSize(),nYOutputPages*GetPageInnerSize()
									);
								
								//sprintf(temp,"Output_%i_%i_%i.png",depth,uXBlock,uYBlock);
								//OutputImage(temp,outputBuffer.data(),nXOutputPages*GetPageInnerSize(),nYOutputPages*GetPageInnerSize());
					
								//write output pages
								SplitPagesFromImage(
									(i64)(uXBlock*maxOutputBlockDimension),(i64)(uXBlock*maxOutputBlockDimension)+nXOutputPages,
									(i64)(uYBlock*maxOutputBlockDimension),(i64)(uYBlock*maxOutputBlockDimension)+nYOutputPages,
									depth-1,outputBuffer.data(),&outputPageStatus);
							}
						}

						//generate border
						generateBorders(inputBuffer.data(),imageReader);

						for(u32 iy = 0 ; iy < nYInputPages-2; iy++)
								for(u32 ix = 0 ; ix < nXInputPages-2; ix++)
									inputPageStatus[iy*(nXInputPages-2) + ix] = inputPageStatus[(iy+1)*nXInputPages + ix+1];
						
						//sprintf(temp,"Bordered_%i_%i_%i.png",depth,uXBlock,uYBlock);
						//OutputImage(temp,inputBuffer.data(),nXInputPages*GetPageOuterSize(),nYInputPages*GetPageOuterSize());

						//write input (with borders) back to file
						u64 stride = nXInputPixels;
						u64 offset = GetPageOuterSize()*stride + GetPageOuterSize();
						SplitPagesFromImage(
								iXBegin+1,iXBegin+(i64)nXInputPages-1,
								iYBegin+1,iYBegin+(i64)nYInputPages-1,
								depth,inputBuffer.data(),&inputPageStatus,PAGE_FINAL,offset,stride);

						/*MergePagesToImage(
							iXBegin,iXBegin+(i64)nXInputPages,
							iYBegin,iYBegin+(i64)nYInputPages,
							depth,inputBuffer.data(),&inputPageStatus,PAGE_FINAL);

						
						sprintf(temp,"Check_%i_%i_%i.png",depth,uXBlock,uYBlock);
						OutputImage(temp,inputBuffer.data(),(nXInputPages)*GetPageOuterSize(),(nYInputPages)*GetPageOuterSize());*/
					}
				}
				
			printf("done\n",depth);
			return true;
		}

		
		static void generateBorders(
			Color* pDataOut,	const BorderedImageReader& imageReader)
		{
			Color Red;
			Red.r = 0xff;
			Red.g=0;
			Red.b=0;
			Red.a=0;

			for(u32 yPage = 1; yPage<imageReader.GetPagesY()-1;yPage++)
				for(u32 xPage = 1; xPage<imageReader.GetPagesX()-1;xPage++)
				{
					for(u32 yPixel = 0; yPixel < imageReader.GetPageBorder(); yPixel++)
						for(u32 xPixel = 0; xPixel < imageReader.GetPageOuter(); xPixel++)
							{
								u32 yOut = (yPixel+yPage*imageReader.GetPageOuter());
								u32 xOut = (xPixel+xPage*imageReader.GetPageOuter());

								pDataOut[(u64)yOut*(u64)imageReader.GetOuterX()+(u64)xOut] = imageReader.GetPixelFromOuter(xPage,yPage,xPixel,yPixel);
							}

					for(u32 yPixel = imageReader.GetPageBorder(); yPixel < imageReader.GetPageOuter()-imageReader.GetPageBorder(); yPixel++)
					{
						for(u32 xPixel = 0; xPixel < imageReader.GetPageBorder(); xPixel++)
						{
							u32 yOut = (yPixel+yPage*imageReader.GetPageOuter());
							u32 xOut = (xPixel+xPage*imageReader.GetPageOuter());

							pDataOut[(u64)yOut*(u64)imageReader.GetOuterX()+(u64)xOut] = imageReader.GetPixelFromOuter(xPage,yPage,xPixel,yPixel);
						}
						for(u32 xPixel = imageReader.GetPageOuter()-imageReader.GetPageBorder(); xPixel < imageReader.GetPageOuter(); xPixel++)
						{
							u32 yOut = (yPixel+yPage*imageReader.GetPageOuter());
							u32 xOut = (xPixel+xPage*imageReader.GetPageOuter());

							pDataOut[(u64)yOut*(u64)imageReader.GetOuterX()+(u64)xOut] = imageReader.GetPixelFromOuter(xPage,yPage,xPixel,yPixel);
						}
					}
					
					for(u32 yPixel = imageReader.GetPageOuter()-imageReader.GetPageBorder(); yPixel < imageReader.GetPageOuter(); yPixel++)
						for(u32 xPixel = 0; xPixel < imageReader.GetPageOuter(); xPixel++)
							{
								u32 yOut = (yPixel+yPage*imageReader.GetPageOuter());
								u32 xOut = (xPixel+xPage*imageReader.GetPageOuter());

								pDataOut[(u64)yOut*(u64)imageReader.GetOuterX()+(u64)xOut] = imageReader.GetPixelFromOuter(xPage,yPage,xPixel,yPixel);
							}
				}
		}
				
	};

	}
#endif