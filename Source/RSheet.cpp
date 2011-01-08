#include "header.h"
#include "RSheet.h"

namespace HAGE {
	static const char szDefFormat[] = "SheetVertexFormat";

	const char* SheetVertexFormat::name = szDefFormat;

	VertexDescriptionEntry SheetFormatDescriptor[] = {
		{"Position",	1,	R32G32B32_FLOAT},
		{"Normal",		1,	R32G32B32_FLOAT},
		{"Color",		1,	R32G32B32_FLOAT},
	};

	RenderingSheet* RenderingSheet::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source)
	{
		return new RenderingSheet(ObjectId,h,source);
	}

	RenderingSheet::RenderingSheet(const guid& ObjectId,const MemHandle& h,const guid& source) :
		ObjectBase<RenderingSheet>(ObjectId,h,source),_pVertexBuffer(nullptr)
	{
		
		std::array<u32,nTriangles*3>* pIndexData= new std::array<u32,nTriangles*3>;
		_pVertexData =new std::array<SheetVertexFormat,nVertices>;
		for(int ix= 0;ix<(SheetSize-1);ix++)
			for(int iy= 0;iy<(SheetSize-1);iy++)
			{
				(*pIndexData)[(iy*(SheetSize-1)+ix)*6+0]=iy*SheetSize+ix;
				(*pIndexData)[(iy*(SheetSize-1)+ix)*6+1]=iy*SheetSize+ix+1;
				(*pIndexData)[(iy*(SheetSize-1)+ix)*6+2]=(iy+1)*SheetSize+ix;
				(*pIndexData)[(iy*(SheetSize-1)+ix)*6+3]=iy*SheetSize+ix+1;
				(*pIndexData)[(iy*(SheetSize-1)+ix)*6+4]=(iy+1)*SheetSize+ix+1;
				(*pIndexData)[(iy*(SheetSize-1)+ix)*6+5]=(iy+1)*SheetSize+ix;
			}
		for(int ix= 0;ix<SheetSize;ix++)
			for(int iy= 0;iy<SheetSize;iy++)
			{
				(*_pVertexData)[iy*SheetSize+ix].position = Input1::Get().positions[iy*SheetSize+ix];
				(*_pVertexData)[iy*SheetSize+ix].normal = Input1::Get().normals[iy*SheetSize+ix];
				(*_pVertexData)[iy*SheetSize+ix].color = Vector3<>(1.0f,1.0f,1.0f);
			}

		RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
		assert(pAlloc);
		pAlloc->BeginAllocation();
		static bool registered = false;
		if(!registered)
		{
			pAlloc->RegisterVertexFormat(szDefFormat,SheetFormatDescriptor,sizeof(SheetFormatDescriptor)/sizeof(VertexDescriptionEntry));
			registered =true;
		}
			
		_pVertexBuffer = pAlloc->CreateVertexBuffer(SheetVertexFormat::name,_pVertexData->data(),nVertices,true);
		_pVertexArray = pAlloc->CreateVertexArray(nTriangles,PRIMITIVE_TRIANGLELIST,&_pVertexBuffer,1,(const u32*)pIndexData->data());
		pAlloc->EndAllocation();
		delete pIndexData;
	}

	int RenderingSheet::Draw(EffectContainer* pEffect,const position_constants& c,APIWConstantBuffer* pBuffer)
	{
		for(int ix= 0;ix<SheetSize;ix++)
			for(int iy= 0;iy<SheetSize;iy++)
			{
				(*_pVertexData)[iy*SheetSize+ix].position = Input1::Get().positions[iy*SheetSize+ix];
				(*_pVertexData)[iy*SheetSize+ix].normal = Input1::Get().normals[iy*SheetSize+ix];
				(*_pVertexData)[iy*SheetSize+ix].color = Vector3<>(1.0f,1.0f,1.0f);
			}
		pBuffer->UpdateContent(&c);
		_pVertexBuffer->UpdateContent(_pVertexData->data());
		pEffect->Draw(0,_pVertexArray);

		return 1;
	}

	RenderingSheet::~RenderingSheet()
	{
	}

	DEFINE_CLASS_GUID(RenderingSheet);
}
