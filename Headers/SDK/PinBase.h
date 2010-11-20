#ifndef HAGE__MAIN__HEADER
#error Do not include this file directly, include HAGE.h instead
#endif

#ifndef PINBASE_H_INCLUDED
#define PINBASE_H_INCLUDED

#include "HAGE.h"
#include "global_allocator.h"
#include <boost/function.hpp>
#include <vector>
#include <array>

namespace HAGE {


class WrapMessage : public MessageHelper<WrapMessage>
{
public:
	WrapMessage() : MessageHelper<WrapMessage>(MESSAGE_RESERVED_WRAP_QUEUE) {}

	static const WrapMessage _wrap;
};

template<u32 size> class StaticMessageQueue
{
public:
	StaticMessageQueue() : nReadIndex(0),nWriteIndex(0)
	{
	}
	virtual ~StaticMessageQueue() {}

	// called by producer
	result			PostMessage(const Message& m)
	{
		u32 nSize = m.GetSize();

		if( nWriteIndex < size - nSize)
		{
			assert(nReadIndex > nWriteIndex + nSize || nReadIndex <= nWriteIndex);
			// no wrap
			m.CopyTo(&Mem[nWriteIndex]);
			nWriteIndex += nSize;
			return S_OK;
		}
		else if( nWriteIndex >= size - sizeof(Message))
		{
			assert(nReadIndex <= nWriteIndex);
			// auto wrap
			nWriteIndex = 0;
			return PostMessage(m);
		}
		else
		{
			// manual wrap
			assert(nReadIndex <= nWriteIndex);
			assert(succeeded(PostMessage(WrapMessage::_wrap)));
			nWriteIndex = 0;
			return PostMessage(m);
		}
	}

	// called by consumer
	void			PopMessage()
	{
		if(nReadIndex == nWriteIndex)
			return;

		nReadIndex += GetTopMessage()->GetSize();

		if(nReadIndex + sizeof(Message) >= size)
			nReadIndex = 0;
		else if(nReadIndex != nWriteIndex && GetTopMessage()->GetMessageCode() == MESSAGE_RESERVED_WRAP_QUEUE)
			nReadIndex = 0;

		return;
	}
	const Message*	GetTopMessage() const
	{
		if(nReadIndex == nWriteIndex)
			return nullptr;

		return (const Message*)(&Mem[nReadIndex]);
	}

private:

	u32		nReadIndex;
	u32		nWriteIndex;

	typedef std::array<u8,size> mm_vector;

	mm_vector		Mem;
};

class LockedMessageQueue
{
public:
	LockedMessageQueue();
	virtual ~LockedMessageQueue();

	result			PostMessage(const Message& m);
	result			ForwardMessage(const Message& m);
	const Message*	GetNextMessage(const Message* prev) const;

	result ClosePin();
	result InitializeInputPin(boost::function<void()> f);
	result InitializeOutputPin(boost::function<void()> f,const guid& sourceId);

	void	Shutdown();

protected:

	i32		nReadIndex;
	i32		nWriteIndex;

private:

	typedef std::vector<boost::function<void()>,global_allocator<boost::function<void()>>> callback_list;
	typedef std::vector<u8,global_allocator<u8>> mm_vector;

	// proper private
	volatile i32	nClosedAccesses;
	volatile bool	bInit;
	volatile i32	nShutdown;
	callback_list fReadReadyCallback;
	boost::function<void()> fWriteReadyCallback;
	guid			sourceStamp;
	mm_vector*		pMem;
	void*			pVoid;

	template<class _T> friend class OutputPin;
	template<class _T,class _T2> friend class InputPin;

};

class PinBase : public LockedMessageQueue
{
public:
	PinBase();
	virtual ~PinBase();

	void* GetWriteMem(MemHandle itemHandle,u32 size) const
	{
		MemoryEntry* entry = (MemoryEntry*)itemHandle._p;

		assert(entry->size == (i32)size);

		return ((u8*)entry)+sizeof(MemoryEntry)+size*nWriteIndex;
	}
	const void* GetReadMem(MemHandle itemHandle,u32 size) const
	{
		MemoryEntry* entry = (MemoryEntry*)itemHandle._p;

		assert(entry->size == (i32)size);

		return ((u8*)entry)+sizeof(MemoryEntry)+size*nReadIndex;
	}
	MemHandle AllocateMemBlock(u32 size);
	void ReferenceMemBlock(MemHandle Handle);
	void FreeMemBlock(MemHandle Handle);

private:
	class MemoryEntry
	{
	public:
		i32 size;
		i32 references;
	};
};

}

#endif
