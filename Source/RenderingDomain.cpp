#include "header.h"
#include "GenericActor.h"
#include "RenderingDomain.h"
#include "UserInterfaceRendering.h"
#include "RActor.h"
#include "RSheet.h"
#include "RLight.h"

#include "SettingsLoader.h"

namespace HAGE {

	static const char* default_program =
	"// This is C2E1v_green from \"The Cg Tutorial\" (Addison-Wesley, ISBN\n"
	"// 0321194969) by Randima Fernando and Mark J. Kilgard.  See page 38.\n"
	"uniform struct\n"
	"{\n"
	"    float4x4 model;\n"
	"    float4x4 inverse_modelview;\n"
	"    float4x4 modelview;\n"
	"    float4x4 modelview_projection;\n"
	"} cbuffer0_TransformGlobal: BUFFER[0];\n"
	"uniform struct\n"
	"{\n"
	"	 float4 light_position_view;\n"
	"	 float4 light_color;\n"
	"} cbuffer0_LightGlobal: BUFFER[1];\n"
	"uniform samplerCube ShadowCube : TEX0;\n"
	"\n"
	"// cbuffer Transform : register(b0)\n"
	"#define Model					 cbuffer0_TransformGlobal.model\n"
	"#define Modelview               cbuffer0_TransformGlobal.modelview\n"
	"#define InverseModelview        cbuffer0_TransformGlobal.inverse_modelview\n"
	"#define ModelviewProjection     cbuffer0_TransformGlobal.modelview_projection\n"
	"#define LightPosition			 cbuffer0_LightGlobal.light_position_view.xyz\n"
	"#define LightColor		         cbuffer0_LightGlobal.light_color\n"
	"#define NearClip				 0.5f\n"
	"#define FarClip		         cbuffer0_LightGlobal.light_position_view.w\n"
	"\n"
	"const float BIAS = 0.98f;\n"
	"\n"
	"struct VertexInput {\n"
	"  float3 position : POSITION;\n"
	"  float3 normal : NORMAL;\n"
	"  float3 color : COLOR;\n"
	"};\n"
	"struct VertexOutput {\n"
	"  float4 position : POSITION;\n"
	"  float4 color    : TEXCOORD0;\n"
	"  float4 lDir     : TEXCOORD1;\n"
	"};\n"
	"struct FragmentOutput {\n"
	"  float4 color : COLOR;\n"
	"};\n"
	"\n"
	"\n"
	"void vertex(in VertexInput VS_IN,out VertexOutput VS_OUT)\n"
	"{	\n"
	"\n"
	"  VS_OUT.position = mul( ModelviewProjection, float4( VS_IN.position, 1.0f ) );\n "
	"  float3 vPosition = mul( Model, float4( VS_IN.position, 1.0f ) ).xyz;\n "
	"  float3 lDir = LightPosition.xyz-vPosition.xyz;\n "
	"  float len = lDir.x*lDir.x+lDir.y*lDir.y+lDir.z*lDir.z;\n"
	"  float3 normDir = lDir/sqrt(len);\n"
	"  float3 light_intensity = dot(normDir,VS_IN.normal);\n"
	"  float3 this_color = light_intensity*VS_IN.color*LightColor.xyz;\n"
	"  VS_OUT.color = float4(this_color,1.0f);\n"
	"  VS_OUT.lDir.xyz = -lDir*BIAS;\n"
	"  float Odist = max(max(abs(VS_OUT.lDir.x),abs(VS_OUT.lDir.y)),abs(VS_OUT.lDir.z));\n"
	"  Odist = ((FarClip+NearClip)/(FarClip-NearClip)) + (1.0/Odist)*((-2.0*FarClip*NearClip)/(FarClip-NearClip));\n"
    "  Odist = (Odist+1.0)/2.0;\n"
	"  VS_OUT.lDir.w = Odist;\n"
	"\n"
	"}"
	"// This is C2E2f_passthru from \"The Cg Tutorial\" (Addison-Wesley, ISBN\n"
	"// 0321194969) by Randima Fernando and Mark J. Kilgard.  See page 53.\n"
	"\n"
	"void fragment(in VertexOutput VS_OUT, out FragmentOutput PS_OUT)\n"
	"{\n"
	"  float Sdist = texCUBE(ShadowCube,float3(VS_OUT.lDir.xyz)).r;\n"
	"  if(Sdist <= VS_OUT.lDir.w) PS_OUT.color = float4(0.0f,0.0f,0.0,1.0f);\n"
	"  else PS_OUT.color = VS_OUT.color;\n"
	"}\n";


