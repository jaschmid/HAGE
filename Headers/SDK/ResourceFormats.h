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
			virtual u64 Read(u64 nReadMax,u8* pReadOut) = 0;
			virtual u64 Seek(i64 location,ORIGIN origin) = 0;
			virtual void Close() = 0;
			virtual bool IsValid() const = 0;
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
			virtual IResource* Finalize(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut) = 0;
	};

	class IStreamingResourceProvider : protected DomainMember<ResourceDomain>
	{
		public:
			virtual ~IStreamingResourceProvider(){}
			virtual bool ProcessResourceAccess(IResource* feedback) =0;
			virtual bool CheckFeedbackBufferReady(IResource* feedback) =0;
			virtual void CreateResourceAccessSet(std::vector<IResource*>& accesses) =0;
			virtual bool QueryDependancies(const ResourceAccess* dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut) = 0;
	};
	
	class IStreamingFeedbackProvider : public IResource
	{
	public:
	};
	
	class IVirtualTexture : public IStreamingFeedbackProvider
	{
	public:
		//static data retrievel, does not change within one frame but results might change from frame to frame
		virtual const APIWTexture* GetCurrentVTCache() const = 0;
		virtual const APIWTexture* GetCurrentVTRedirection() const = 0;
		virtual u32 GetCacheSize() const = 0; //returns current cache size (in bytes), note that adjusting it will not immediately change this

		//debug
		virtual const APIWTexture* _Debug_GetFeedbackTexture() const = 0;
		virtual const APIWConstantBuffer* GetSettings() const = 0;
		
		//various functions to provide feedback
		virtual void AdjustCacheSize(u32 newSize) const = 0; //enters a request to adjust the cache size
		virtual u32 GetId() const = 0;
		virtual APIWEffect* BeginFeedback(RenderingAPIWrapper* pRenderer) const =0;
		virtual void EndFeedback(RenderingAPIWrapper* pRenderer) const = 0;
	};

	class IImageData : public IResource
	{
	public:
		typedef enum
		{
			R8G8B8,
			R8G8B8A8,
			DXTC1,
			DXTC3,
			DXTC5
		} FORMAT;

		virtual ~IImageData(){}
		virtual u32 GetImageWidth() const =0;
		virtual u32 GetImageHeight() const =0;
		virtual u32 GetImageFormat() const =0;
		virtual u32 GetImageLevels() const =0;
		virtual const void*	GetImageData() const =0;
	};

	class IMeshData : public IResource
	{
	public:
		typedef enum _VERTEX_DATA_TYPE
		{
			POSITION,
			NORMAL,
			COLOR,
			TEXCOORD0,
			TEXCOORD1,
			TEXCOORD2,
			TEXCOORD3,
			BLEND_WEIGHTS_VEC3
		} VERTEX_DATA_TYPE;
		typedef enum _EXTENDED_DATA_TYPE
		{
			TRIANGLE_MATERIAL_INFO,
			CHILD_COUNT,
			CHILD_LIST,
			RESERVED
		} EXTENDED_DATA_TYPE;
		
		struct ChildObject
		{
			TResourceAccess<IMeshData>	childMesh;
			Matrix4<>					transformation;
			//Vector3<>					scale;
			//Quaternion<>				rotation;
		};

		virtual ~IMeshData(){}
		virtual u32 GetNumVertices() const =0;
		virtual u32 GetVertexData(const u8** pDataOut,u32& pOutStride,VERTEX_DATA_TYPE type) const =0;
		virtual u32	GetNumIndices() const =0;
		virtual u32 GetIndexData(const u8** pDataOut) const =0;
		virtual const void* GetExtendedData(EXTENDED_DATA_TYPE type) const = 0;
		virtual const char* GetTextureName(u32 index) const = 0;
		virtual const TResourceAccess<IImageData>* GetTexture(u32 index) const = 0;
	};

	class IRawData: public IResource
	{
	public:
		virtual ~IRawData(){}
		virtual u64 GetSize() const = 0;
		virtual u64 GetData(const u8** pDataOut) const = 0;
	};

	class IDrawableMesh : public IResource
	{
	public:
		virtual ~IDrawableMesh(){}
		virtual const APIWVertexArray* GetVertexArray() const = 0;
		virtual const APIWTexture* GetTexture(u32 index) const = 0;
		virtual void Draw(const Vector3<>& position, const Matrix4<>& view,const Matrix4<>& inv_view, const Matrix4<>& proj,const Vector3<>& lPosition, const Vector3<>& lColor) const = 0;
	};

	class ITextureImage : public IResource
	{
	public:
		virtual ~ITextureImage(){}
		virtual const APIWTexture* GetTexture() const = 0;
	};

	
	DECLARE_CLASS_GUID(IVirtualTexture,	0x0549a888,0xb591,0x497f,0xbb2c,0x5a639476da5b);
	DECLARE_CLASS_GUID(IDrawableMesh,	0x12d47f0e,0x8813,0x4ad5,0xb13d,0x916b7ab4a618);
	DECLARE_CLASS_GUID(IMeshData,		0xcf361bc0,0xf572,0x4cc9,0xbc2f,0x547537da68a8);
	DECLARE_CLASS_GUID(ITextureImage,	0x6353c6b2,0x1018,0x436f,0xb67b,0x6c2e69f7a931);
	DECLARE_CLASS_GUID(IImageData,		0x490ab3bf,0xa632,0x42f8,0xa003,0x4fdd4b338ef8);
	DECLARE_CLASS_GUID(IRawData,		0xf8abf156,0x9c54,0x431c,0x9c66,0x8c46b72e12c8);
}

#endif