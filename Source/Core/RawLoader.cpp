#include "HAGE.h"
#include "CMeshLoader.h"

namespace HAGE
{
	
IResourceLoader* CRawDataLoader::Initialize(IDataStream* pStream,const IResource* pPrev)
{
	return new CRawDataLoader(pStream,pPrev);
}

CRawDataLoader::CRawDataLoader(IDataStream* pStream,const IResource* pPrev)
{
	_pRawData = new CRawData(pStream);
}
CRawDataLoader::~CRawDataLoader()
{
}

IResource* CRawDataLoader::Finalize(const IResource** dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
{
	return (IResource*)_pRawData;
}

CRawDataLoader::CRawData::CRawData(IDataStream* pData)
{
	if(!pData->IsValid())
	{
		_Data = 0;
		_Size = 0;
		return;
	}
	_Size = pData->Seek(0,IDataStream::ORIGIN_END)+1;
	_Data = new u8[_Size];
	pData->Seek(0,IDataStream::ORIGIN_BEGINNING);
	assert(pData->Read(_Size,(u8*)_Data)==_Size);
}


CRawDataLoader::CRawData::~CRawData()
{
	delete [] _Data;
}

u32 CRawDataLoader::CRawData::GetSize() const
{
	return _Size;
}

u32 CRawDataLoader::CRawData::GetData(const u8** pDataOut) const
{
	if(pDataOut)
		*pDataOut = (u8*)_Data;
	return _Size;
}

}
