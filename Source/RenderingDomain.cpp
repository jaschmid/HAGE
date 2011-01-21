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
	"#define nLights				 3\n"
	"uniform struct\n"
	"{\n"
	"    float4x4 model;\n"
	"    float4x4 inverse_modelview;\n"
	"    float4x4 modelview;\n"
	"    float4x4 modelview_projection;\n"
	"} cbuffer0_TransformGlobal: BUFFER[0];\n"
	"uniform struct\n"
	"{\n"
	"	 float4 light_position_view[nLights];\n"
	"	 float4 light_color[nLights];\n"
	"} cbuffer0_LightGlobal: BUFFER[1];\n"
	"uniform samplerCube ShadowCube1 : TEX0;\n"
	"uniform samplerCube ShadowCube2 : TEX1;\n"
	"uniform samplerCube ShadowCube3 : TEX2;\n"
	"uniform sampler2D DiffuseTexture : TEX3;\n"
	"\n"
	"// cbuffer Transform : register(b0)\n"
	"#define Model					 cbuffer0_TransformGlobal.model\n"
	"#define Modelview               cbuffer0_TransformGlobal.modelview\n"
	"#define InverseModelview        cbuffer0_TransformGlobal.inverse_modelview\n"
	"#define ModelviewProjection     cbuffer0_TransformGlobal.modelview_projection\n"
	"#define LightPosition			 cbuffer0_LightGlobal.light_position_view\n"
	"#define LightColor		         cbuffer0_LightGlobal.light_color\n"
	"#define NearClip(x)			 0.05f\n"
	"#define FarClip(x)		         cbuffer0_LightGlobal.light_position_view[x].w\n"
	"\n"
	"const float BIAS = 0.98f;\n"
	"\n"
	"struct VertexInput {\n"
	"  float3 position : POSITION;\n"
	"  float3 normal : NORMAL;\n"
	"  float3 color : COLOR;\n"
	"  float2 tex		 : TEXCOORD0;\n"
	"};\n"
	"struct VertexOutput {\n"
	"  float4 position : POSITION;\n"
	"  float4 color    : TEXCOORD0;\n"
	"  float3 world_position     : TEXCOORD1;\n"
	"  float3 normal     : TEXCOORD2;\n"
	"  float2 tex		 : TEXCOORD3;\n"
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
	"  VS_OUT.world_position = vPosition;\n "
	"  //float3 lDir = LightPosition[0].xyz-vPosition.xyz;\n "
	"  //float len = lDir.x*lDir.x+lDir.y*lDir.y+lDir.z*lDir.z;\n"
	"  //float3 normDir = lDir/sqrt(len);\n"
	"  //float3 light_intensity = dot(normDir,VS_IN.normal);\n"
	"  //float3 this_color = light_intensity*VS_IN.color*LightColor[0].xyz;\n"
	"  VS_OUT.color = float4(VS_IN.color,1.0f);\n"
	"  VS_OUT.normal = VS_IN.normal;\n"
	"  VS_OUT.tex = VS_IN.tex;\n"
	"  //VS_OUT.lDir.xyz = -lDir;\n"
	"  //float Odist = max(max(abs(VS_OUT.lDir.x),abs(VS_OUT.lDir.y)),abs(VS_OUT.lDir.z));\n"
	"  //Odist = ((FarClip+NearClip)/(FarClip-NearClip)) + (1.0/Odist)*((-2.0*FarClip*NearClip)/(FarClip-NearClip));\n"
    "  //Odist = (Odist+1.0)/2.0*BIAS;\n"
	"  //VS_OUT.lDir.w = Odist;\n"
	"\n"
	"}"
	"// This is C2E2f_passthru from \"The Cg Tutorial\" (Addison-Wesley, ISBN\n"
	"// 0321194969) by Randima Fernando and Mark J. Kilgard.  See page 53.\n"
	"\n"
	"void fragment(in VertexOutput VS_OUT, out FragmentOutput PS_OUT)\n"
	"{\n"
	"  //float Sdist = texCUBE(ShadowCube,float3(VS_OUT.lDir.xyz)).r;\n"
	"  //if(Sdist <= VS_OUT.lDir.w) PS_OUT.color = float4(0.0f,0.0f,0.0,1.0f);\n"
	"  //else PS_OUT.color = VS_OUT.color;\n"
	"  float3 normal = normalize(VS_OUT.normal.xyz);\n"
	"  PS_OUT.color = float4(0.0f,0.0f,0.0f,1.0f);\n"
	"  float3 lDir[nLights];\n"
	"  for(int i =0;i<nLights;++i)\n"
	"  {\n"
	"    lDir[i] = LightPosition[i].xyz-VS_OUT.world_position.xyz;\n "
	"  }\n"
	"  \n"
	"  float Sdist[nLights] = {\n"
	"    texCUBE(ShadowCube1,float3(-lDir[0].xyz)).r,\n"
	"    texCUBE(ShadowCube2,float3(-lDir[1].xyz)).r,\n"
	"    texCUBE(ShadowCube3,float3(-lDir[2].xyz)).r\n"
	"  };\n"
	"  float3 cOdist = float3(\n"
	"    max(max(abs(lDir[0].x),abs(lDir[0].y)),abs(lDir[0].z)),\n"
	"    max(max(abs(lDir[1].x),abs(lDir[1].y)),abs(lDir[1].z)),\n"
	"    max(max(abs(lDir[2].x),abs(lDir[2].y)),abs(lDir[2].z))\n"
	"  )*BIAS;\n"
	"  float3 near = float3(NearClip(0),NearClip(1),NearClip(2));\n"
	"  float3 far = float3(FarClip(0),FarClip(1),FarClip(2));\n"
	"  cOdist = ((far+near)/(far-near)) + (float3(1.0,1.0f,1.0f)/cOdist)*((float3(-2.0,-2.0f,-2.0f)*far*near)/(far-near));\n"
    "  cOdist = (cOdist+1.0)/2.0;\n"
	"  float Odist[3] = {cOdist.x,cOdist.y,cOdist.z};\n"
	"  \n"
	"  for(int i =0;i<nLights;++i)\n"
	"  {\n"
	"      float visibility = step(Odist[i],Sdist[i]);\n"
	"      float3 normDir = normalize(lDir[i]);\n"
	"      float light_intensity = saturate(dot(normDir,normal));\n"
	"      PS_OUT.color.xyz += light_intensity*LightColor[i].xyz*visibility;\n"
	"  }\n"
	"  PS_OUT.color = saturate(float4(PS_OUT.color.xyz*VS_OUT.color.xyz*tex2D(DiffuseTexture,VS_OUT.tex).xyz,1.0f));\n"
	"}\n";


	struct light_constants
	{
		Vector4<>	LightPosition[3];
		Vector4<>	LightColor[3];
	};

	struct shadowcube_constants
	{
		Matrix4<>	CubeMatrix[6];
		Vector4<>	CubeCenter;
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
	"	 float4   cube_center;\n"
	"} cbuffer1_GeometryCubeGlobal: BUFFER[1];\n"
	"\n"
	"// cbuffer Transform : register(b0)\n"
	"#define Modelview               cbuffer0_TransformGlobal.modelview\n"
	"#define InverseModelview        cbuffer0_TransformGlobal.inverse_modelview\n"
	"#define ModelviewProjection     cbuffer0_TransformGlobal.modelview_projection\n"
	"#define CubeSide				 cbuffer1_GeometryCubeGlobal.cube_side\n"
	"#define CubeCenter				 cbuffer1_GeometryCubeGlobal.cube_center\n"
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
	"\n"
	"\n"
	"void OutputTriangleToStream(int iStream,float4 position[3])\n"
	"{\n"
	"	// Compute screen coordinates\n"
	"	GeometryOutput output;\n"
	"	output.layer = (uint)iStream;\n"
	"	for( int v = 0; v < 3; v++ )\n"
	"	{\n"
	"		output.position =  mul( CubeSide[iStream], position[v] );\n"
	"		output.fragment.color = float4(output.position.w,output.position.w,output.position.w,1.0f);\n"
	"		emitVertex(output);\n"
	"	}\n"
	"	restartStrip();\n"
	"}\n"
	"int GetCubeSide(float3 position)\n"
	"{\n"
	"	float3 abs_position = abs(position);\n"
	"	if(abs_position.x > abs_position.y && abs_position.x > abs_position.z) return (position.x>0.0f)?0:1;\n"
	"	else if(abs_position.y > abs_position.z) return (position.y>0.0f)?2:3;\n"
	"	else return (position.z>0.0f)?4:5;\n"
	"}\n"
	"\n"
	"\n"
	"TRIANGLE void geometry( AttribArray< VertexOutput > input)\n"
	"{\n"
	"	for( int t = 0; t < input.length / 3; ++t)\n"
	"	{\n"
	"		float4 position[3] = {input[t*3].position,input[t*3+1].position,input[t*3+2].position};\n"
	"		int Side1 = GetCubeSide(position[0].xyz-CubeCenter.xyz);\n"
	"		int Side2 = GetCubeSide(position[1].xyz-CubeCenter.xyz);\n"
	"		int Side3 = GetCubeSide(position[2].xyz-CubeCenter.xyz);\n"
	"		OutputTriangleToStream(Side1,position);\n"
	"		if(Side2 != Side1)OutputTriangleToStream(Side2,position);\n"
	"		if(Side3 != Side1 && Side3 != Side2)OutputTriangleToStream(Side3,position);\n"
	"		//for( int f = 0; f < 6; ++f )\n"
	"		//{\n"
	"		//	OutputTriangleToStream(f,position);\n"
	"		//}\n"
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
		return _light[u];
	}

	void RenderingDomain::DomainStep(u64 step)
	{
		pWrapper->BeginFrame();

		const int max_lights = 3;
		
		auto result3 = Factory.ForEach<GLightOut,RenderingLight>( [this](RenderingLight* o) -> GLightOut {return o->Step(this);} , guid_of<RenderingLight>::Get() ,true);
		int nLights = std::min<u32>(result3.second,max_lights);
		light_constants lc;
		int i = 0;
		for(; i < nLights; ++i)
		{
			_light[i] = result3.first[i];
		}
		for(i=0; i < nLights; ++i)
		{
			//draw light
			
			_lightCubeDepth[i]->Clear(true,1.0f);

			pWrapper->SetRenderTarget(nullptr,_lightCubeDepth[i]);

			
			position_constants pc;
			pc.model	=				Matrix4<>::One();
			pc.modelview =				Matrix4<>::One();
			pc.inverse_modelview =		Matrix4<>::One();
			pc.modelview_projection =	pc.modelview;

			shadowcube_constants sc;
			
			Vector3<> vEyePt = Vector3<>(0.0f,0.0f,0.0f);
			Vector3<> vLookDir;
			Vector3<> vUpDir;

			Matrix4<> projection = Matrix4<>::Perspective(0.05f,_light[i].Range,3.1415926536/2.0f,3.1415926536/2.0f);
			Matrix4<> light_position = Matrix4<>::Translate(-_light[i].Position);

			vLookDir = Vector3<>( 1.0f, 0.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[0] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position);
			vLookDir = Vector3<>( -1.0f, 0.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[1] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position);
			vLookDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 0.0f, -1.0f );
			sc.CubeMatrix[2] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position);
			vLookDir = Vector3<>( 0.0f, -1.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 0.0f, 1.0f );
			sc.CubeMatrix[3] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position);
			vLookDir = Vector3<>( 0.0f, 0.0f, 1.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[4] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position);
			vLookDir = Vector3<>( 0.0f, 0.0f, -1.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[5] = projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position);

			sc.CubeCenter = Vector4<>(_light[i].Position,1.0f);

			_pShadowcubeConstants->UpdateContent(&sc);
			
			auto result = Factory.ForEach<int,RenderingActor>( [&](RenderingActor* o) -> int {return o->Draw(_pShadowmapEffect,pc,_pConstants);} , guid_of<RenderingActor>::Get() ,true);
			auto result2 = Factory.ForEach<int,RenderingSheet>( [&](RenderingSheet* o) -> int {return o->Draw(_pShadowmapEffect,pc,_pConstants);} , guid_of<RenderingSheet>::Get() ,true);

			pWrapper->SetRenderTarget(nullptr,nullptr);

			lc.LightPosition[i] = Vector4<>(_light[i].Position,_light[i].Range);
			lc.LightColor[i] = Vector4<>(_light[i].Color,1.0f);
		}
		for(;i<max_lights;++i)
		{
			lc.LightPosition[i] = Vector4<>(0.0f,0.0f,0.0f,0.0f);
			lc.LightColor[i] = Vector4<>(0.0f,0.0f,0.0f,0.0f);
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
		_lightCubeDepth[0] = pWrapper->CreateTexture(1024,1024,1,R16_UNORM,TEXTURE_CUBE | TEXTURE_GPU_DEPTH_STENCIL,nullptr);
		_lightCubeDepth[1] = pWrapper->CreateTexture(1024,1024,1,R16_UNORM,TEXTURE_CUBE | TEXTURE_GPU_DEPTH_STENCIL,nullptr);
		_lightCubeDepth[2] = pWrapper->CreateTexture(1024,1024,1,R16_UNORM,TEXTURE_CUBE | TEXTURE_GPU_DEPTH_STENCIL,nullptr);

		_pEffect = new EffectContainer(pWrapper,default_program);
		_pConstants = pWrapper->CreateConstantBuffer(sizeof(position_constants));
		_pLightConstants = pWrapper->CreateConstantBuffer(sizeof(light_constants));
		_pShadowcubeConstants = pWrapper->CreateConstantBuffer(sizeof(shadowcube_constants));
		_pEffect->SetConstant(0,_pConstants);
		_pEffect->SetConstant(1,_pLightConstants);
		_pEffect->SetTexture(0,_lightCubeDepth[0]);
		_pEffect->SetTexture(1,_lightCubeDepth[1]);
		_pEffect->SetTexture(2,_lightCubeDepth[2]);

		
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
