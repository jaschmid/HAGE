#include "header.h"
#include "GenericActor.h"
#include "RenderingDomain.h"
#include "UserInterfaceRendering.h"
#include "RActor.h"

namespace HAGE {

	static const char* vertex_program =
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
	"C2E1v_Output vertex(float3 position : POSITION,float3 color : COLOR)\n"
	"{	\n"
	"  C2E1v_Output OUT;\n"
	"\n"
	"  OUT.position = mul( ModelviewProjection, float4( position, 1.0f ) );\n "
	"  OUT.color = float4(color,1.0f);\n"
	"\n"
	"  return OUT;	\n"
	"}";

	static const char* fragment_program =
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


	struct testConstants
	{
		Matrix4<>	modelview;
		Matrix4<>	inverse_modelview;
		Matrix4<>	modelview_projection;
	};

	static const char szDefFormat[] = "DefaultVertexFormat";
	struct DefaultVertexFormat
	{
	public:
		Vector3<>	position;
		Vector3<>	color;

		static const char* name;
	};
	const char* DefaultVertexFormat::name = szDefFormat;

	VertexDescriptionEntry DefFormatDescriptor[] = {
		{"Position",	1,	R32G32B32_FLOAT},
		{"Color",		1,	R32G32B32_FLOAT},
	};
	
	static const float p = (1.0f + sqrtf(5.0f))/4.0f;
	static const float _1 = 0.5f;

	static const DefaultVertexFormat vertices[] =
	{
		{Vector3<>(_1,  0.0f, p),		Vector3<>(1.0f, 0.5f, 0.5f)},
		{Vector3<>(-_1,  0.0f, p),		Vector3<>(0.5f, 1.0f, 0.5f)},
		{Vector3<>(_1,   0.0f, -p),		Vector3<>(0.5f, 0.5f, 1.0f)},
		{Vector3<>(-_1,  0.0f, -p),		Vector3<>(0.0f, 0.5f, 0.5f)},
		{Vector3<>(0.0f, p,	   _1),		Vector3<>(0.5f, 0.0f, 0.5f)},
		{Vector3<>(0.0f, -p,   _1),		Vector3<>(0.5f, 0.0f, 0.5f)},
		{Vector3<>(0.0f, p,	   -_1),	Vector3<>(0.5f, 0.0f, 1.0f)},
		{Vector3<>(0.0f, -p,   -_1),	Vector3<>(1.0f, 0.0f, 0.5f)},
		{Vector3<>(p,	 _1,	0.0f),	Vector3<>(0.0f, 1.0f, 0.5f)},
		{Vector3<>(-p,   _1,	0.0f),	Vector3<>(0.0f, 0.5f, 1.0f)},
		{Vector3<>(p,	 -_1,	0.0f),	Vector3<>(1.0f, 0.5f, 0.0f)},
		{Vector3<>(-p,   -_1,	0.0f),	Vector3<>(0.5f, 1.0f, 0.0f)},
	};


	static const u32	indices[] =
	{
		0,4,1,
		0,1,5,
		0,5,10,
		0,10,8,
		0,8,4,
		4,8,6,
		4,6,9,
		4,9,1,
		1,9,11,
		1,11,5,
		2,7,3,
		2,3,6,
		2,6,8,
		2,8,10,
		2,10,7,
		7,10,5,
		7,5,11,
		7,11,3,
		3,11,9,
		3,9,6,
	};

	bool RenderingDomain::MessageProc(const Message* m)
	{
		if(IsMessageType(m->GetMessageCode(),MESSAGE_UI_UNKNOWN))
		{
			switch(m->GetMessageCode())
			{
			case MESSAGE_UI_ADJUST_CAMERA:
				{
					MessageUIAdjustCamera* pMessage=(MessageUIAdjustCamera*)m;
					fCameraX+=pMessage->GetRotateX();
					fCameraY+=pMessage->GetRotateY();
					fCameraZ+=pMessage->GetZoom();
				}
				return true;
			}
			return pInterface->ProcessUserInterfaceMessage(m);
		}
		switch(m->GetMessageCode())
		{
			case MESSAGE_UI_UNKNOWN:
			default:
				return SharedDomainBase::MessageProc(m);
		}
	}

	void RenderingDomain::DrawIco(Vector3<> position)
	{
		testConstants c;
		c.modelview =				Matrix4<>::Translate(Vector3<>(0.0,0.0,fCameraZ))*Matrix4<>::AngleRotation(Vector3<>(1.0,0.0,0.0),fCameraX)*Matrix4<>::AngleRotation(Vector3<>(0.0,1.0,0.0),fCameraY)*Matrix4<>::Translate(position);
		c.inverse_modelview =		c.modelview;
		c.modelview_projection =	Matrix4<>::Perspective(0.1f,200000.0f,1.3f,3.0f/4.0f*1.3f)*c.modelview;
		pConstants->UpdateContent(&c);
		pEffect->Draw(pVertexArray,&pConstants);
	}

	void RenderingDomain::DomainStep(u64 step)
	{
		pWrapper->BeginFrame();

		auto result = Factory.ForEach<int,RenderingActor>( [this](RenderingActor* o) -> int {return o->Step(this);} , guidNull ,true);

		pWrapper->PresentFrame();
	}

	RenderingDomain::RenderingDomain() : 
		pVertexBuffer(nullptr),pEffect(nullptr),fCameraX(0.0),fCameraY(0.0),fCameraZ(50.0)
	{
		Factory.RegisterObjectType<RenderingActor>();

		//pWrapper = RenderingAPIWrapper::CreateD3D11Wrapper();
		pWrapper = RenderingAPIWrapper::CreateOpenGL3Wrapper();

		pWrapper->BeginFrame();
		pWrapper->RegisterVertexFormat(szDefFormat,DefFormatDescriptor,sizeof(DefFormatDescriptor)/sizeof(VertexDescriptionEntry));
		
		// Create the vertex buffer

		pVertexBuffer = pWrapper->CreateVertexBuffer(DefaultVertexFormat::name,vertices,12);
		pVertexArray = pWrapper->CreateVertexArray(20,PRIMITIVE_TRIANGLELIST,&pVertexBuffer,1,indices);
		pEffect = pWrapper->CreateEffect(vertex_program,fragment_program);
		pConstants = pWrapper->CreateConstantBuffer(sizeof(testConstants));
		pInterface = new UserInterfaceRendering(pWrapper);
		pWrapper->PresentFrame();

		printf("Init Rendering\n");
	}

	RenderingDomain::~RenderingDomain()
	{
		printf("Destroy Rendering\n");

		if(pInterface)delete pInterface;
		if(pVertexBuffer)delete pVertexBuffer;
		if(pVertexArray)delete pVertexArray;
		if(pEffect)delete pEffect;
		if(pConstants)delete pConstants;
		if(pWrapper)delete pWrapper;
	}
}
