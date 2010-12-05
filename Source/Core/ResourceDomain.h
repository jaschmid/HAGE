#ifndef RESOURCE__DOMAIN__INCLUDED
#define RESOURCE__DOMAIN__INCLUDED

#include "HAGE.h"

namespace HAGE {


class ResourceDomain : public DomainBase<ResourceDomain>
{
	public:
		ResourceDomain();
		~ResourceDomain();
		void DomainStep(u64 step);

	private:

		class CNullStream : public IDataStream
		{
		public:
			CNullStream(){}
			~CNullStream(){}
			std::string GetIdentifierString() const{return std::string("Null");}
			u64 Read(u64 nReadMax,u8* pReadOut) const{return 0;}
			void Close(){}
		};

		class CFileStream : public IDataStream
		{
		public:
			CFileStream(std::string filename);
			~CFileStream();
			std::string GetIdentifierString() const;
			u64 Read(u64 nReadMax,u8* pReadOut) const;
			void Close(){delete this;}
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

		const std::unordered_map<guid,IResource*,guid_hasher>& RegisterResourceManager(CResourceManager::QueueInType& in_queue,CResourceManager::QueueOutType& out_queue);

		void _RegisterResourceType(const guid& resourceId,const loaderFunction& stage0Loader);

		std::unordered_map<guid,IResource*,guid_hasher>							_stage0Database;
		CNullStream																_NullStream;
		bool																	_registrationLocked;

		std::unordered_multimap<guid,loaderFunction,guid_hasher>							_loaderMap;
		typedef	std::unordered_map<tResourceKey,std::vector<SStagedResource*>,key_hasher> tResourceMap;
		tResourceMap			_centralResourceMap;

		std::vector<RegisteredResourceManager>									_clients;

		friend class CResourceManager;
		friend class RenderingAPIWrapper;
};

}

#endif
