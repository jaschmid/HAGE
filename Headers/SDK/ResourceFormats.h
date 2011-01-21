/********************************************************/
/* FILE: ResourceFormats.h                              */
/* DESCRIPTION: Defines various Resource Fromats for    */
/*              usage with the HAGE Engine              */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef __RESOURCE_FORMATS_H__
#define __RESOURCE_FORMATS_H__

namespace HAGE {
	class IDataStream
	{
		public:
			typedef enum _ORIGIN
			{
				ORIGIN_BEGINNING,
				ORIGIN_CURRENT,
				ORIGIN_END
			} ORIGIN;
			virtual std::string GetIdentifierString() const = 0;
			virtual u64 Read(u64 nReadMax,u8* pReadOut) const = 0;
			virtual u64 Seek(i64 location,ORIGIN origin) const = 0;
			virtual void Close() = 0;
	};

	class IResource
	{
		public:
			virtual ~IResource(){}
	};

	class IResourceLoader  : protected DomainMember<ResourceDomain>
	{
		public:
			virtual ~IResourceLoader(){}
			virtual u32 GetDependancies(const std::pair<std::string,guid>** pDependanciesOut) = 0;
			virtual IResource* Finalize(const IResource** dependanciesIn,u32 nDependanciesIn) = 0;
	};

	class IMeshData
	{
	public:
		virtual ~IMeshData(){}
		virtual u32 GetNumVertices() const =0;
		virtual u32 GetVertexSize() const =0;
		virtual u32 GetVertexData(const u8** pDataOut) const =0;
		virtual u32	GetNumIndices() const =0;
		virtual u32 GetIndexData(const u8** pDataOut) const =0;
	};

	class IImageData
	{
	public:
		typedef enum
		{
			R8G8B8,
			R8G8B8A8
		} FORMAT;

		virtual ~IImageData(){}
		virtual u32 GetImageWidth() const =0;
		virtual u32 GetImageHeight() const =0;
		virtual u32 GetImageFormat() const =0;
		virtual const void*	GetImageData() const =0;
	};

	class IDrawableMesh
	{
	public:
		virtual ~IDrawableMesh(){}
		virtual const APIWVertexArray* GetVertexArray() const = 0;
		virtual void Draw(const Vector3<>& position, const Matrix4<>& view,const Matrix4<>& inv_view, const Matrix4<>& proj,const Vector3<>& lPosition, const Vector3<>& lColor) const = 0;
	};
	class ITextureImage
	{
	public:
		virtual ~ITextureImage(){}
		virtual const APIWTexture* GetTexture() const = 0;
	};

	class CDrawableMeshLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CDrawableMeshLoader(IDataStream* pStream,const IResource* pPrev);
		~CDrawableMeshLoader();
		u32 GetDependancies(const std::pair<std::string,guid>** pDependanciesOut);
		IResource* Finalize(const IResource** dependanciesIn,u32 nDependanciesIn);
	private:

		class CDrawableMesh : public IDrawableMesh
		{
		public:
			void Draw(const Vector3<>& position, const Matrix4<>& view,const Matrix4<>& inv_view, const Matrix4<>& proj,const Vector3<>& lPosition, const Vector3<>& lColor)  const;
			const APIWVertexArray* GetVertexArray() const{return _pVertexArray;}
			virtual ~CDrawableMesh();
			CDrawableMesh(const IMeshData* pData);
		private:

			APIWVertexBuffer*							_pVertexBuffer;
			APIWVertexArray*							_pVertexArray;
			APIWConstantBuffer*							_pConstants;
			APIWEffect*									_pEffect;
		};

		std::pair<std::string,guid>*				_pDependancies;

		CDrawableMesh* _pDrawableMesh;
	};
	class CTextureImageLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CTextureImageLoader(IDataStream* pStream,const IResource* pPrev);
		~CTextureImageLoader();
		u32 GetDependancies(const std::pair<std::string,guid>** pDependanciesOut);
		IResource* Finalize(const IResource** dependanciesIn,u32 nDependanciesIn);
	private:

		class CTextureImage : public ITextureImage
		{
		public:
			virtual ~CTextureImage();
			CTextureImage(const IImageData* pData);
			virtual const APIWTexture* GetTexture() const;
		private:

			APIWTexture*							_pTexture;
		};

		std::pair<std::string,guid>*				_pDependancies;

		CTextureImage* _pTextureImage;
	};

	class CMeshDataLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CMeshDataLoader(IDataStream* pStream,const IResource* pPrev);
		~CMeshDataLoader();
		u32 GetDependancies(const std::pair<std::string,guid>** pDependanciesOut);
		IResource* Finalize(const IResource** dependanciesIn,u32 nDependanciesIn);
	private:

		class CMeshData : public IMeshData
		{
		public:
			virtual u32 GetNumVertices() const;
			virtual u32 GetVertexSize() const;
			virtual u32 GetVertexData(const u8** pDataOut) const;
			virtual u32	GetNumIndices() const;
			virtual u32 GetIndexData(const u8** pDataOut) const;
			virtual ~CMeshData();
			CMeshData(const IDataStream* pData);
		private:

			u32		_vertexSize;
			u32		_nVertices;
			u32		_nIndices;
			u8*		_pVertexData;
			u8*		_pIndexData;
		};

		CMeshData* _pMeshData;
	};
		
	class CImageDataLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CImageDataLoader(IDataStream* pStream,const IResource* pPrev);
		~CImageDataLoader();
		u32 GetDependancies(const std::pair<std::string,guid>** pDependanciesOut);
		IResource* Finalize(const IResource** dependanciesIn,u32 nDependanciesIn);
	private:

		class CImageData : public IImageData
		{
		public:		
			virtual u32 GetImageWidth() const;
			virtual u32 GetImageHeight() const;
			virtual u32 GetImageFormat() const;
			virtual const void*	GetImageData() const;
			virtual ~CImageData();
			CImageData(const IDataStream* pData);
		private:
			u32		_Width;
			u32		_Height;
			u32		_Format;
			void*	_Data;
		};

		CImageData* _pImageData;
	};
	DECLARE_CLASS_GUID(IDrawableMesh,	0x12d47f0e,0x8813,0x4ad5,0xb13d,0x916b7ab4a618);
	DECLARE_CLASS_GUID(IMeshData,		0xcf361bc0,0xf572,0x4cc9,0xbc2f,0x547537da68a8);
	DECLARE_CLASS_GUID(ITextureImage,	0x6353c6b2,0x1018,0x436f,0xb67b,0x6c2e69f7a931);
	DECLARE_CLASS_GUID(IImageData,		0x490ab3bf,0xa632,0x42f8,0xa003,0x4fdd4b338ef8);
}

#endif