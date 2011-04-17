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
		
		u32 RunGarbageCollection();
		
		class StagedResourceMaster : public StagedResource
		{
		public:
			
			virtual ~StagedResourceMaster(){ if(pModStage) delete pModStage; }
			StagedResourceMaster() : StagedResource(0,nullptr,0),pModStage(nullptr) {}
			u32 GetRefCount() const{return nRefCount;}
			IResource* FullAccessResource() {return pModStage;}
			void UpdateStage(u32 newStage,IResource* newResource){nCurrentStage=newStage;pCurrentStage=newResource;pModStage=newResource;}

		protected:
	
			IResource* pModStage;
		};
		
		typedef std::function<IResourceLoader* (IDataStream* pStream,const IResource* pPrev)> loaderFunction;
		typedef std::function<IStreamingResourceProvider* (IDataStream* pStream)> streamingLoaderFunction;
		typedef	std::unordered_map<tResourceKey,std::vector<StagedResourceMaster*>,key_hasher> tResourceMap;
		typedef tResourceMap::iterator											tResourceMapReference;
		typedef std::list<tResourceMapReference>								tStreamerList;
		typedef std::unordered_map<std::string,IDataArchive*>					archiveMap;

		class StagedResourceMasterStreaming : public  StagedResourceMaster
		{
		public:

			StagedResourceMasterStreaming(IStreamingResourceProvider* p,tStreamerList& list,tResourceMapReference ref) : provider(p) , _list(list)
			{
				nCurrentStage = 1;
				pCurrentStage = nullptr;
				nRefCount = 0xffffffff;
				pModStage=nullptr;
				_list.push_front(ref);
				_listEntry = _list.begin();
			}

			void ProcessFeedback(const StagedResourceMaster* feedback,u32 idxClient);
			void ProcessQueuedFeedback();
			StagedResourceMaster* OpenStream(u32 idxClient);
			StagedResourceMaster* ContinueStream(u32 idxClient);
			void CloseStream(u32 idxClient);
			u32 GetNumClients(){return clients.size();}
			virtual ~StagedResourceMasterStreaming();

			u32 RunStreamGarbageCollect();

		private:

			typedef enum _StreamStatus
			{
				OPENING,
				OPEN,
				CLOSING,
				CLOSED
			} StreamStatus;

			struct ClientEntry
			{
				std::vector<StagedResourceMaster*>	feedbackBuffers;

				u32									nBuffersWaitingForProcessing;

				u32									nFirstUsedBuffer;
				u32									nFirstAvailableBuffer;
				u32									nUsedBuffers;
				StreamStatus						status;
				StagedResourceMaster				streamingMaster;
			};

			IStreamingResourceProvider* provider;
			std::vector< ClientEntry*>  clients;
			tStreamerList&				_list;
			tStreamerList::iterator		_listEntry;
		};

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


		struct RegisteredResourceManager
		{
			CResourceManager::QueueInType* inQueue;
			CResourceManager::QueueOutType* outQueue;
		};

		StagedResourceMaster* _LoadResource(const char* pName,const guid& type);
		IDataStream* _OpenDataStream(const char* pName);

		const std::unordered_map<guid,StagedResource*,guid_hasher>& RegisterResourceManager(CResourceManager::QueueInType& in_queue,CResourceManager::QueueOutType& out_queue);

		void _RegisterResourceType(const guid& resourceId,const loaderFunction& stage0Loader);
		void _RegisterResourceStreamingType(const guid& resourceId,const streamingLoaderFunction& streamingLoader);

		//actually they are all StagedResourceMaster
		std::unordered_map<guid,StagedResource*,guid_hasher>				_stage0Database;
		CNullStream																_NullStream;
		bool																	_registrationLocked;

		std::unordered_multimap<guid,loaderFunction,guid_hasher>				_loaderMap;
		std::unordered_multimap<guid,streamingLoaderFunction,guid_hasher>		_streamingLoaderMap;



		tResourceMap															_centralResourceMap;
		tStreamerList															_streamerList;

		archiveMap																_archives;

		std::vector<RegisteredResourceManager>									_clients;

		t64																		_timeLastGC;

		friend class CResourceManager;
		friend class RenderingAPIWrapper;
		friend class D3D11APIWrapper;
		friend class OpenGL3APIWrapper;
};

}

#endif
