#include "HAGE.h"
#include "ResourceDomain.h"
#include "mpq_stormlib.h"

namespace HAGE {

		ResourceDomain::CFileStream::CFileStream(std::string filename) : _filename(filename),_file(fopen(filename.c_str(),"rb"))
		{
		}

		ResourceDomain::CFileStream::~CFileStream()
		{
			if(_file)
				fclose(_file);
		}

		std::string ResourceDomain::CFileStream::GetIdentifierString() const
		{
			return _filename;
		}

		u64 ResourceDomain::CFileStream::Read(u64 nReadMax,u8* pReadOut)
		{
			return fread(pReadOut,1,nReadMax,_file);
		}
		u64 ResourceDomain::CFileStream::Seek(i64 iPosition,ORIGIN origin)
		{
			int seek_origin;
			switch(origin)
			{
			case ORIGIN_BEGINNING:
				seek_origin = SEEK_SET;
				break;
			case ORIGIN_CURRENT:
				seek_origin = SEEK_CUR;
				break;
			case ORIGIN_END:
				seek_origin = SEEK_END;
				break;
			}
			return fseek(_file,iPosition,origin);
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
				IDataStream* pStream = _OpenDataStream(pName);
				for(auto it= loaders.first;it!=loaders.second;++it)
				{
					IResourceLoader* pLoader = it->second(pStream,nullptr);
					if(!pLoader)
					{
						pStream->Seek(0,IDataStream::ORIGIN_BEGINNING);
						continue;
					}
					const std::pair<std::string,guid>* pDependancies = nullptr;
					u32 nDependancies = 0;
					const IResource** loadedDependancies = nullptr;
					IResource* pResource = nullptr;
					do
					{
						if(nDependancies)
						{
							loadedDependancies = new const IResource*[nDependancies];
							for(int i =0;i<nDependancies;++i)
							{
								SStagedResource* resource = _LoadResource(pDependancies[i].first.c_str(),pDependancies[i].second);
								assert(resource);
								loadedDependancies[i] = (IResource*)resource->pCurrentStage;
							}
						}
						else
						{
							assert(pDependancies== nullptr);
							if(loadedDependancies)
								delete [] loadedDependancies;
							loadedDependancies = nullptr;
						}

						pResource = pLoader->Finalize(loadedDependancies,&pDependancies,nDependancies);
					}
					while(!pResource);

					if(loadedDependancies)
						delete [] loadedDependancies;
					loadedDependancies = nullptr;
					delete pLoader;

					found->second[0]->pCurrentStage=pResource;

					pStream->Close();
					break;
				}

				assert(found->second[0]->pCurrentStage);

			}
			return found->second.back();
		}

		void ResourceDomain::DomainStep(t64 time)
		{
			for(int i =0;i<_clients.size();++i)
			{
				const Message* curr;
				while(curr=_clients[i].outQueue->GetTopMessage())
				{
					switch(curr->GetMessageCode())
					{
					case MESSAGE_RESOURCE_REQUEST_LOAD:
						{
							const MessageResourceRequestLoad* pDetail = (const MessageResourceRequestLoad*)curr;
							SStagedResource* pMaster = _LoadResource(pDetail->GetName(),pDetail->GetType());
							pMaster->nRefCount++;
							_clients[i].inQueue->PostMessage(MessageResourceNotifyLoaded(pMaster,pDetail->GetName(),pDetail->GetType()));
						}
						break;
					}
					_clients[i].outQueue->PopMessage();
				}
			}
		}

		ResourceDomain::ResourceDomain() : _registrationLocked(false),_NullStream("Null")
		{
			HAGE::domain_access<ResourceDomain>::Get()->_RegisterResourceType(guid_of<IMeshData>::Get(),&CMapDataLoader::Initialize);
			HAGE::domain_access<ResourceDomain>::Get()->_RegisterResourceType(guid_of<IMeshData>::Get(),&CMeshDataLoader::Initialize);
			HAGE::domain_access<ResourceDomain>::Get()->_RegisterResourceType(guid_of<IImageData>::Get(),&CMapDataImageLoader::Initialize);
			HAGE::domain_access<ResourceDomain>::Get()->_RegisterResourceType(guid_of<IImageData>::Get(),&CImageDataLoader::Initialize);
			HAGE::domain_access<ResourceDomain>::Get()->_RegisterResourceType(guid_of<IRawData>::Get(),&CRawDataLoader::Initialize);
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

			for(auto it = _archives.begin();it!=_archives.end();++it)
			{
				it->second->Close();
				it->second = nullptr;
			}
		}

		const std::unordered_map<guid,IResource*,guid_hasher>& ResourceDomain::RegisterResourceManager(CResourceManager::QueueInType& in_queue,CResourceManager::QueueOutType& out_queue)
		{
			assert(!_registrationLocked);
			//switch domains
			TLS_data* p = TLS::getData();

			TLS_data backup = *p;

			p->domain_guid=guid_of<ResourceDomain>::Get();
			p->domain_ptr = this;
			p->random_generator = this;

			RegisteredResourceManager m = {&in_queue,&out_queue};
			_clients.push_back(m);

			//switch back
			*p = backup;

			return _stage0Database;
		}

		void ResourceDomain::_RegisterResourceType(const guid& resourceId,const std::function<IResourceLoader* (IDataStream* pStream,const IResource* pPrev)>& loader)
		{
			assert(!_registrationLocked);

			//switch domains
			TLS_data* p = TLS::getData();

			TLS_data backup = *p;

			p->domain_guid=guid_of<ResourceDomain>::Get();
			p->domain_ptr = this;
			p->random_generator = this;

			_loaderMap.insert(std::pair<guid,loaderFunction>(resourceId,loader));
			auto found = _stage0Database.find(resourceId);
			if(found==_stage0Database.end())
			{
				IResourceLoader* pLoader = loader(&_NullStream,nullptr);
				if(pLoader)
				{
					const std::pair<std::string,guid>* pDependancies = nullptr;
					u32 nDependancies = 0;
					const IResource** loadedDependancies = nullptr;
					IResource* pResource = nullptr;
					do
					{
						if(nDependancies)
						{
							loadedDependancies = new const IResource*[nDependancies];
							for(int i =0;i<nDependancies;++i)
							{
								auto stage0it = _stage0Database.find(pDependancies[i].second);
								assert(stage0it!= _stage0Database.end());
								loadedDependancies[i]=stage0it->second;
							}
						}
						else
						{
							assert(pDependancies== nullptr);
							if(loadedDependancies)
								delete [] loadedDependancies;
							loadedDependancies = nullptr;
						}

						pResource = pLoader->Finalize(loadedDependancies,&pDependancies,nDependancies);
					}
					while(!pResource);

					if(loadedDependancies)
						delete [] loadedDependancies;
					loadedDependancies = nullptr;
					delete pLoader;

					_stage0Database.insert(std::pair<guid,IResource*>(resourceId,pResource));
				
				}
			}
			
			//switch back
			*p = backup;
		}

		
		IDataStream* ResourceDomain::_OpenDataStream(const char* pName)
		{
			if(!pName)
				return &_NullStream;

			IDataStream* pRes=nullptr;

			if(pName[0] == '\@')
			{
				// archive file
				char ArchiveName[256];
				const char* FirstSlash = strchr(pName,'/');
				const char* FirstBackslash = strchr(pName,'\\');
				int ArchiveNameLength;
				assert(FirstSlash || FirstBackslash);

				if(FirstSlash && FirstBackslash)
					ArchiveNameLength = std::min(FirstSlash,FirstBackslash)-pName-1;
				else if(FirstBackslash)
					ArchiveNameLength = FirstBackslash-pName-1;
				else
					ArchiveNameLength = FirstSlash-pName-1;

				memcpy(ArchiveName,&pName[1],ArchiveNameLength);
				ArchiveName[ArchiveNameLength] = '\0';
				const char* FileName = &pName[ArchiveNameLength+2];

				auto archive = _archives.find(ArchiveName);
				if(archive == _archives.end())
				{
					std::pair<std::string,IDataArchive*> entry(ArchiveName,new CMPQArchive(ArchiveName));
					_archives.insert(entry);
					archive = _archives.find(ArchiveName);
					assert(archive!=_archives.end());
				}

				pRes = archive->second->OpenDataStream(FileName,pName);
			}
			else
				pRes= new CFileStream(pName);
	
			if(pRes == nullptr)
				return new CNullStream(pName);
			else if(!pRes->IsValid())
			{
				pRes->Close();
				return new CNullStream(pName);
			}

			return pRes;
		}

}
