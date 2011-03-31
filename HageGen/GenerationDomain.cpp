#include "header.h"

#include "GenerationDomain.h"

#define FREEIMAGE_LIB
#include <FreeImage.h>

namespace HAGE {

	class SparseVirtualTextureFile
	{
	public:
		typedef enum _PageFlags
		{
			PAGE_FINAL = 0x10000000
		} PAGE_FLAGS;

		// create a new sparse virtual texture file, virtualsize is in # of pages per dimension (pixel size = ((2<<(pagedepth-1)) * virtualsize) ^2 )
		SparseVirtualTextureFile(char* workingFile, u32 pagesize,u32 pagedepth, u32 bordersize)
		{

			fileHeader.fourCC[0] = 'H';
			fileHeader.fourCC[1] = 'S';
			fileHeader.fourCC[2] = 'V';
			fileHeader.fourCC[3] = 'T';
			fileHeader.bordersize = bordersize;
			fileHeader.pagedepth = pagedepth;
			fileHeader.majorVersion = 1;
			fileHeader.minorVersion = 0;
			fileHeader.pagesize = pagesize;
			u64 nPages = getTotalNumPages(fileHeader.pagedepth);
			pageHeaders.resize(nPages);
			for(int i = 0; i<nPages; ++i)
			{
				pageHeaders[i].file_offset = 0;
				pageHeaders[i].page_flags = 0;
			}

			tempFile = fopen(workingFile,"w+b");
			fwrite(&fileHeader,sizeof(FileHeader),1,tempFile);
			fwrite(pageHeaders.data(),sizeof(PageHeader),pageHeaders.size(),tempFile);
			fflush(tempFile);
		}

		//open svtf for editing
		SparseVirtualTextureFile(char* fileToOpen,char* workingFile)
		{

			assert(!"Not supported yet");
		}

		// open existing Sparse Virtual Texture File from data stream (read only)
		SparseVirtualTextureFile(IDataStream* stream)
		{
			assert(!"Not supported yet");
		}

		~SparseVirtualTextureFile()
		{
			fclose(tempFile);
		}

		void WritePage(u64 PageIndex,const void* pData, u32 flags = 0)
		{
			pageHeaders[PageIndex].page_flags = flags;

			u32 bytes,padding;

			if(flags& PAGE_FINAL)
			{
				bytes = getPageSize()*getPageSize()*sizeof(u32);
				padding = 0;
			}
			else
			{
				bytes = sizeof(u32)*GetPageWritePixelCount();;
				padding = getPageSize()*getPageSize()*sizeof(u32) - bytes;
			}

			if(pageHeaders[PageIndex].file_offset != 0)
			{
				//already in tempfile overwrite

				
				fseek(tempFile,pageHeaders[PageIndex].file_offset,SEEK_SET);
				fwrite(pData,bytes,1,tempFile);
				if(padding)
					fwrite(pData,padding,1,tempFile);
				fflush(tempFile);
			}
			else
			{
				fseek(tempFile,1,SEEK_END);
				pageHeaders[PageIndex].file_offset = ftell(tempFile);
				fwrite(pData,bytes,1,tempFile);
				if(padding)
					fwrite(pData,padding,1,tempFile);
				fflush(tempFile);
			}
		}

		bool ReadPage(u64 PageIndex,void* pDataOut,u32 flags = 0)
		{
			
			u32 bytes,padding;
			void* pWriteTarget = pDataOut;

			if(pageHeaders[PageIndex].page_flags & PAGE_FINAL)
			{
				bytes = getPageSize()*getPageSize()*sizeof(Color);
				padding = 0;

				if(!flags & PAGE_FINAL)
					pWriteTarget = new Color[bytes/sizeof(Color)];

			}
			else
			{
				bytes = sizeof(u32)*GetPageWritePixelCount();
				padding = getPageSize()*getPageSize()*sizeof(u32) - bytes;

				if(flags & PAGE_FINAL)
					pWriteTarget = new Color[bytes/sizeof(Color)];
			}

			bool bResult = true;

			if(pageHeaders[PageIndex].file_offset != 0)
			{
				//already in tempfile overwrite
				
				fseek(tempFile,pageHeaders[PageIndex].file_offset,SEEK_SET);
				u32 read=fread(pWriteTarget,1,bytes,tempFile);
				assert(read == bytes);
				fflush(tempFile);
				bResult = true;
			}
			else
			{
				memset(pWriteTarget,0,bytes);
				bResult = false;
			}

			if(pWriteTarget != pDataOut)
			{
				
				if(!flags & PAGE_FINAL)
					stripBorder((const Color*)pWriteTarget,(Color*)pDataOut);
				else if(flags & PAGE_FINAL)
					blackBorder((const Color*)pWriteTarget,(Color*)pDataOut);
				else
					assert(!"Huh?");

				delete pWriteTarget;
			}

			return bResult;
		}


