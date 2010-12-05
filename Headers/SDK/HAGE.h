/********************************************************/
/* FILE: HAGE.h                                         */
/* DESCRIPTION: HAGE Main Header                        */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef HAGE__MAIN__HEADER
#define HAGE__MAIN__HEADER

#include "preproc.h"
#include "types.h"
#include "results.h"
#include "messages.h"
#include "TLS.h"

#include "hage_math.h"

#include "interlocked_functions.h"

#include "IObject.h"
#include "IDomain.h"
#include "traits.h"

#include "global_allocator.h"

#include "MainClass.h"

#include "DomainBase.h"
#include "ObjectBase.h"
#include "PinHelpers.h"
#include "RenderingAPIWrapper.h"

namespace HAGE
{

	class IDataStream
	{
		public:
			virtual std::string GetIdentifierString() const = 0;
			virtual u64 Read(u64 nReadMax,u8* pReadOut) const = 0;
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

	class IDrawableMesh
	{
	public:
		virtual ~IDrawableMesh(){}
		virtual void Draw(const Vector3<>& position, const Matrix4<>& view, const Matrix4<>& proj) const = 0;
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
			void Draw(const Vector3<>& position, const Matrix4<>& view, const Matrix4<>& proj)  const;
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

	DECLARE_CLASS_GUID(IDrawableMesh,	0x12d47f0e,0x8813,0x4ad5,0xb13d,0x916b7ab4a618);
	DECLARE_CLASS_GUID(IMeshData,		0xcf361bc0,0xf572,0x4cc9,0xbc2f,0x547537da68a8);

	template<> class get_traits<InputDomain> : public DomainTraits<InputDomain,true> {};
	template<> class get_traits<ResourceDomain> : public DomainTraits<ResourceDomain,false,InputDomain> {};
}

#endif
