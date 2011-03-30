/********************************************************/
/* FILE: CMeshLoader.h                                 */
/* DESCRIPTION: Defines various Resource Fromats for    */
/*              usage with the HAGE Engine              */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/


#ifndef __INTERNAL_MESH_LOADER_H__
#define __INTERNAL_MESH_LOADER_H__

namespace HAGE {

	class CDrawableMeshLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CDrawableMeshLoader(IDataStream* pStream,const IResource* pPrev);
		~CDrawableMeshLoader();
		IResource* Finalize(const IResource** dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
	private:

		class CDrawableMesh : public IDrawableMesh
		{
		public:
			void Draw(const Vector3<>& position, const Matrix4<>& view,const Matrix4<>& inv_view, const Matrix4<>& proj,const Vector3<>& lPosition, const Vector3<>& lColor)  const;
			const APIWVertexArray* GetVertexArray() const{return _pVertexArray;}
			const APIWTexture* GetTexture(u32 index) const{return (index<_nTextures)?_ppTextures[index]:nullptr;}
			virtual ~CDrawableMesh();
			CDrawableMesh(const IMeshData* pData);

			IResource* Finalize(const IResource** dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
		private:

			APIWVertexBuffer*							_pVertexBuffer;
			APIWVertexArray*							_pVertexArray;
			APIWConstantBuffer*							_pConstants;
			APIWEffect*									_pEffect;
			u32											_nTextures;
			const APIWTexture**							_ppTextures;
			std::pair<std::string,guid>*				_pTextureNames;
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
		IResource* Finalize(const IResource** dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);

		bool IsValid(){return _pMeshData?true:false;}
	private:

		class CMeshData : public IMeshData
		{
		public:				
			virtual u32 GetNumVertices() const;
			virtual u32 GetVertexData(const u8** pDataOut,u32& pOutStride,VERTEX_DATA_TYPE type) const;
			virtual u32	GetNumIndices() const;
			virtual u32 GetIndexData(const u8** pDataOut) const;
			virtual const void* GetExtendedData(EXTENDED_DATA_TYPE type) const;
			virtual const char* GetTextureName(u32 index) const;

			virtual ~CMeshData();
			CMeshData(IDataStream* pData);

			bool IsValid(){return _bValid;}

			IResource* Finalize(const IResource** dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
		private:
			
			void GenerateNormals();
			bool TryLoadHGEO(IDataStream* pData);
			bool TryLoadM2(IDataStream* pData);
			void FinalizeLoadM2(const IResource** dependanciesIn,u32 nDependanciesIn);
			bool	_MD2;
			IDataStream* _pData;

			std::vector<std::pair<std::string,guid>> _depencancies;
			std::vector<std::string>	_textureNames;

			u32		_vertexSize;
			u32		_nVertices;
			u32		_nIndices;
			u8*		_pVertexData;
			u8*		_pIndexData;

			bool	_bValid;
		};

		CMeshData* _pMeshData;
	};
		
	class CRawDataLoader : public IResourceLoader
	{
	public:
		static IResourceLoader* Initialize(IDataStream* pStream,const IResource* pPrev);

		CRawDataLoader(IDataStream* pStream,const IResource* pPrev);
		~CRawDataLoader();
		IResource* Finalize(const IResource** dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut);
	private:

		class CRawData : public IRawData
		{
		public:		
			virtual u32 GetSize() const;
			virtual u32 GetData(const u8** pDataOut) const;
			virtual ~CRawData();
			CRawData(IDataStream* pData);
		private:
			u32		_Size;
			void*	_Data;
		};

		CRawData* _pRawData;
	};
}

#endif