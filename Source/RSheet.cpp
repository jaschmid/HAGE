#include "header.h"
#include "RSheet.h"

namespace HAGE {
	static const char szDefFormat[] = "SheetVertexFormat";

	const char* SheetVertexFormat::name = szDefFormat;

	VertexDescriptionEntry SheetFormatDescriptor[] = {
		{"Position",	1,	R32G32B32_FLOAT},
		{"Normal",		1,	R32G32B32_FLOAT},
		{"Texcoord",	1,	R32G32_FLOAT},
		{"Color",		1,	R32G32B32_FLOAT},
	};

	RenderingSheet* RenderingSheet::CreateSub(const guid& ObjectId,const MemHandle& h,const guid& source)
	{
		return new RenderingSheet(ObjectId,h,source);
	}

	RenderingSheet::RenderingSheet(const guid& ObjectId,const MemHandle& h,const guid& source) :
		ObjectBase<RenderingSheet>(ObjectId,h,source),_pVertexBuffer(nullptr)
	{
		
		_texture = GetResource()->OpenResource<ITextureImage>("Cloth.png");
		
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
		for(int i = nTriangles/2;i<nTriangles;++i)
		{
			(*pIndexData)[i*3+2] = (*pIndexData)[(i -nTriangles/2)*3+0] + nVertices/2;
			(*pIndexData)[i*3+1] = (*pIndexData)[(i -nTriangles/2)*3+1] + nVertices/2;
			(*pIndexData)[i*3+0] = (*pIndexData)[(i -nTriangles/2)*3+2] + nVertices/2;
		}
		for(int ix= 0;ix<SheetSize;ix++)
			for(int iy= 0;iy<SheetSize;iy++)
			{
				(*_pVertexData)[iy*SheetSize+ix].texcoord = Vector2<>((f32)ix/(f32)SheetSize,(f32)iy/(f32)SheetSize);
				(*_pVertexData)[iy*SheetSize+ix].color = Vector3<>(1.0f,1.0f,1.0f);
				(*_pVertexData)[iy*SheetSize+ix + nVertices/2].texcoord = Vector2<>((f32)ix/(f32)SheetSize,(f32)iy/(f32)SheetSize);
				(*_pVertexData)[iy*SheetSize+ix + nVertices/2].color = Vector3<>(1.0f,1.0f,1.0f);
			}
		for(int i= 0;i<nVertices;i++)
		{
			(*_pVertexData)[i].position = Input1::Get().positions[i];
			(*_pVertexData)[i].normal = Input1::Get().normals[i];
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
		for(int i= 0;i<SheetSize*SheetSize*2;i++)
		{
			(*_pVertexData)[i].position = Input1::Get().positions[i];
			(*_pVertexData)[i].normal = Input1::Get().normals[i];
		}
		pBuffer->UpdateContent(&c);
		_pVertexBuffer->UpdateContent(_pVertexData->data());
		pEffect->SetTexture(3,_texture->GetTexture());
		pEffect->Draw(0,_pVertexArray);

		return 1;
	}

	RenderingSheet::~RenderingSheet()
	{
	}

	DEFINE_CLASS_GUID(RenderingSheet);
}