	struct light_constants
	{
		Vector4<>	LightPosition;
		Vector4<>	LightColor;
	};

	struct shadowcube_constants
	{
		Matrix4<>	CubeMatrix[6];
	};

	static const char* shadowmap_program =
	"// This is C2E1v_green from \"The Cg Tutorial\" (Addison-Wesley, ISBN\n"
	"// 0321194969) by Randima Fernando and Mark J. Kilgard.  See page 38.\n"
	"uniform struct\n"
	"{\n"
	"    float4x4 model;\n"
	"    float4x4 inverse_modelview;\n"
	"    float4x4 modelview;\n"
	"    float4x4 modelview_projection;\n"
	"} cbuffer0_TransformGlobal: BUFFER[0];\n"
	"uniform struct\n"
	"{\n"
	"    float4x4 cube_side[6];\n"
	"} cbuffer1_GeometryCubeGlobal: BUFFER[1];\n"
	"\n"
	"// cbuffer Transform : register(b0)\n"
	"#define Modelview               cbuffer0_TransformGlobal.modelview\n"
	"#define InverseModelview        cbuffer0_TransformGlobal.inverse_modelview\n"
	"#define ModelviewProjection     cbuffer0_TransformGlobal.modelview_projection\n"
	"#define CubeSide				 cbuffer1_GeometryCubeGlobal.cube_side\n"
	"\n"
	"\n"
	"struct VertexInput {\n"
	"  float3 position : POSITION;\n"
	"  float3 normal : NORMAL;\n"
	"  float3 color : COLOR;\n"
	"};\n"
	"struct VertexOutput {\n"
	"  float4 position : POSITION;\n"
	"  float4 color    : TEXCOORD0;\n"
	"};\n"
	"struct FragmentInput {\n"
	"  float4 color : COLOR;\n"
	"};\n"
	"struct GeometryOutput {\n"
	"  float4 position : POSITION;\n"
	"  uint layer : LAYER;\n"
	"  FragmentInput fragment;\n"
	"};\n"
	"struct FragmentOutput {\n"
	"  float4 color : COLOR;\n"
	"};\n"
	"\n"
	"\n"
	"void vertex(in VertexInput VS_IN,out VertexOutput VS_OUT)\n"
	"{	\n"
	"\n"
	"  VS_OUT.position = mul( Modelview, float4(VS_IN.position, 1.0f) );\n "
	"  VS_OUT.color = float4(VS_IN.color,1.0f);\n"
	"\n"
	"}"
	"TRIANGLE void geometry( AttribArray< VertexOutput > input)\n"
	"{\n"
	"	for( int t = 0; t < input.length / 3; ++t)\n"
	"	{\n"
	"		for( int f = 0; f < 6; ++f )\n"
	"		{\n"
	"			// Compute screen coordinates\n"
	"			GeometryOutput output;\n"
	"			output.layer = (uint)f;\n"
	"			for( int v = 0; v < 3; v++ )\n"
	"			{\n"
	"				output.position =  mul( CubeSide[f], input[t*3+v].position );\n"
	"				output.fragment.color = float4(output.position.w,output.position.w,output.position.w,1.0f);\n"
	"				emitVertex(output);\n"
	"			}\n"
	"			restartStrip();\n"
	"		}\n"
	"	}\n"
	"}\n"
	"// This is C2E2f_passthru from \"The Cg Tutorial\" (Addison-Wesley, ISBN\n"
	"// 0321194969) by Randima Fernando and Mark J. Kilgard.  See page 53.\n"
	"\n"
	"void fragment(in FragmentInput PS_IN, out FragmentOutput PS_OUT)\n"
	"{\n"
	"  PS_OUT.color = PS_IN.color;\n"
	"}\n";

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
					_viewMatrix = Matrix4<>::Translate(Vector3<>(0.0,0.0,fCameraZ))*Matrix4<>::AngleRotation(Vector3<>(1.0,0.0,0.0),fCameraX)*Matrix4<>::AngleRotation(Vector3<>(0.0,1.0,0.0),fCameraY);
					_invViewMatrix = Matrix4<>::AngleRotation(Vector3<>(0.0,1.0,0.0),-fCameraY)*Matrix4<>::AngleRotation(Vector3<>(1.0,0.0,0.0),-fCameraX)*Matrix4<>::Translate(Vector3<>(0.0,0.0,-fCameraZ));
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

