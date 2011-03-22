#ifndef RESOURCE__DOMAIN__INCLUDED
#define RESOURCE__DOMAIN__INCLUDED

#include "HAGE.h"

#include "CMapLoader.h"
#include "CImageLoader.h"
#include "CMeshLoader.h"

namespace HAGE {


class ResourceDomain : public DomainBase<ResourceDomain>
{
	public:
		ResourceDomain();
		~ResourceDomain();
		void DomainStep(t64 time);
		
		class IDataArchive
		{
		public:
			virtual IDataStream* OpenDataStream(const char* filename,const char* stream_identifier) = 0;
			virtual void Close() = 0;
		};

	private:
		
		class CNullStream : public IDataStream
		{
		public:
			CNullStream(std::string identifier):_identifier(identifier){}
			~CNullStream(){}
			std::string GetIdentifierString() const{return _identifier;}
			u64 Read(u64 nReadMax,u8* pReadOut){return 0;}
			u64 Seek(i64,ORIGIN){return 0;}
			void Close(){}
			bool IsValid() const{return false;}
		private:
			std::string _identifier;
		};

		class CFileStream : public IDataStream
		{
		public:
			CFileStream(std::string filename);
			~CFileStream();
			std::string GetIdentifierString() const;
			u64 Read(u64 nReadMax,u8* pReadOut);
			u64 Seek(i64 iPosition,ORIGIN origin);
			void Close(){delete this;}
			bool IsValid() const{return _file?true:false;}
		private:
			std::string _filename;
			FILE*		_file;
		};

		typedef std::function<IResourceLoader* (IDataStream* pStream,const IResource* pPrev)> loaderFunction;

		struct RegisteredResourceManager
		{
			CResourceManager::QueueInType* inQueue;
			CResourceManager::QueueOutType* outQueue;
		};

		SStagedResource* _LoadResource(const char* pName,const guid& type);
		IDataStream* _OpenDataStream(const char* pName);

		const std::unordered_map<guid,IResource*,guid_hasher>& RegisterResourceManager(CResourceManager::QueueInType& in_queue,CResourceManager::QueueOutType& out_queue);

		void _RegisterResourceType(const guid& resourceId,const loaderFunction& stage0Loader);

		std::unordered_map<guid,IResource*,guid_hasher>							_stage0Database;
		CNullStream																_NullStream;
		bool																	_registrationLocked;

		std::unordered_multimap<guid,loaderFunction,guid_hasher>							_loaderMap;
		typedef	std::unordered_map<tResourceKey,std::vector<SStagedResource*>,key_hasher> tResourceMap;
		tResourceMap			_centralResourceMap;

		typedef std::unordered_map<std::string,IDataArchive*>					archiveMap;
		archiveMap																_archives;

		std::vector<RegisteredResourceManager>									_clients;

		friend class CResourceManager;
		friend class RenderingAPIWrapper;
		friend class D3D11APIWrapper;
		friend class OpenGL3APIWrapper;
};

}

#endif
