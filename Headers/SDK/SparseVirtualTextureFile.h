/********************************************************/
/* FILE: SparseVirtualTextureFile.h                     */
/* DESCRIPTION: Declares a class intended to aid in the */
/*              usage of Spars Virtual Texture Files    */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __SPARSE_VIRTUAL_TEXTURE_FILE__
#define __SPARSE_VIRTUAL_TEXTURE_FILE__


namespace HAGE {

	class SparseVirtualTextureFile
	{
	public:
		typedef enum _PageFlags
		{
			PAGE_FINAL = 0x10000000
		} PAGE_FLAGS;

		// create a new sparse virtual texture file, virtualsize is in # of pages per dimension (pixel size = ((2<<(pagedepth-1)) * virtualsize) ^2 )
		SparseVirtualTextureFile();
		~SparseVirtualTextureFile();
		bool New(char* FileName, u32 pagesize,u32 pagedepth, u32 bordersize);
		bool Open(char* FileName);
		bool Open(IDataStream* pDataStream);
		void WritePage(u64 PageIndex,const void* pData, u32 flags = 0);
		bool ReadPage(u64 PageIndex,void* pDataOut,u32 flags = 0);
		void WriteImageToVirtualTexture(u64 vtXOffset, u64 vtYOffset,u64 ImageX,u64 ImageY,const void* DataIn,int level = -1);
		void ReadImageFromVirtualTexture(u64 vtXOffset, u64 vtYOffset,u64 ImageX,u64 ImageY,void* pDataOut,int level = -1);
		void Commit();

		//some simple inline functions

		u64 GetPageIndex(u64 xLocation,u64 yLocation,u32 depth) const
		{
			if(depth == 0)
			{
				assert(xLocation == 0);
				assert(yLocation == 0);
				return 0;
			}
			else
			{
				u64 result = GetFirstSubPage(GetPageIndex(xLocation/2,yLocation/2,depth-1));
				if(xLocation%2)
					result += 1;
				if(yLocation%2)
					result +=2;
				return result;
			}
		}

		inline u32 GetSize() const
		{
			return GetPageInnerSize() * GetNumXPagesAtDepth(fileHeader.pagedepth-1);
		}

		inline u64 GetTotalNumPages() const
		{
			return pageHeaders.size();
		}

		inline u64 GetMaxDepthPages() const
		{
			return 1i64 << (((u64)fileHeader.pagedepth-1i64)*2i64);
		}

		inline u32 GetPageInnerSize() const
		{
			return fileHeader.pagesize-2*fileHeader.bordersize;
		}
		inline u32 GetPageInnerPixelCount() const
		{
			return GetPageInnerSize()*GetPageInnerSize();
		}

		inline u32 GetPageOuterSize() const
		{
			return fileHeader.pagesize;
		}

		inline u32 GetPageOuterPixelCount() const
		{
			return GetPageOuterSize()*GetPageOuterSize();
		}

		inline u32 GetMaxDepth() const
		{
			return fileHeader.pagedepth;
		}

		inline bool IsWriteEnabled() const
		{
			return (tempFile != nullptr);
		}
		
		inline static u64 GetFirstSubPage(u64 page)
		{
			return page*4+1;
		}
		inline static u64 GetParentPage(u64 page)
		{
			return (page-1)/4;
		}
		inline static u64 GetFirstPageOfSubpageBlock(u64 page)
		{
			return GetFirstSubPage(GetParentPage(page));
		}
		
		inline static u32 GetNumXPagesAtDepth(u32 pagedepth)
		{
			return 1 << (pagedepth);
		}
		inline static u64 GetNumPagesAtDepth(u32 pagedepth)
		{
			return 1i64 << ((u64)pagedepth*2i64);
		}

		inline static u64 GetTotalNumPagesUntilDepth(u32 pagedepth)
		{
			u64 numPages = 0;
			for(int i = 0 ; i<(int)pagedepth+1;++i)
				numPages |= GetNumPagesAtDepth(i);
			return numPages;
		}

		inline static u64 GetFirstPageAtDepth(u32 pagedepth)
		{
			if(pagedepth == 0)
				return 0;
			return GetTotalNumPagesUntilDepth(pagedepth -1);
		}

	private:
	
		void stripBorder(const u32* pIn,u32* pOut) const;
		void blackBorder(const u32* pIn,u32* pOut) const;

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
			u32 page_size;
			u64 file_offset;
		};

		FILE* tempFile;
		IDataStream* dataStream;

		bool bOpen;
		bool bLastWrite;

		FileHeader fileHeader;
		std::vector<PageHeader> pageHeaders;
	};
}

#endif