	const Matrix4<>& RenderingDomain::GetProjectionMatrix()
	{
		return _projectionMatrix;
	}

	const Matrix4<>& RenderingDomain::GetViewMatrix()
	{
		return _viewMatrix;
	}

	const Matrix4<>& RenderingDomain::GetInvViewMatrix()
	{
		return _invViewMatrix;
	}
	
	const GLightOut& RenderingDomain::GetLight(u32 u)
	{
		return _light1;
	}

	void RenderingDomain::DomainStep(u64 step)
	{
		pWrapper->BeginFrame();

		
		auto result3 = Factory.ForEach<GLightOut,RenderingLight>( [this](RenderingLight* o) -> GLightOut {return o->Step(this);} , guid_of<RenderingLight>::Get() ,true);
		if(result3.second)
		{
			_light1 = result3.first[0];

			//draw light
			
			_light1CubeDepth->Clear(true,1.0f);

			pWrapper->SetRenderTarget(nullptr,_light1CubeDepth);

			
			position_constants pc;
			pc.model	=				Matrix4<>::One();
			pc.modelview =				Matrix4<>::One();
			pc.inverse_modelview =		Matrix4<>::One();
			pc.modelview_projection =	pc.modelview;

			shadowcube_constants sc;
			
			Vector3<> vEyePt = Vector3<>(0.0f,0.0f,0.0f);
			Vector3<> vLookDir;
			Vector3<> vUpDir;

			Matrix4<> projection = Matrix4<>::Perspective(0.5f,_light1.Range,3.1415926536/2.0f,3.1415926536/2.0f);

			vLookDir = Vector3<>( 1.0f, 0.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[0] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*Matrix4<>::Translate(-_light1.Position));
			vLookDir = Vector3<>( -1.0f, 0.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[1] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*Matrix4<>::Translate(-_light1.Position));
			vLookDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 0.0f, -1.0f );
			sc.CubeMatrix[2] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*Matrix4<>::Translate(-_light1.Position));
			vLookDir = Vector3<>( 0.0f, -1.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 0.0f, 1.0f );
			sc.CubeMatrix[3] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*Matrix4<>::Translate(-_light1.Position));
			vLookDir = Vector3<>( 0.0f, 0.0f, 1.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[4] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*Matrix4<>::Translate(-_light1.Position));
			vLookDir = Vector3<>( 0.0f, 0.0f, -1.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[5] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*Matrix4<>::Translate(-_light1.Position));

			_pShadowcubeConstants->UpdateContent(&sc);
			
			auto result = Factory.ForEach<int,RenderingActor>( [&](RenderingActor* o) -> int {return o->Draw(_pShadowmapEffect,pc,_pConstants);} , guid_of<RenderingActor>::Get() ,true);
			auto result2 = Factory.ForEach<int,RenderingSheet>( [&](RenderingSheet* o) -> int {return o->Draw(_pShadowmapEffect,pc,_pConstants);} , guid_of<RenderingSheet>::Get() ,true);

			pWrapper->SetRenderTarget(nullptr,nullptr);
		}
		position_constants c;
		c.model	=					Matrix4<>::One();
		c.modelview =				GetViewMatrix();
		c.inverse_modelview =		GetInvViewMatrix();
		c.modelview_projection =	GetProjectionMatrix()*c.modelview;
	/*
		c.model	=					Matrix4<>::One();
		c.modelview =				Matrix4<>::One();
		c.inverse_modelview =		Matrix4<>::One();
		c.modelview_projection =	c.modelview;*/
		
		light_constants lc;
		lc.LightPosition = Vector4<>(_light1.Position,_light1.Range);
		lc.LightColor = Vector4<>(_light1.Color,1.0f);
		_pLightConstants->UpdateContent(&lc);

		auto result = Factory.ForEach<int,RenderingActor>( [&](RenderingActor* o) -> int {return o->Draw(_pEffect,c,_pConstants);} , guid_of<RenderingActor>::Get() ,true);
		auto result2 = Factory.ForEach<int,RenderingSheet>( [&](RenderingSheet* o) -> int {return o->Draw(_pEffect,c,_pConstants);} , guid_of<RenderingSheet>::Get() ,true);

		pWrapper->PresentFrame();
	}

	RenderingDomain::RenderingDomain() :
		fCameraX(settings->getf32Setting("cam_x")),
		fCameraY(settings->getf32Setting("cam_y")),
		fCameraZ(settings->getf32Setting("cam_z"))
	{
		Factory.RegisterObjectType<RenderingActor>();
		Factory.RegisterObjectType<RenderingSheet>();
		Factory.RegisterObjectType<RenderingLight>();

		pWrapper = RenderingAPIWrapper::CreateD3D11Wrapper();
		//pWrapper = RenderingAPIWrapper::CreateOpenGL3Wrapper();

		pWrapper->BeginAllocation();

		// Create the vertex buffer

		pInterface = new UserInterfaceRendering(pWrapper);

		// Create the Light Depth Cube
		
		//_light1Cube = pWrapper->CreateTexture(512,512,1,R32_FLOAT,TEXTURE_CUBE | TEXTURE_GPU_WRITE,nullptr);
		_light1CubeDepth = pWrapper->CreateTexture(1024,1024,1,R16_UNORM,TEXTURE_CUBE | TEXTURE_GPU_DEPTH_STENCIL,nullptr);

		_pEffect = new EffectContainer(pWrapper,default_program);
		_pConstants = pWrapper->CreateConstantBuffer(sizeof(position_constants));
		_pLightConstants = pWrapper->CreateConstantBuffer(sizeof(light_constants));
		_pShadowcubeConstants = pWrapper->CreateConstantBuffer(sizeof(shadowcube_constants));
		_pEffect->SetConstant(0,_pConstants);
		_pEffect->SetConstant(1,_pLightConstants);
		_pEffect->SetTexture(0,_light1CubeDepth);

		
		_pShadowmapEffect = new EffectContainer(pWrapper,shadowmap_program);
		_pShadowmapEffect->SetConstant(0,_pConstants);
		_pShadowmapEffect->SetConstant(1,_pShadowcubeConstants);

		pWrapper->EndAllocation();

		_projectionMatrix = Matrix4<>::Perspective(0.1f,200000.0f,1.3f,3.0f/4.0f*1.3f);
		_viewMatrix = Matrix4<>::Translate(Vector3<>(0.0,0.0,fCameraZ))*Matrix4<>::AngleRotation(Vector3<>(1.0,0.0,0.0),fCameraX)*Matrix4<>::AngleRotation(Vector3<>(0.0,1.0,0.0),fCameraY);
		_invViewMatrix = Matrix4<>::AngleRotation(Vector3<>(0.0,1.0,0.0),-fCameraY)*Matrix4<>::AngleRotation(Vector3<>(1.0,0.0,0.0),-fCameraX)*Matrix4<>::Translate(Vector3<>(0.0,0.0,-fCameraZ));

		printf("Init Rendering\n");
	}

	RenderingDomain::~RenderingDomain()
	{
		printf("Destroy Rendering\n");
		if(_pShadowmapEffect)delete _pShadowmapEffect;
		if(_pEffect)delete _pEffect;
		if(_pConstants)delete _pConstants;
		if(_pLightConstants)delete _pLightConstants;
		if(_pShadowcubeConstants)delete _pShadowcubeConstants;
		if(pInterface)delete pInterface;
		if(pWrapper)delete pWrapper;
	}
}
