#include "header.h"
#include "RSheet.h"

namespace HAGE {

	static const char* sheet_vertex_program =
	"// This is C2E1v_green from \"The Cg Tutorial\" (Addison-Wesley, ISBN\n"
	"// 0321194969) by Randima Fernando and Mark J. Kilgard.  See page 38.\n"
	"struct TransformGlobal\n"
	"{\n"
	"    float4x4 modelview;\n"
	"    float4x4 inverse_modelview;\n"
	"    float4x4 modelview_projection;\n"
	"} cbuffer0_TransformGlobal: BUFFER[0];\n"
	"\n"
	"// cbuffer Transform : register(b0)\n"
	"#define Modelview               cbuffer0_TransformGlobal.modelview\n"
	"#define InverseModelview        cbuffer0_TransformGlobal.inverse_modelview\n"
	"#define ModelviewProjection     cbuffer0_TransformGlobal.modelview_projection\n"
	"\n"
	"struct C2E1v_Output {\n"
	"  float4 position : POSITION;\n"
	"  float4 color    : COLOR;\n"
	"};\n"
	"\n"
	"C2E1v_Output vertex(float3 position : POSITION,float3 normal : NORMAL,float3 color : COLOR)\n"
	"{	\n"
	"  C2E1v_Output OUT;\n"
	"\n"
	"  OUT.position = mul( ModelviewProjection, float4( position, 1.0f ) );\n "
	"  float3 light_intensity = -dot(float3(0.0f,-0.70f,0.70f),normal);\n"
	"  float3 this_color = light_intensity*color;\n"
	"  OUT.color = float4(this_color,1.0f);\n"
	"\n"
	"  return OUT;	\n"
	"}";

	static const char* sheet_fragment_program =
	"// This is C2E2f_passthru from \"The Cg Tutorial\" (Addison-Wesley, ISBN\n"
	"// 0321194969) by Randima Fernando and Mark J. Kilgard.  See page 53.\n"
	"\n"
	"struct C2E2f_Output {\n"
	"  float4 color : COLOR;\n"
	"};\n"
	"\n"
	"C2E2f_Output fragment(float4 color : COLOR)\n"
	"{\n"
	"  C2E2f_Output OUT;\n"
	"  OUT.color = color;\n"
	"  return OUT;\n"
	"}\n";


	struct sheet_Constants
	{
		Matrix4<>	modelview;
		Matrix4<>	inverse_modelview;
		Matrix4<>	modelview_projection;
	};

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
		_pEffect = pAlloc->CreateEffect(sheet_vertex_program,sheet_fragment_program);
		_pConstants = pAlloc->CreateConstantBuffer(sizeof(sheet_Constants));
		_pVertexArray = pAlloc->CreateVertexArray(nTriangles,PRIMITIVE_TRIANGLELIST,&_pVertexBuffer,1,(const u32*)pIndexData->data());
		pAlloc->EndAllocation();
		delete pIndexData;
	}

	int RenderingSheet::Step(RenderingDomain* pRendering)
	{
		sheet_Constants c;
		c.modelview =				pRendering->GetViewMatrix();
		c.inverse_modelview =		c.modelview;
		c.modelview_projection =	pRendering->GetProjectionMatrix()*c.modelview;
		for(int ix= 0;ix<SheetSize;ix++)
			for(int iy= 0;iy<SheetSize;iy++)
			{
				(*_pVertexData)[iy*SheetSize+ix].position = Input1::Get().positions[iy*SheetSize+ix];
				(*_pVertexData)[iy*SheetSize+ix].normal = Input1::Get().normals[iy*SheetSize+ix];
				(*_pVertexData)[iy*SheetSize+ix].color = Vector3<>(1.0f,1.0f,1.0f);
			}
			
		_pVertexBuffer->UpdateContent(_pVertexData->data());
		_pConstants->UpdateContent(&c);
		_pEffect->Draw(_pVertexArray,&_pConstants);
		return 1;
	}

	RenderingSheet::~RenderingSheet()
	{
	}

	DEFINE_CLASS_GUID(RenderingSheet);
}
