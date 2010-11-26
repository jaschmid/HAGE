#include <HAGE.h>

namespace HAGE {
	
	const WrapMessage WrapMessage::_wrap = WrapMessage();

	LockedMessageQueue::LockedMessageQueue() :
		nClosedAccesses(0),bInit(false),nShutdown(0),
		nWriteIndex(0),nReadIndex(-1),
		pMem((mm_vector*)DomainMemory::GlobalAllocate(sizeof(mm_vector)*FRAME_BUFFER_COUNT)),
		m_pvpDestructors((pd_vector*)DomainMemory::GlobalAllocate(sizeof(pd_vector)*FRAME_BUFFER_COUNT))

	{
		for(int i=0;i<FRAME_BUFFER_COUNT;++i)
		{
			new (&pMem[i]) mm_vector;
			new (&m_pvpDestructors[i]) pd_vector;
		}
	}

	LockedMessageQueue::~LockedMessageQueue()
	{
		for(int i=0;i<FRAME_BUFFER_COUNT;++i)
		{
			for(auto j=m_pvpDestructors[i].begin();j!=m_pvpDestructors[i].end();++j)
				((Package*)&pMem[i][*j])->~Package();
			m_pvpDestructors[i].~vector();
			pMem[i].~vector();
		}
		DomainMemory::GlobalFree(pMem);
	}

	result LockedMessageQueue::ClosePin()
	{
		//printf("Pin %08x closed\n",this);
		i32 count = _InterlockedIncrement(&nClosedAccesses);
		if(count == (bInit?fReadReadyCallback.size():0) + 1)
		{
			//printf("Pin %08x ready\n",this);
			bInit=true;
			nClosedAccesses=nShutdown;
			if(nReadIndex>=0)
			{
				for(auto i = m_pvpDestructors[nReadIndex].begin();i!=m_pvpDestructors[nReadIndex].end();++i)
					((Package*)&pMem[nReadIndex][*i])->~Package();
				m_pvpDestructors[nReadIndex].clear();
				pMem[nReadIndex].clear();
			}
			nWriteIndex = (nWriteIndex+1) % FRAME_BUFFER_COUNT;
			nReadIndex = (nReadIndex+1) % FRAME_BUFFER_COUNT;
			fWriteReadyCallback();
			for(u32 i=0;i<fReadReadyCallback.size();++i)
				fReadReadyCallback[i]();
		}
		return S_OK;
	}

	result LockedMessageQueue::InitializeOutputPin(boost::function<void()> f,const guid& guid)
	{
		fWriteReadyCallback = f;
		sourceStamp = guid;
		fWriteReadyCallback();

		return S_OK;
	}

	result LockedMessageQueue::InitializeInputPin(boost::function<void()> f)
	{
		fReadReadyCallback.push_back( f );
		//printf("Pin %08x has %i Domains that use it as input now\n",this,fReadReadyCallback.size());
		return S_OK;
	}


	const Message* LockedMessageQueue::GetNextMessage(const Message* prev) const
	{
		if(prev == (void*)nullptr)
		{
			if( pMem[nReadIndex].size() == 0)
				return nullptr;
			else
				return (const Message*) &pMem[nReadIndex][0];
		}
		else if(prev->GetSize() + (u64)prev >= (u64)&pMem[nReadIndex][0] + pMem[nReadIndex].size())
		{
			return nullptr;
		}
		else
		{
			return (const Message*)((u64)prev + (prev->GetSize()));
		}
	}

	result LockedMessageQueue::_PostMessage(const Message& m)
	{
		if(m.GetMessageCode()&MESSAGE_IS_PACKAGE)
			return _PostPackage((const Package&)m);
		pMem[nWriteIndex].resize( pMem[nWriteIndex].size() + m.GetSize());
		Message* target = (Message*)(&pMem[nWriteIndex][0] + pMem[nWriteIndex].size() - m.GetSize());
		m.CopyTo(target);
		target->SetSource(sourceStamp);
		return S_OK;
	}

	result LockedMessageQueue::_ForwardMessage(const Message& m)
	{
		if(m.GetMessageCode()&MESSAGE_IS_PACKAGE)
			return _ForwardPackage((const Package&)m);
		assert( m.GetSource() != guidNull);
		pMem[nWriteIndex].resize( pMem[nWriteIndex].size() + m.GetSize());
		Message* target = (Message*)(&pMem[nWriteIndex][0] + pMem[nWriteIndex].size() - m.GetSize());
		m.CopyTo(target);
		return S_OK;
	}
	result LockedMessageQueue::_PostPackage(const Package& m)
	{
		assert(m.GetMessageCode()&MESSAGE_IS_PACKAGE);
		pMem[nWriteIndex].resize( pMem[nWriteIndex].size() + m.GetSize());
		u32 index =  pMem[nWriteIndex].size() - m.GetSize();
		Package* target = (Package*)(&pMem[nWriteIndex][index]);
		m_pvpDestructors[nWriteIndex].push_back(index);
		m.CopyTo(target);
		target->SetSource(sourceStamp);
		return S_OK;
	}

	result LockedMessageQueue::_ForwardPackage(const Package& m)
	{
		assert(m.GetMessageCode()&MESSAGE_IS_PACKAGE);
		assert( m.GetSource() != guidNull);
		pMem[nWriteIndex].resize( pMem[nWriteIndex].size() + m.GetSize());
		u32 index =  pMem[nWriteIndex].size() - m.GetSize();
		Package* target = (Package*)(&pMem[nWriteIndex][index]);
		m_pvpDestructors[nWriteIndex].push_back(index);
		m.CopyTo(target);
		return S_OK;
	}
	void LockedMessageQueue::Shutdown()
	{
		_InterlockedIncrement((i32*)&nShutdown);
		ClosePin();
	}


	PinBase::PinBase()
	{
	}

	PinBase::~PinBase()
	{
	}

	MemHandle PinBase::AllocateMemBlock(u32 size)
	{
		MemoryEntry* pEntry = (MemoryEntry*)DomainMemory::GlobalAllocate(sizeof(MemoryEntry)+FRAME_BUFFER_COUNT*size);
		pEntry->size = size;
		pEntry->references = 1;
		return MemHandle(pEntry);
	}

	void PinBase::ReferenceMemBlock(MemHandle handle)
	{
		MemoryEntry* pEntry = (MemoryEntry*)handle._p;
		i32 val =_InterlockedIncrement(&pEntry->references);
		assert(val > 1);
	}

	void PinBase::FreeMemBlock(MemHandle handle)
	{
		MemoryEntry* pEntry = (MemoryEntry*)handle._p;
		i32 val =_InterlockedDecrement(&pEntry->references);
		if(val == 0)
			DomainMemory::GlobalFree(pEntry);
	}
}