		void Commit()
		{
			//make mipmaps
			GenerateMipPageDepth(0);

			//debug stuff
			// without borders
			for(int i = 0; i< fileHeader.pagedepth;++i)
			{
				std::vector<u32> buffer;
				u64 xSize = GetPageWriteSize()*getNumXPagesAtDepth(i);
				buffer.resize(GetPageWritePixelCount()*getNumPagesAtDepth(i));

				MergePagesToImage(
							0,getNumXPagesAtDepth(i),
							0,getNumXPagesAtDepth(i),
							i,(Color*)buffer.data(),0);

				FIBITMAP* b =FreeImage_Allocate(xSize,xSize,24);
				for(u64 iy = 0; iy < xSize;iy++)
					for(u64 ix = 0; ix < xSize;ix++)
						FreeImage_SetPixelColor(b,ix,iy,(RGBQUAD*)&buffer[iy*xSize+ix]);
				char temp[256];
				sprintf(temp,"MipLevel%i.png",i);

				BOOL res = FreeImage_Save(FIF_PNG,b,temp);
				FreeImage_Unload(b);
			}

			//with borders
			for(int i = 0; i< fileHeader.pagedepth;++i)
			{
				std::vector<u32> buffer;
				u64 xSize = getPageSize()*getNumXPagesAtDepth(i);
				buffer.resize(xSize*xSize);

				MergePagesToImage(
							0,getNumXPagesAtDepth(i),
							0,getNumXPagesAtDepth(i),
							i,(Color*)buffer.data(),PAGE_FINAL);

				FIBITMAP* b =FreeImage_Allocate(xSize,xSize,24);
				for(u64 iy = 0; iy < xSize;iy++)
					for(u64 ix = 0; ix < xSize;ix++)
						FreeImage_SetPixelColor(b,ix,iy,(RGBQUAD*)&buffer[iy*xSize+ix]);
				char temp[256];
				sprintf(temp,"MipLevel_borders_%i.png",i);

				BOOL res = FreeImage_Save(FIF_PNG,b,temp);
				FreeImage_Unload(b);
			}
		}

		u64 GetTotalNumPages() const
		{
			return pageHeaders.size();
		}

		u64 GetMaxDepthPages() const
		{
			return 1 << (((u64)fileHeader.pagedepth-1)*2);
		}

		u32 GetPageWriteSize() const
		{
			return fileHeader.pagesize-2*fileHeader.bordersize;
		}
		u32 GetPageWritePixelCount() const
		{
			return GetPageWriteSize()*GetPageWriteSize();
		}

		u32 GetPageReadSize() const
		{
			return fileHeader.pagesize;
		}
		
		struct Color
		{
			u8 r,g,b,a;
		};
	private:


		u64 getPage(u64 xLocation,u64 yLocation,u32 depth)
		{
			if(depth == 0)
			{
				assert(xLocation == 0);
				assert(yLocation == 0);
				return 0;
			}
			else
			{
				u64 result = getFirstSubPage(getPage(xLocation/2,yLocation/2,depth-1));
				if(xLocation%2)
					result += 1;
				if(yLocation%2)
					result +=2;
				return result;
			}
		}

