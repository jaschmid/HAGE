#include "HAGE.h"
#include "ResourceDomain.h"

namespace HAGE {

		ResourceDomain::CFileStream::CFileStream(std::string filename) : _filename(filename),_file(fopen(filename.c_str(),"rb"))
		{
			assert(_file);
		}

		ResourceDomain::CFileStream::~CFileStream()
		{
			fclose(_file);
		}

		std::string ResourceDomain::CFileStream::GetIdentifierString() const
		{
			return _filename;
		}

		u64 ResourceDomain::CFileStream::Read(u64 nReadMax,u8* pReadOut) const
		{
			return fread(pReadOut,1,nReadMax,_file);
		}

		class ResourceTask : public TaskManager::genericTask
		{
		public:
			void operator() ()
			{
			}
		private:
		};

		SStagedResource* ResourceDomain::_LoadResource(const char* pName,const guid& type)
		{
			auto found = _centralResourceMap.find(tResourceKey(pName,type));
			if(found == _centralResourceMap.end())
			{
				//load it
				_centralResourceMap.insert(tResourceMap::value_type(tResourceKey(pName,type),std::vector<SStagedResource*>(1)));
				found = _centralResourceMap.find(tResourceKey(pName,type));
				assert(found != _centralResourceMap.end());
				found->second[0] = new SStagedResource;
				found->second[0]->nRefCount = 1;
				found->second[0]->nCurrentStage = 1;
				found->second[0]->pCurrentStage = nullptr;

				auto loaders = _loaderMap.equal_range(type);
				for(auto it= loaders.first;it!=loaders.second;++it)
				{
					IResourceLoader* pLoader = it->second(new CFileStream(pName),nullptr);
					if(pLoader == nullptr)
						continue;
					const std::pair<std::string,guid>* pDependancies;
					u32 nDependancies = pLoader->GetDependancies(&pDependancies);

					const IResource** loadedDependancies = new const IResource*[nDependancies];
					for(int i =0;i<nDependancies;++i)
					{
						SStagedResource* resource = _LoadResource(pDependancies[i].first.c_str(),pDependancies[i].second);
						assert(resource!= nullptr);
						loadedDependancies[i] = (IResource*)resource->pCurrentStage;
					}
			
					found->second[0]->pCurrentStage=(IResource*)pLoader->Finalize(loadedDependancies,nDependancies);
				
					delete [] loadedDependancies;
					delete pLoader;
					break;
				}

				assert(found->second[0]->pCurrentStage!=nullptr);

			}
			return found->second.back();
		}
		
		void ResourceDomain::DomainStep(u64 step)
		{
			for(int i =0;i<_clients.size();++i)
			{
				const Message* curr;
				while(curr=_clients[i].outQueue.GetTopMessage())
				{
					switch(curr->GetMessageCode())
					{
					case MESSAGE_RESOURCE_REQUEST_LOAD:
						{
							const MessageResourceRequestLoad* pDetail = (const MessageResourceRequestLoad*)curr;

							_clients[i].inQueue.PostMessage(MessageResourceNotifyLoaded(_LoadResource(pDetail->GetName(),pDetail->GetType()),pDetail->GetName(),pDetail->GetType()));
						}
						break;
					}
					_clients[i].outQueue.PopMessage();
				}
			}
		}

		ResourceDomain::ResourceDomain() : _registrationLocked(false)
		{	
			HAGE::domain_access<ResourceDomain>::Get()->_RegisterResourceType(guid_of<IMeshData>::Get(),&CMeshDataLoader::Initialize);
			printf("Init Resource\n");
		}

		ResourceDomain::~ResourceDomain()
		{
			printf("Destroy Resource\n");
			//remove all the stage 0 resources
			for(auto it = _stage0Database.begin();it!=_stage0Database.end();++it)
			{
				delete it->second;
				it->second = nullptr;
			}
		}

		const std::unordered_map<guid,IResource*,guid_hasher>& ResourceDomain::RegisterResourceManager(CResourceManager::QueueInType& in_queue,CResourceManager::QueueOutType& out_queue)
		{
			assert(!_registrationLocked);
			//switch domains
			const guid* pDomainGuid = TLS::domain_guid.release();
			IDomain* pDomain = TLS::domain_ptr.release();
			TLS::domain_guid.reset(const_cast<guid*>(&guid_of<ResourceDomain>::Get()));
			TLS::domain_ptr.reset(this);

			RegisteredResourceManager m = {in_queue,out_queue};
			_clients.push_back(m);

			//switch back
			TLS::domain_guid.release();
			TLS::domain_ptr.release();
			TLS::domain_guid.reset(const_cast<guid*>(pDomainGuid));
			TLS::domain_ptr.reset(pDomain);

			return _stage0Database;
		}

		void ResourceDomain::_RegisterResourceType(const guid& resourceId,const std::function<IResourceLoader* (IDataStream* pStream,const IResource* pPrev)>& loader)
		{
			assert(!_registrationLocked);

			//switch domains
			const guid* pDomainGuid = TLS::domain_guid.release();
			IDomain* pDomain = TLS::domain_ptr.release();
			TLS::domain_guid.reset(const_cast<guid*>(&guid_of<ResourceDomain>::Get()));
			TLS::domain_ptr.reset(this);

			_loaderMap.insert(std::pair<guid,loaderFunction>(resourceId,loader));
			auto found = _stage0Database.find(resourceId);
			if(found==_stage0Database.end())
			{
				IResourceLoader* pLoader = loader(&_NullStream,nullptr);
				const std::pair<std::string,guid>* pDependancies;
				u32 nDependancies = pLoader->GetDependancies(&pDependancies);

				const IResource** loadedDependancies = new const IResource*[nDependancies];
				for(int i =0;i<nDependancies;++i)
				{
					auto stage0it = _stage0Database.find(pDependancies[i].second);
					assert(stage0it!= _stage0Database.end());
					loadedDependancies[i]=stage0it->second;
				}
			
				_stage0Database.insert(std::pair<guid,IResource*>(resourceId,(IResource*)pLoader->Finalize(loadedDependancies,nDependancies)));
				
				delete [] loadedDependancies;
				delete pLoader;
			}

			//switch back
			TLS::domain_guid.release();
			TLS::domain_ptr.release();
			TLS::domain_guid.reset(const_cast<guid*>(pDomainGuid));
			TLS::domain_ptr.reset(pDomain);
		}

}