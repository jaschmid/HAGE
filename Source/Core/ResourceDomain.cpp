#include "HAGE.h"
#include "ResourceDomain.h"
#include "mpq_stormlib.h"

extern HAGE::t64 OSGetTime();

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
			return _fseeki64(_file,iPosition,origin);
		}

		class ResourceTask : public TaskManager::genericTask
		{
		public:
			void operator() ()
			{
			}
		private:
		};

		ResourceDomain::StagedResourceMaster* ResourceDomain::_LoadResource(const char* pName,const guid& type)
		{
			auto found = _centralResourceMap.find(tResourceKey(pName,type));
			if(found == _centralResourceMap.end())
			{
				//load it

				IResource* pResource = nullptr;

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
					ResourceAccess* loadedDependancies = nullptr;
					do
					{
						if(nDependancies)
						{
							if(loadedDependancies)
								delete [] loadedDependancies;
							loadedDependancies = new ResourceAccess[nDependancies];
							for(int i =0;i<nDependancies;++i)
							{
								StagedResourceMaster* resource = _LoadResource(pDependancies[i].first.c_str(),pDependancies[i].second);
								assert(resource);
								loadedDependancies[i] = ResourceAccess(*resource);
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
					
					break;
				}

				if(pResource)
				{
				
					_centralResourceMap.insert(tResourceMap::value_type(tResourceKey(pName,type),std::vector<StagedResourceMaster*>(1)));
					found = _centralResourceMap.find(tResourceKey(pName,type));
					assert(found != _centralResourceMap.end());

					found->second[0] = new StagedResourceMaster;
					found->second[0]->UpdateStage(1,pResource);
					
					pStream->Close();
					return found->second.back();
				}

				// try stream loaders
				
				auto streamLoaders = _streamingLoaderMap.equal_range(type);
				pStream->Seek(0,IDataStream::ORIGIN_BEGINNING);
				IStreamingResourceProvider* provider = nullptr;

				for(auto it= streamLoaders.first;it!=streamLoaders.second;++it)
				{
					provider = it->second(pStream);
					if(provider)
						break;
					pStream->Seek(0,IDataStream::ORIGIN_BEGINNING);
				}
				
				if(provider)
				{
					_centralResourceMap.insert(tResourceMap::value_type(tResourceKey(pName,type),std::vector<StagedResourceMaster*>(1)));
					found = _centralResourceMap.find(tResourceKey(pName,type));

					StagedResourceMasterStreaming* entry = new StagedResourceMasterStreaming(provider,_streamerList,found);

					assert(found != _centralResourceMap.end());

					found->second[0] = entry;

					return found->second.back();
				}

				printf("Unable to load resource: %s\n",pStream->GetIdentifierString().c_str());
				pStream->Close();

				return nullptr;

			}
			else
				return found->second.back();
		}

		u32 ResourceDomain::RunGarbageCollection()
		{
			u32 nReleased = 0;
			auto it = _centralResourceMap.begin();
			while(it != _centralResourceMap.end())
			{
				u32 total_count = 0;
				for(auto it2 = it->second.begin();it2!=it->second.end();++it2)
				{
					u32 ref =  (*it2)->GetRefCount();

					if(ref == RESOURCE_STAGE_STREAMING)
					{
						StagedResourceMasterStreaming* pStreaming = static_cast<StagedResourceMasterStreaming*>(*it2);
						total_count += pStreaming->RunStreamGarbageCollect();
					}
					else
						total_count += ref;
				}
				
				if(total_count == 0)
				{
					for(auto it2 = it->second.begin();it2!=it->second.end();++it2)
					{
						assert((*it2)->GetRefCount() == 0 || (*it2)->GetRefCount() == RESOURCE_STAGE_STREAMING);
						delete (*it2);
					}
					
					auto last = it;
					++it;
					_centralResourceMap.erase(last);
					++nReleased;
				}
				else
					++it;
			}
			return nReleased;
		}


		void ResourceDomain::DomainStep(t64 time)
		{
			const t64 stepBeginTime = OSGetTime();

			for(int i =0;i<_clients.size();++i)
			{
				const Message* curr;
				while(curr=_clients[i].outQueue->GetTopMessage())
				{
					if(OSGetTime().time_utc > stepBeginTime.time_utc + RESOURCE_DOMAIN_MAX_TIME_SLICE)
						break;

					switch(curr->GetMessageCode())
					{
					case MESSAGE_RESOURCE_REQUEST_LOAD:
						{
							const MessageResourceRequestLoad* pDetail = (const MessageResourceRequestLoad*)curr;
							StagedResourceMaster* pMaster = _LoadResource(pDetail->GetName(),pDetail->GetType());

							if(pMaster->GetRefCount() != RESOURCE_STAGE_STREAMING)
							{
								//regular resource
								pMaster->AddRef();
								_clients[i].inQueue->PostMessage(MessageResourceNotifyLoaded(pMaster,pDetail->GetName(),pDetail->GetType()));
							}
							else
							{
								//streaming resource
								StagedResourceMasterStreaming* pStreamingMaster = static_cast<StagedResourceMasterStreaming*>(pMaster);
								pMaster = pStreamingMaster->OpenStream(i);
								if(pMaster)
								{
									StagedResourceMaster* pStreaming = pStreamingMaster->ContinueStream(i);
									assert(pStreaming);
									pMaster->AddRef();
									_clients[i].inQueue->PostMessage(MessageResourceNotifyStreamOpen(pMaster,pStreaming,pDetail->GetName(),pDetail->GetType()));
								}
							}
						}
						break;
					case MESSAGE_RESOURCE_NOTIFY_STREAM_OUT:
						{
							const MessageResourceNotifyStreamOut* pDetail = (const MessageResourceNotifyStreamOut*)curr;
							auto found = _centralResourceMap.find(tResourceKey(pDetail->GetName(),pDetail->GetType()));

							if(found == _centralResourceMap.end())
								break;

							assert(found->second[0]->GetRefCount() == RESOURCE_STAGE_STREAMING);

							StagedResourceMasterStreaming* pStreamingMaster = static_cast<StagedResourceMasterStreaming*>(found->second[0]);

							pStreamingMaster->ProcessFeedback(static_cast<const StagedResourceMaster*>(pDetail->GetStream()),i);
						}
						break;
					case MESSAGE_RESOURCE_REQUEST_STREAM_CLOSE:
						{
							const MessageResourceRequestStreamClose* pDetail = (const MessageResourceRequestStreamClose*)curr;
							auto found = _centralResourceMap.find(tResourceKey(pDetail->GetName(),pDetail->GetType()));

							
							if(found == _centralResourceMap.end())
								break;
							
							assert(found->second[0]->GetRefCount() == RESOURCE_STAGE_STREAMING);

							StagedResourceMasterStreaming* pStreamingMaster = static_cast<StagedResourceMasterStreaming*>(found->second[0]);
							
							pStreamingMaster->CloseStream(i);
						}
						break;
					}
					//printf("Domain Processed %08x\n",curr->GetMessageCode());
					_clients[i].outQueue->PopMessage();
				}
			}

			for(auto it = _streamerList.begin(); it!= _streamerList.end();it++)
			{
				StagedResourceMasterStreaming* streamer = static_cast<StagedResourceMasterStreaming*>((*it)->second[0]);
				for(int i = 0; i < streamer->GetNumClients(); ++i)
				{
					StagedResourceMaster* next = streamer->ContinueStream(i);
					if(next)
						_clients[i].inQueue->PostMessage(MessageResourceNotifyStreamIn(next,(*it)->first.first,(*it)->first.second));
				}
			}

			if((GetTime() - _timeLastGC).toSeconds() > 1.0f)
			{
				_timeLastGC = GetTime();
				RunGarbageCollection();
			}
		}

		ResourceDomain::ResourceDomain() : _registrationLocked(false),_NullStream("Null")
		{
			_timeLastGC = GetTime();
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
			
			while(RunGarbageCollection());

			auto it = _centralResourceMap.begin();
			while(it != _centralResourceMap.end())
			{
				printf("Unfreed Resource %s in Central Repository!\n",it->first.first.c_str());
				for(auto it2 = it->second.begin();it2!=it->second.end();++it2)
				{
					//we do not need to handle stream specially since we can just kick it
					//assert((*it2)->GetRefCount() == 0);
					delete (*it2);
				}
				auto last = it;
				++it;
				_centralResourceMap.erase(last);
			}

			//remove all the stage 0 resources
			for(auto it = _stage0Database.begin();it!=_stage0Database.end();++it)
			{
				delete (StagedResourceMaster*)it->second;	
			}

			for(auto it = _archives.begin();it!=_archives.end();++it)
			{
				it->second->Close();
				it->second = nullptr;
			}

			assert(_streamerList.size() == 0);

		}

		const std::unordered_map<guid,StagedResource*,guid_hasher>& ResourceDomain::RegisterResourceManager(CResourceManager::QueueInType& in_queue,CResourceManager::QueueOutType& out_queue)
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
					ResourceAccess* loadedDependancies = nullptr;
					IResource* pResource = nullptr;
					do
					{
						if(nDependancies)
						{
							loadedDependancies = new ResourceAccess[nDependancies];
							for(int i =0;i<nDependancies;++i)
							{
								auto stage0it = _stage0Database.find(pDependancies[i].second);
								assert(stage0it!= _stage0Database.end());
								loadedDependancies[i]=ResourceAccess(*stage0it->second);
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

					StagedResourceMaster* resource = new StagedResourceMaster();
					resource->UpdateStage(0,pResource);
					_stage0Database.insert(std::pair<guid,StagedResourceMaster*>(resourceId,resource));
				
				}
			}
			
			//switch back
			*p = backup;
		}

		void ResourceDomain::_RegisterResourceStreamingType(const guid& resourceId,const streamingLoaderFunction& streamingLoader)
		{
			assert(!_registrationLocked);

			//switch domains
			TLS_data* p = TLS::getData();

			TLS_data backup = *p;

			p->domain_guid=guid_of<ResourceDomain>::Get();
			p->domain_ptr = this;
			p->random_generator = this;

			_streamingLoaderMap.insert(std::pair<guid,streamingLoaderFunction>(resourceId,streamingLoader));
			auto found = _stage0Database.find(resourceId);
			if(found==_stage0Database.end())
			{
				IStreamingResourceProvider* pProvider = streamingLoader(&_NullStream);
				if(pProvider)
				{
					std::vector<IResource*> resourceVector;

					pProvider->CreateResourceAccessSet(resourceVector);

					assert(resourceVector.size() == 1 && "Streaming Resource Provider did not provide exactly ONE Resource for nullstream");

					StagedResourceMaster* resource = new StagedResourceMaster();
					resource->UpdateStage(0,resourceVector[0]);
					_stage0Database.insert(std::pair<guid,StagedResourceMaster*>(resourceId,resource));
					
					delete pProvider;
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

		ResourceDomain::StagedResourceMasterStreaming::~StagedResourceMasterStreaming()
		{
			for(auto it = clients.begin(); it!= clients.end(); ++it)
			{
				if(*it)
				{
					for(auto it2 = (*it)->feedbackBuffers.begin();it2 != (*it)->feedbackBuffers.end();it2++)
						delete *it2;
					delete *it;
				}
			}
			delete provider;

			_list.erase(_listEntry);
			_listEntry = _list.end();
		}

		u32 ResourceDomain::StagedResourceMasterStreaming::RunStreamGarbageCollect()
		{
			u32 refs = 0;
			for(auto it = clients.begin(); it!= clients.end(); ++it)
			{
				if(*it)
				{
					u32 local_refs = (*it)->streamingMaster.GetRefCount();

					if((*it)->status == CLOSED && local_refs == 0)
					{
						//garbage collect this entry
						for(auto it2 = (*it)->feedbackBuffers.begin();it2 != (*it)->feedbackBuffers.end();it2++)
						{
							assert( (*it2)->GetRefCount() == 0 );
							delete (*it2);
						}

						delete (*it);
						*it = nullptr;
					}
					else
						refs += local_refs;
				}
			}
			return refs;
		}
		/*
			StagedResourceMaster* OpenStream(u32 idxClient);
			StagedResourceMaster* ContinueStream(u32 idxClient);
			void CloseStream(u32 idxClient);
			*/
		ResourceDomain::StagedResourceMaster* ResourceDomain::StagedResourceMasterStreaming::OpenStream(u32 idxClient)
		{
			if(clients.size() < idxClient+1)
			{
				size_t oldSize = clients.size();
				clients.resize(idxClient+1);
				for(size_t i = oldSize; i < clients.size(); ++i)
					clients[i] = nullptr;
			}

			if(!clients[idxClient])
			{
				clients[idxClient] = new ClientEntry;

				std::vector<IResource*> createdFeedbacks;
				provider->CreateResourceAccessSet(createdFeedbacks);

				clients[idxClient]->feedbackBuffers.resize(createdFeedbacks.size());
				for(size_t i = 0; i < createdFeedbacks.size();++i)
				{
					clients[idxClient]->feedbackBuffers[i] = new StagedResourceMaster();
					clients[idxClient]->feedbackBuffers[i]->UpdateStage(1,createdFeedbacks[i]);
				}

				clients[idxClient]->nFirstAvailableBuffer = 0;
				clients[idxClient]->nFirstUsedBuffer = 0;
				clients[idxClient]->status = OPENING;
				clients[idxClient]->nUsedBuffers = 0;

				return &clients[idxClient]->streamingMaster;
			}
			else if(clients[idxClient]->nFirstAvailableBuffer != clients[idxClient]->nFirstUsedBuffer && (clients[idxClient]->status == CLOSING || clients[idxClient]->status == CLOSED))
			{
				assert(clients[idxClient]->status == CLOSING);
				assert(clients[idxClient]->nUsedBuffers != 0);
				StagedResourceMaster* result = &clients[idxClient]->streamingMaster;
				//clients[idxClient]->nFirstAvailableBuffer = (clients[idxClient]->nFirstAvailableBuffer+1)%clients[idxClient]->feedbackBuffers.size();
				clients[idxClient]->status = OPEN;
				return result;
			}
			else if(clients[idxClient]->nFirstAvailableBuffer == clients[idxClient]->nFirstUsedBuffer && (clients[idxClient]->status == CLOSING || clients[idxClient]->status == CLOSED))
			{
				assert(clients[idxClient]->status == CLOSED);
				assert(clients[idxClient]->nUsedBuffers == 0);

				clients[idxClient]->nFirstAvailableBuffer = 0;
				clients[idxClient]->nFirstUsedBuffer = 0;
				clients[idxClient]->status = OPENING;
				clients[idxClient]->nUsedBuffers = 0;

				return &clients[idxClient]->streamingMaster;
			}
			else
				return nullptr;
		}

		ResourceDomain::StagedResourceMaster* ResourceDomain::StagedResourceMasterStreaming::ContinueStream(u32 idxClient)
		{
			if(idxClient >= clients.size() || !clients[idxClient])
				return nullptr;

			ClientEntry& entry = *clients[idxClient];
			
			if(entry.status == OPENING)
			{
				assert(entry.nFirstAvailableBuffer == 0);
				assert(entry.nFirstUsedBuffer == 0);
				assert(entry.nUsedBuffers == 0);

				entry.nFirstAvailableBuffer = 1;
				entry.nUsedBuffers = 1;
				entry.status = OPEN;
				//printf("Domain Sending %08x\n",entry.feedbackBuffers[0]);
				return entry.feedbackBuffers[0];
			}
			else if(entry.status == OPEN)
			{
				if(entry.nFirstAvailableBuffer == entry.nFirstUsedBuffer)
					return nullptr;
				StagedResourceMaster* result = entry.feedbackBuffers[entry.nFirstAvailableBuffer];
				entry.nFirstAvailableBuffer = (entry.nFirstAvailableBuffer+1)%entry.feedbackBuffers.size();
				entry.nUsedBuffers++;
				assert(entry.nUsedBuffers <= entry.feedbackBuffers.size());
				//printf("Domain Sending %08x\n",result);
				return result;
			}
			else
				return nullptr;
		}

		void ResourceDomain::StagedResourceMasterStreaming::CloseStream(u32 idxClient)
		{
			ClientEntry& entry = *clients[idxClient];

			assert(entry.status == OPENING || entry.status == OPEN);

			if(entry.status == OPENING)
			{
				assert(entry.nUsedBuffers == 0);
				entry.status = CLOSED;
			}
			else if(entry.status == OPEN)
			{
				assert(entry.nUsedBuffers != 0);
				entry.status = CLOSING;
			}
		}
		
		void  ResourceDomain::StagedResourceMasterStreaming::ProcessFeedback(const StagedResourceMaster* feedback,u32 idxClient)
		{
			ClientEntry& entry = *clients[idxClient];

			//printf("Domain Recieved %08x\n",feedback);
			
			assert(entry.nUsedBuffers != 0);
			assert(entry.status == OPEN || entry.status == CLOSING);

			size_t feedbackIndex = entry.nFirstUsedBuffer;

			assert( entry.feedbackBuffers[feedbackIndex] == feedback);

			provider->ProcessResourceAccess(entry.feedbackBuffers[feedbackIndex]->FullAccessResource());

			entry.nFirstUsedBuffer = (entry.nFirstUsedBuffer+1)%entry.feedbackBuffers.size();
			entry.nUsedBuffers--;

			if(entry.nFirstAvailableBuffer == entry.nFirstUsedBuffer)
			{
				assert(entry.nUsedBuffers == 0);
				assert(entry.status == CLOSING);
				entry.status = CLOSED;
			}
		}

}