		static void applyBoxFilter(const Color* pInput,u64 xInputSize,u64 yInputSize,u64 InputBorder,
			Color* pOutput,u64 xOutputSize,u64 yOutputSize)
		{
			static const u64 FilterSize = 2;
			static const float base = 1.0f/4.0f;
			static const float base_filter[FilterSize][FilterSize] = 
			{
				base,base,
				base,base,
			};

			float trapez_filter[FilterSize+1][FilterSize+1];
			u64 trapez_size;

			//our trapez filter only supports even output sizes
			assert(xOutputSize%2 == 0);
			assert(yOutputSize%2 == 0);
				
			if(FilterSize%2 == 1)
			{
				trapez_size = FilterSize+1;
				for(int fy = 0; fy < trapez_size; ++fy)
					for(int fx = 0; fx < trapez_size; ++fx)
						trapez_filter[fy][fx] = 0.0f;

				for(int fy = 0; fy < FilterSize; ++fy)
					for(int fx = 0; fx < FilterSize; ++fx)
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
				for(int fy = 0; fy < trapez_size; ++fy)
					for(int fx = 0; fx < trapez_size; ++fx)
						trapez_filter[fy][fx] = base_filter[fy][fx];
			}
			
			/*
			float Filter[xFilterSize][yFilterSize] = {
				0.25f,0.25f,
				0.25f,1.0f/9.0f,1.0f/9.0f,
				base/2.0f,base,base,base/2.0f

			};*/

			assert( xInputSize == 2*InputBorder + 2*xOutputSize );
			assert( yInputSize == 2*InputBorder + 2*yOutputSize );

			for(int iy = 0; iy < yOutputSize; iy++)
				for(int ix = 0; ix < xOutputSize; ix++)
				{
					u64 xInput = InputBorder - (trapez_size-1)/2 + ix*2;
					u64 yInput = InputBorder - (trapez_size-1)/2 + iy*2;
					float red=0.0f,green=0.0f,blue=0.0f,alpha=0.0f;
					for(int fy = 0; fy < trapez_size; ++fy)
						for(int fx = 0; fx < trapez_size; ++fx)
						{
							const Color& c = pInput[(yInput+fy)*xInputSize + xInput+fx];
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
public:
	//temp
		void SplitPagesFromImage(i64 xPageBegin,u64 xPageEnd,u64 yPageBegin,i64 yPageEnd,u32 depth,const Color* pIn)
		{
			i64 nXPages = xPageEnd-xPageBegin;
			i64 nYPages = yPageEnd-yPageBegin;
			i64 nXPixels = nXPages*GetPageWriteSize();
			i64 NumXPages =		getNumXPagesAtDepth(depth);

			std::vector<Color> tempBuffer;
			tempBuffer.resize(GetPageWritePixelCount());
			
			for(i64 uYPage = 0; uYPage < nYPages; uYPage++)
				for(i64 uXPage = 0; uXPage < nXPages; uXPage++)
				{
					i64 iXAbsolute = (i64)(uXPage + xPageBegin) ;
					i64 iYAbsolute = (i64)(uYPage + yPageBegin) ;

					//magic wrap around

					iXAbsolute = (iXAbsolute+NumXPages)%NumXPages;
					iYAbsolute = (iYAbsolute+NumXPages)%NumXPages;

					//copy tempBuffer to inputBuffer

					for(int iYPixel = 0; iYPixel < (int)GetPageWriteSize(); iYPixel++)
						for(int iXPixel = 0; iXPixel < (int)GetPageWriteSize(); iXPixel++)
							tempBuffer[iYPixel * GetPageWriteSize() + iXPixel] = pIn[(iYPixel+uYPage*GetPageWriteSize())*nXPixels + iXPixel + uXPage*GetPageWriteSize()];

					
					WritePage(getPage(iXAbsolute,iYAbsolute,depth),tempBuffer.data());
				}
		}
private:
		void MergePagesToImage(i64 xPageBegin,i64 xPageEnd,i64 yPageBegin,i64 yPageEnd,u32 depth,Color* pOut,u32 flags)
		{
			i64 nPageSize = GetPageWriteSize();
			if(flags & PAGE_FINAL)
				nPageSize = getPageSize();
			i64 nPagePixels = nPageSize*nPageSize;
			i64 nXPages = xPageEnd-xPageBegin;
			i64 nYPages = yPageEnd-yPageBegin;
			i64 nXPixels = nXPages*nPageSize;
			i64 NumXPages =		getNumXPagesAtDepth(depth);

			std::vector<Color> tempBuffer;
			tempBuffer.resize(nPagePixels);

			//read input pages
			for(i64 uYPage = 0; uYPage < nYPages; uYPage++)
				for(i64 uXPage = 0; uXPage < nXPages; uXPage++)
				{
					i64 iXAbsolute = (i64)(uXPage + xPageBegin) ;
					i64 iYAbsolute = (i64)(uYPage + yPageBegin) ;

					//magic wrap around

					iXAbsolute = (iXAbsolute+NumXPages)%NumXPages;
					iYAbsolute = (iYAbsolute+NumXPages)%NumXPages;

					ReadPage(getPage(iXAbsolute,iYAbsolute,depth),tempBuffer.data(),flags);

					//copy tempBuffer to inputBuffer

					for(int iXPixel = 0; iXPixel < (int)nPageSize; iXPixel++)
						for(int iYPixel = 0; iYPixel < (int)nPageSize; iYPixel++)
							pOut[(iYPixel+uYPage*nPageSize)*nXPixels + iXPixel + uXPage*nPageSize] =
								tempBuffer[iYPixel * nPageSize + iXPixel];
				}
				
		}

		bool GenerateMipPageDepth(u32 depth) 
		{
			if(depth >= fileHeader.pagedepth -1)
				return true;

			GenerateMipPageDepth(depth+1);

			static const u64 maxMemoryForMipGeneration = 512*1024*1024; //512 MB for mip generation
			const u64 maxInputPages = (maxMemoryForMipGeneration*4/5)/GetPageWriteSize()/sizeof(Color)-1;
			const u64 maxInputBlockDimension = ((u64)sqrt((float)maxInputPages))/2*2;
			const u64 maxInputBlockInnerDimension = maxInputBlockDimension-2;
			const u64 maxOutputBlockDimension = maxInputBlockDimension/2;
			const u64 InputDimension = getNumXPagesAtDepth(depth+1);
			const u64 OutputDimension = getNumXPagesAtDepth(depth);
			const u64 nXInputBlocks = ((InputDimension-1)/maxInputBlockDimension) +1;
			const u64 nYInputBlocks = ((InputDimension-1)/maxInputBlockDimension) +1;

			for(u64 uYBlock = 0; uYBlock < nYInputBlocks; uYBlock++)
				for(u64 uXBlock = 0; uXBlock < nXInputBlocks; uXBlock++)
				{
					const u64 nYInputPages = std::min((InputDimension - uXBlock*maxInputBlockInnerDimension),maxInputBlockInnerDimension) + 2;
					const u64 nXInputPages = std::min((InputDimension - uYBlock*maxInputBlockInnerDimension),maxInputBlockInnerDimension) + 2;
					const u64 nYOutputPages = (nYInputPages-2)/2;
					const u64 nXOutputPages = (nXInputPages-2)/2;
					const u64 nInputPages = nYInputPages*nXInputPages;
					const u64 nOutputPages = nYOutputPages*nXOutputPages;
					const u64 nYInputPixels = nYInputPages*GetPageWriteSize();
					const u64 nXInputPixels = nXInputPages*GetPageWriteSize();
					const u64 nYOutputPixels = nYOutputPages*GetPageWriteSize();
					const u64 nXOutputPixels = nXOutputPages*GetPageWriteSize();

					//allocate buffers
					std::vector<Color> inputBuffer;
					std::vector<Color> outputBuffer;
					inputBuffer.resize(nInputPages*GetPageWritePixelCount());
					outputBuffer.resize(nOutputPages*GetPageWritePixelCount());

					//read input pages
					i64 iXBegin = (i64)(uXBlock*maxInputBlockInnerDimension) - 1;
					i64 iYBegin = (i64)(uYBlock*maxInputBlockInnerDimension) - 1;
					MergePagesToImage(
						iXBegin,iXBegin+(i64)nXInputPages,
						iYBegin,iYBegin+(i64)nYInputPages,
						depth+1,inputBuffer.data(),0);

					//process
					applyBoxFilter(
						inputBuffer.data(),nXInputPages*GetPageWriteSize(),nYInputPages*GetPageWriteSize(),GetPageWriteSize(),
						outputBuffer.data(),nXOutputPages*GetPageWriteSize(),nYOutputPages*GetPageWriteSize()
						);
					
					//write output pages
					SplitPagesFromImage(
						
						(i64)(uXBlock*maxOutputBlockDimension),(i64)(uXBlock*maxOutputBlockDimension)+nXOutputPages,
						(i64)(uXBlock*maxOutputBlockDimension),(i64)(uXBlock*maxOutputBlockDimension)+nYOutputPages,
						depth,outputBuffer.data());
					
				}

			return true;
		}
		
		void stripBorder(const Color* pIn,Color* pOut)
		{
		}
		void blackBorder(const Color* pIn,Color* pOut)
		{
		}

		u32 getPageSize() const
		{
			return fileHeader.pagesize;
		}
		
		static u64 getFirstSubPage(u64 page)
		{
			return page*4+1;
		}
		static u64 getParentPage(u64 page)
		{
			return (page-1)/4;
		}
		static u64 getFirstPageOfSubpageBlock(u64 page)
		{
			return getFirstSubPage(getParentPage(page));
		}
		
		static u64 getNumXPagesAtDepth(u32 pagedepth)
		{
			return 1 << ((u64)pagedepth);
		}
		static u64 getNumPagesAtDepth(u32 pagedepth)
		{
			return 1 << ((u64)pagedepth*2);
		}

		static u64 getTotalNumPages(u32 pagedepth)
		{
			u64 numPages = 0;
			for(int i = 0 ; i<(int)pagedepth+1;++i)
				numPages |= getNumPagesAtDepth(i);
			return numPages;
		}

		static u64 getFirstPageAtDepth(u32 pagedepth)
		{
			if(pagedepth == 0)
				return 0;
			return getTotalNumPages(pagedepth -1);
		}

		struct FileHeader
		{
			char fourCC[4];//HSVT
			u16 majorVersion; // 0 currently
			u16 minorVersion; // 1 currently
			u32 pagesize;
			u32 bordersize;
			u32 pagedepth; // how deep does the rabbit hole go?
		};

		struct PageHeader
		{
			u32 page_flags;
			u64 file_offset;
		};

		FILE* tempFile;
		FileHeader fileHeader;
		std::vector<PageHeader> pageHeaders;
	};

	class SparseVirtualTextureGenerator
	{
	public:
		//SparseVirtualTextureGenerator(,
	private:
	};



		GenerationDomain::GenerationDomain() 
		{
			printf("Init Generation\n");
			
			FreeImage_Initialise();
		}

		bool GenerationDomain::MessageProc(const Message* m)
		{
			return SharedDomainBase::MessageProc(m);
		}

		char* str_meshes[] = {
			"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_38_40.adt",
			"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_39_40.adt",
			"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_40_40.adt",
			"\0"
		};
		
		void GenerationDomain::writeOutputMesh()
		{
			printf("Writing...\n");
			FILE* f = fopen("landscape.hgeo","wb");
			char header[] = "HGEO";
			fwrite(header,4,1,f);

			_mesh.Compact();

			u32 nVertices = _mesh.GetNumVertices();
			u32 nPrimitives = _mesh.GetNumFaces();
			
			fwrite(&nVertices,sizeof(u32),1,f);
			for(int i = 0; i< nVertices; ++i)
			{
				Vector3<> out = _mesh.GetVertex(i);
				fwrite(&out,sizeof(Vector3<>),1,f);
				Vector2<> texcoord = Vector2<>((out.x-min.x)/(max.x-min.x),(out.z-min.z)/(max.z-min.z));
				fwrite(&texcoord,sizeof(Vector2<>),1,f);
			}
			
			fwrite(&nPrimitives,sizeof(u32),1,f);
			for(int i = 0; i< nPrimitives; ++i)
			{
				MeshType::VertexTriple vt = _mesh.GetFaceVertices(_mesh.GetFace(i));
				u32 index[3];
				for(int i2 = 0; i2 <3;++i2)
					index[i2] =	_mesh.GetIndex(vt[i2]);
				fwrite(index,3*sizeof(u32),1,f);
			}

			fclose(f);
			printf("Done Writing\n");

		}

		void GenerationDomain::mergeMeshVertices()
		{
			printf("Merging...\n");
			Vector3<> first = _mesh.GetVertexData(_mesh.GetVertex(0));
			min=first;max=first;
			for(int i = 1; i < _mesh.GetNumVertexIndices(); ++i)
			{
				Vector3<> v = _mesh.GetVertexData(_mesh.GetVertex(i));
				if(v.x < min.x)
					min.x = v.x;
				if(v.x > max.x)
					max.x = v.x;
				if(v.y < min.y)
					min.y = v.y;
				if(v.y > max.y)
					max.y = v.y;
				if(v.z < min.z)
					min.z = v.z;
				if(v.z > max.z)
					max.z = v.z;
			}

			typedef SpatialTree<MeshType::Vertex> myTree;

			myTree tree(myTree::SplitterType(min,max));

			Vector3<> test1 = _mesh.GetVertexData(_mesh.GetVertex(8));
			Vector3<> test2 = _mesh.GetVertexData(_mesh.GetVertex(9*9+8*8));
			for(int i = 0; i < _mesh.GetNumVertexIndices(); ++i)
			{
				MeshType::Vertex v = _mesh.GetVertex(i);
				if(v!=MeshType::nullVertex)
					tree.Insert(v);
			}

			const myTree::TreeNode* cur = tree.TopNode();
			while(cur)
			{
				if(cur->IsLeaf())
				{
					auto elements = cur->LeafElements();
					for(size_t i = 0; i < elements.size(); ++i)
						for(size_t i2 = i+1; i2 < elements.size(); ++i2)
						{
							MeshType::Vertex v1 = elements[i];
							MeshType::Vertex v2 = elements[i2];
							if(v1 != MeshType::nullVertex && v2 != MeshType::nullVertex)
							{
								Vector3<> pos1 = _mesh.GetVertexData(v1);
								Vector3<> pos2 = _mesh.GetVertexData(v2);
								float distance_sq = !( pos1 - pos2);
								if(distance_sq < 0.000001f)
								{
									assert(_mesh.MergeVertex(_mesh.MakePair(v1,v2)));
									break;
								}
							}
						}
				}
				cur = cur->GetNextNode();
			}
					
			_mesh.DebugValidateMesh();

			printf("vertices merged\n");
			printf("%i vertices\n",_mesh.GetNumVertices());
			printf("%i faces\n",_mesh.GetNumFaces());
			printf("%i edges\n",_mesh.GetNumEdges());
		}

		void GenerationDomain::DomainStep(t64 time)
		{
			static bool done = false;
			static bool load_requested = false;
			static char temp[256];
			static int numLoaded = 0;

			if(!load_requested)
			{
				for(int iy = yBegin; iy <= xEnd; ++iy)
					for(int ix = xBegin; ix <= xEnd; ++ix)
					{						
						sprintf(temp,"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_%i_%i.adt",ix,iy);
						data[ix-xBegin][iy-yBegin] = Resource->OpenResource<IMeshData>(temp);
						loaded[ix-xBegin][iy-yBegin] = false;
					}

				printf("creating HSVT\n");
				SparseVirtualTextureFile hsvt("temp.hsvt",128,4,4);

				FIBITMAP* b = FreeImage_Load(FIF_JPEG,"Dirty.jpg");

				const u32 size = 960;

				std::vector<u32> Buffer;
				Buffer.resize(size*size);

				for(int iy=0;iy<size;iy++)
					for(int ix=0;ix<size;ix++)
						FreeImage_GetPixelColor(b,ix,iy,(RGBQUAD*)&Buffer[iy*size+ix]);

				hsvt.SplitPagesFromImage(0,8,0,8,3,(SparseVirtualTextureFile::Color*)Buffer.data());

				hsvt.Commit();
				
				load_requested = true;
				done = true;
			}

			if(!done)
				for(int iy = yBegin; iy <= xEnd; ++iy)
					for(int ix = xBegin; ix <= xEnd; ++ix)
						if(!loaded[ix-xBegin][iy-yBegin] && data[ix-xBegin][iy-yBegin].getStage() > 0)
						{
				
							sprintf(temp,"@world.MPQ\\world\\maps\\Azeroth\\Azeroth_%i_%i.adt",ix,iy);
							TResourceAccess<IMeshData>& curr = data[ix-xBegin][iy-yBegin];
							loaded[ix-xBegin][iy-yBegin] = true;
							printf("data %s loaded!\n",temp);
							{
								const u8* pData;
								u32 nIndices = curr->GetIndexData(&pData);

								u32 preVertices = _mesh.GetNumVertexIndices();

								_mesh.ImportRawIndexData<MeshType::RAW_INDEX_TRIANGLE_LIST,u32>(nIndices/3,(const u32*)pData);

								const u8* positionData;
								u32 stride;
								u32 nVertices = curr->GetVertexData((const u8**)&positionData,stride,IMeshData::POSITION);

								Vector3<> offset = Vector3<>( (float)(ix - (xEnd + xBegin) / 2) * 2.0f, 0.0f , (float)(iy - (yEnd + yBegin) / 2) * 2.0f);

								for(int i = 0; i < nVertices; ++i)
									_mesh.GetVertexData(_mesh.GetVertex(i + preVertices)) = *(const Vector3<>*)&positionData[stride*i] + offset;
								printf("data loaded\n");
								printf("%i vertices\n",_mesh.GetNumVertices());
								printf("%i faces\n",_mesh.GetNumFaces());
								printf("%i edges\n",_mesh.GetNumEdges());

					
								_mesh.DebugValidateMesh();
							}

							numLoaded++;
							if(numLoaded == numToLoad)
							{
								mergeMeshVertices();
								writeOutputMesh();
								done = true;
							}
						}
			if(done)
				Tasks.Shutdown();
			//printf("Time %f, Elapsed Time: %f\n",GetTime().toSeconds(),GetElapsedTime().toSeconds());

		}

		GenerationDomain::~GenerationDomain()
		{
			FreeImage_DeInitialise();
			printf("Destroy GenerationDomain\n");
		}
}
