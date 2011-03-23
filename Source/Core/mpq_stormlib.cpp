#include "HAGE.h"
#include "mpq_stormlib.h"

#include <StormLib.h>
#include <vector>
#include <string>

using namespace std;

namespace HAGE
{

CMPQArchive::CMPQArchive(const char* filename)
{
	if (!SFileOpenArchive( filename, 0, MPQ_OPEN_READ_ONLY, &mpq_a)) {
		int nError = GetLastError();
		printf("Error opening archive %s, error #: 0x%x\n", filename, nError);
		return;
	}
	printf("Opening archive %s\n", filename);
}

CMPQArchive::~CMPQArchive()
{
	SFileCloseArchive(mpq_a);
}

void CMPQArchive::Close()
{
	delete this;
}

IDataStream* CMPQArchive::OpenDataStream(const char* filename,const char* stream_identifier)
{
	HANDLE fh;
	if(!SFileHasFile(mpq_a,filename))
		return nullptr;
	if(SFileOpenFileEx( mpq_a, filename, 0, &fh ))
		return new CMPQFile(fh,stream_identifier);
	else
		return nullptr;
}

CMPQFile::CMPQFile(void* fh, const char* filename)
	: _filename(filename)
{
	eof = false;
	buffer = 0;
	pointer = 0;
	size = 0;
	
	if(fh)
	{
		DWORD filesize = SFileGetFileSize( fh );
		size = filesize;

		// HACK: in patch.mpq some files don't want to open and give 1 for filesize
		if (size<=1) {
			eof = true;
			buffer = 0;
			return;
		}

		buffer = new unsigned char[size];
		assert(SFileReadFile( fh, buffer, size ));
		assert(SFileCloseFile( fh ));
		fh = nullptr;

		return;
	}

	eof = true;
	buffer = 0;
}

CMPQFile::~CMPQFile()
{
	if (buffer) delete[] buffer;
	buffer = 0;
	eof = true;
}

std::string CMPQFile::GetIdentifierString() const
{
	return _filename;
}

bool CMPQFile::IsValid() const
{
	return buffer?true:false;
}

u64 CMPQFile::Read(u64 nReadMax,u8* pReadOut)
{
	if (eof) 
		return 0;

	size_t rpos = pointer + nReadMax;
	if (rpos > size) {
		nReadMax = size - pointer;
		const_cast<bool&>(eof) = true;
	}

	memcpy(pReadOut, &(buffer[pointer]), nReadMax);

	const_cast<size_t&>(pointer) = rpos;

	return nReadMax;
}

u64 CMPQFile::Seek(i64 iPosition,ORIGIN origin)
{
	switch(origin)
	{
	case IDataStream::ORIGIN_BEGINNING:
		const_cast<size_t&>(pointer) = iPosition;
		break;
	case IDataStream::ORIGIN_CURRENT:
		const_cast<size_t&>(pointer) += iPosition;
		break;
	case IDataStream::ORIGIN_END:
		const_cast<size_t&>(pointer) = size - iPosition;
		break;
	}
	if(pointer >= size)
	{
		const_cast<size_t&>(pointer) = size-1;
		const_cast<bool&>(eof) = true;
	}
	else
		const_cast<bool&>(eof) = false;
	return pointer;
}

}