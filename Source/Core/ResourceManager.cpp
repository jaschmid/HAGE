#include <HAGE.h>
#include "ResourceDomain.h"

namespace HAGE {
	CResourceManager::CResourceManager() : 
		_remoteStage0Database(domain_access<ResourceDomain>::Get()->RegisterResourceManager(_queueIn,_queueOut))
	{
	}
	CResourceManager::~CResourceManager()
	{
		ProcessMessages();
		for(auto it = _localResourceMap.begin();it!=_localResourceMap.end();++it)
		{
			StagedResourceLocal& resource = it->second;
			if(resource.HasMaster())
			{
				if(resource.GetRefCount() != 0)
					printf("Unreleased Resource \"%s\" in Domain \"%s\" refcount: %i\n",it->first.first.c_str(),"dunno",resource.GetRefCount());
			}
		}
	}

	void CResourceManager::RunGarbageCollection()
	{
		auto it = _localResourceMap.begin();
		while(it!=_localResourceMap.end())
		{
			StagedResourceLocal& resource = it->second;
			if(resource.HasMaster() && resource.GetRefCount() == 0)
			{
				if(resource.IsStreaming())
				{
					const StagedResource* oldStreaming = resource.UpdateStreaming(nullptr);
					_queueOut.PostMessage(MessageResourceRequestStreamClose(it->first.first,it->first.second));
					//printf("Manager returning %08x, object being deleted\n",oldStreaming);
					_queueOut.PostMessage(MessageResourceNotifyStreamOut(oldStreaming,it->first.first,it->first.second));
				}

				// should auto-decrement
				// _InterlockedDecrement(&resource.pMaster->nRefCount);
				auto last = it;
				++it;
				_localResourceMap.erase(last);
			}
			else
				++it;
		}
	}

	StagedResource* CResourceManager::_OpenResource(const std::string& name, const guid& guid)
	{
		tResourceKey key(name,guid);
		auto found = _localResourceMap.find(key);
		if(found == _localResourceMap.end())
		{
			// create stage 0 of it
			auto stage0it = _remoteStage0Database.find(guid);
			assert(stage0it!= _remoteStage0Database.end());

			StagedResourceLocal resource(stage0it->second);
			_localResourceMap.insert(std::pair<tResourceKey,StagedResourceLocal>(key,resource));
			found = _localResourceMap.find(key);
			assert(found != _localResourceMap.end());
			//also request to get stage 1+ of this resource
			_queueOut.PostMessage(MessageResourceRequestLoad(name,guid));
		}
		return &found->second;
	}
	
	void CResourceManager::ProcessMessages()
	{
		const Message* curr;
		while(curr=_queueIn.GetTopMessage())
		{
			switch(curr->GetMessageCode())
			{
			case MESSAGE_RESOURCE_NOTIFY_LOADED:
				{
					const MessageResourceNotifyLoaded* pDetail = (const MessageResourceNotifyLoaded*)curr;

					tResourceKey key(pDetail->GetName(),pDetail->GetType());
					auto found = _localResourceMap.find(key);
					if(found == _localResourceMap.end())
					{
						StagedResourceLocal resource(pDetail->GetMaster());
						_localResourceMap.insert(std::pair<tResourceKey,StagedResourceLocal>(key,resource));
					}
					else
					{
						found->second.Update(pDetail->GetMaster());
					}

					pDetail->GetMaster()->Release();
				}
				break;
			case MESSAGE_RESOURCE_NOTIFY_STREAM_IN:
				{
					const MessageResourceNotifyStreamIn* pDetail = (const MessageResourceNotifyStreamIn*)curr;
					
					tResourceKey key(pDetail->GetName(),pDetail->GetType());
					auto found = _localResourceMap.find(key);
					if(found == _localResourceMap.end())
					{
						//printf("Manager returning %08x, object not open anymore\n",pDetail->GetStream());
						_queueOut.PostMessage(MessageResourceNotifyStreamOut(pDetail->GetStream(),pDetail->GetName(),pDetail->GetType()));
					}
					else
					{
						const StagedResource* old;
						if(old = found->second.UpdateStreaming( pDetail->GetStream()))
						{
							assert(old!=pDetail->GetStream());
							//printf("Manager returning %08x, recieved %08x\n",old,pDetail->GetStream());
							_queueOut.PostMessage(MessageResourceNotifyStreamOut(old,found->first.first,found->first.second));
						}
					}
				}
				break;
			case MESSAGE_RESOURCE_NOTIFY_STREAM_OPEN:
				{
					const MessageResourceNotifyStreamOpen* pDetail = (const MessageResourceNotifyStreamOpen*)curr;

					tResourceKey key(pDetail->GetName(),pDetail->GetType());
					auto found = _localResourceMap.find(key);
					if(found == _localResourceMap.end())
					{
						//printf("Manager recieved %08x snew stream\n",pDetail->GetStreamingMaster());
						StagedResourceLocal resource(pDetail->GetMaster(),pDetail->GetStreamingMaster());
						_localResourceMap.insert(std::pair<tResourceKey,StagedResourceLocal>(key,resource));
					}
					else
					{
						found->second.Update(pDetail->GetMaster());
						const StagedResource* old;
						if(old = found->second.UpdateStreaming( pDetail->GetStreamingMaster()))
						{
							//printf("Manager returning %08x, recieved %08x stream already exists\n",old,pDetail->GetStreamingMaster());
							_queueOut.PostMessage(MessageResourceNotifyStreamOut(old,found->first.first,found->first.second));
						}
					}

					pDetail->GetMaster()->Release();
				}
				break;
			}
			//printf("Manager Processed %08x\n",curr->GetMessageCode());
			_queueIn.PopMessage();
		}
	}
}