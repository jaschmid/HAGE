#include <HAGE.h>
#include "ResourceDomain.h"

namespace HAGE {
	CResourceManager::CResourceManager() : 
		_remoteStage0Database(domain_access<ResourceDomain>::Get()->RegisterResourceManager(_queueIn,_queueOut))
	{
	}
	CResourceManager::~CResourceManager()
	{
	}

	SStagedResource& CResourceManager::_OpenResource(const std::string& name, const guid& guid)
	{
		tResourceKey key(name,guid);
		auto found = _localResourceMap.find(key);
		if(found == _localResourceMap.end())
		{
			// create stage 0 of it
			SStagedResource resource;
			resource.nCurrentStage = 0;
			resource.nRefCount = 0;
			auto stage0it = _remoteStage0Database.find(guid);
			assert(stage0it!= _remoteStage0Database.end());
			resource.pCurrentStage = stage0it->second;
			_localResourceMap.insert(std::pair<tResourceKey,SStagedResource>(key,resource));
			found = _localResourceMap.find(key);
			assert(found != _localResourceMap.end());
			//also request to get stage 1+ of this resource
			_queueOut.PostMessage(MessageResourceRequestLoad(name,guid));
		}
		return found->second;
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
						SStagedResource resource;
						resource.nCurrentStage = pDetail->GetMaster()->nCurrentStage;
						resource.pCurrentStage = pDetail->GetMaster()->pCurrentStage;
						resource.nRefCount = 0;
						_localResourceMap.insert(std::pair<tResourceKey,SStagedResource>(key,resource));
					}
					else
					{
						found->second.nCurrentStage = pDetail->GetMaster()->nCurrentStage;
						found->second.pCurrentStage = pDetail->GetMaster()->pCurrentStage;
					}
				}
				break;
			}
			_queueIn.PopMessage();
		}
	}
}