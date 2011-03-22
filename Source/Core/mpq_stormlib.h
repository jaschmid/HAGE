#ifndef MPQ_H
#define MPQ_H

#undef _DLL

#include "HAGE.h"
#include "ResourceDomain.h"

// C++ files
#include <string>
#include <set>
#include <vector>
/*
struct FileTreeItem {
	std::string fn;
	int col;

	/// Comparison
	bool operator<(const FileTreeItem &i) const {
		return fn < i.fn;
	}

	bool operator>(const FileTreeItem &i) const {
		return fn < i.fn;
	}
};*/

namespace HAGE {

class CMPQArchive : public ResourceDomain::IDataArchive
{
	//MPQHANDLE handle;
	void* mpq_a;
public:
	CMPQArchive(const char* filename);
	~CMPQArchive();
	IDataStream* OpenDataStream(const char* filename,const char* stream_identifier);
	void Close();
};


class CMPQFile : public IDataStream
{
	//MPQHANDLE handle;
	bool eof;
	unsigned char *buffer;
	size_t pointer, size;
	std::string _filename;

	// disable copying
	CMPQFile(const CMPQFile &f) {}
	void operator=(const CMPQFile &f) {}

	friend class CMPQArchive;

	CMPQFile():eof(false),size(0),buffer(0),pointer(0) {}
	CMPQFile(void* pArchiveHandle, const char* filename);	// filenames are not case sensitive
	~CMPQFile();
public:

	std::string GetIdentifierString() const;
	bool IsValid() const;
	u64 Read(u64 nReadMax,u8* pReadOut);
	u64 Seek(i64 iPosition,ORIGIN origin);
	void Close(){delete this;}
};
/*
inline void flipcc(char *fcc)
{
	char t;
	t=fcc[0];
	fcc[0]=fcc[3];
	fcc[3]=t;
	t=fcc[1];
	fcc[1]=fcc[2];
	fcc[2]=t;
}

inline bool defaultFilterFunc(std::string) { return true; }
*/
}

#endif

