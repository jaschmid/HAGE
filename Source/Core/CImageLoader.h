/********************************************************/
/* FILE: CImageLoader.h                                 */
/* DESCRIPTION: Defines various Resource Fromats for    */
/*              usage with the HAGE Engine              */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/


#ifndef __INTERNAL_IMAGE_LOADER_H__
#define __INTERNAL_IMAGE_LOADER_H__

namespace HAGE {

	class CTextureImageLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CTextureImageLoader(IDataStream* pStream,const IResource* pPrev);
		~CTextureImageLoader();
		IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
	private:

		class CTextureImage : public ITextureImage
		{
		public:
			virtual ~CTextureImage();
			CTextureImage(const TResourceAccess<IImageData>& pData);
			virtual const APIWTexture* GetTexture() const;
		private:

			APIWTexture*							_pTexture;
		};

		std::pair<std::string,guid>*				_pDependancies;

		CTextureImage* _pTextureImage;
	};

	
	class CImageDataLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CImageDataLoader(IDataStream* pStream,const IResource* pPrev);
		~CImageDataLoader();
		IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
	private:

		class CImageData : public IImageData
		{
		public:		
			virtual u32 GetImageWidth() const;
			virtual u32 GetImageHeight() const;
			virtual u32 GetImageFormat() const;
			virtual u32 GetImageLevels() const;
			virtual const void*	GetImageData() const;
			virtual ~CImageData();
			CImageData(IDataStream* pData);
		private:

			bool TryLoadBLP(IDataStream* pStream);

			u32		_Width;
			u32		_Height;
			u32		_Levels;
			u32		_Format;
			void*	_Data;
		};

		CImageData* _pImageData;
	};
	
}

#endif