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
	"#define nLights				 3\n"
	"H_CONSTANT_BUFFER_BEGIN(TransformGlobal)\n"
	"    float4x4 model;\n"
	"    float4x4 inverse_modelview;\n"
	"    float4x4 modelview;\n"
	"    float4x4 modelview_projection;\n"
	"    float ambient_factor;\n"
	"    float diffuse_factor;\n"
	"H_CONSTANT_BUFFER_END\n"
	"\n"
	"H_CONSTANT_BUFFER_BEGIN(LightGlobal)\n"
	"	 float4 light_position_arg1[nLights];\n"
	"	 float4 light_color_arg2[nLights];\n"
	"H_CONSTANT_BUFFER_END\n"
	"\n"
	"H_TEXTURE_CUBE_CMP(ShadowCube1);\n"
	"H_TEXTURE_CUBE_CMP(ShadowCube2);\n"
	"H_TEXTURE_CUBE_CMP(ShadowCube3);\n"
	"H_TEXTURE_2D(DiffuseTexture);\n"
	"\n"
	"// cbuffer Transform : register(b0)\n"
	"#define Model					 model\n"
	"#define Modelview               modelview\n"
	"#define InverseModelview        inverse_modelview\n"
	"#define ModelviewProjection     modelview_projection\n"
	"#define LightPosition(x)		  (light_position_arg1[x].xyz)\n"
	"#define LightColor(x)		      (light_color_arg2[x].xyz)\n"
	"#define Arg1(x)				 (light_position_arg1[x].w)\n"
	"#define Arg2(x)		         (light_color_arg2[x].w)\n"
	"\n"
	"static const float BIAS = 0.998f;\n"
	"\n"
	"VS_IN_BEGIN\n"
	"  DECL_VS_IN(float3,position);\n"
	"  DECL_VS_IN(float3,normal);\n"
	"  DECL_VS_IN(float3,color);\n"
	"  DECL_VS_IN(float2,texcoord);\n"
	"VS_IN_END\n"
	"VS_OUT_BEGIN\n"
	"  DECL_VS_OUT(float4,color);\n"
	"  DECL_VS_OUT(float3,world_position);\n"
	"  DECL_VS_OUT(float3,normal);\n"
	"  DECL_VS_OUT(float2,tex);\n"
	"  DECL_VS_POSITION;\n"
	"VS_OUT_END\n"
	"\n"
	"VERTEX_SHADER\n"
	"{	\n"
	"\n"
	"  VS_OUT_POSITION = mul( ModelviewProjection, float4( VS_IN(position), 1.0f ) );\n "
	"  float3 vPosition = mul( Model, float4( VS_IN(position), 1.0f ) ).xyz;\n "
	"  VS_OUT(world_position) = vPosition;\n "
	"  VS_OUT(color) = float4(VS_IN(color),1.0f);\n"
	"  VS_OUT(normal) = normalize(mul( Model, float4( VS_IN(normal), 1.0f ) ).xyz - mul( Model, float4( 0.0f,0.0f,0.0f, 1.0f ) ).xyz);\n"
	"  VS_OUT(tex) = VS_IN(texcoord);\n"
	"\n"
	"}"
	"FS_IN_BEGIN\n"
	"  DECL_FS_IN(float4,color);\n"
	"  DECL_FS_IN(float3,world_position);\n"
	"  DECL_FS_IN(float3,normal);\n"
	"  DECL_FS_IN(float2,tex);\n"
	"FS_IN_END\n"
	"FS_OUT_BEGIN\n"
	"  DECL_FS_COLOR;\n"
	"FS_OUT_END\n"
	"\n"
	"FRAGMENT_SHADER\n"
	"{\n"
	"  //float Sdist = texCUBE(ShadowCube,float3(VS_OUT.lDir.xyz)).r;\n"
	"  //if(Sdist <= VS_OUT.lDir.w) PS_OUT.color = float4(0.0f,0.0f,0.0,1.0f);\n"
	"  //else PS_OUT.color = VS_OUT.color;\n"
	"  float3 normal = normalize(FS_IN(normal).xyz);\n"
	"  FS_OUT_COLOR = float4(0.0f,0.0f,0.0f,1.0f);\n"
	"  float3 lDir[nLights];\n"
	"  for(int i =0;i<nLights;++i)\n"
	"  {\n"
	"    lDir[i] = LightPosition(i)-FS_IN(world_position).xyz;\n "
	"  }\n"
	"  \n"
	"  float3 cOdist = float3(\n"
	"    max(max(abs(lDir[0].x),abs(lDir[0].y)),abs(lDir[0].z)),\n"
	"    max(max(abs(lDir[1].x),abs(lDir[1].y)),abs(lDir[1].z)),\n"
	"    max(max(abs(lDir[2].x),abs(lDir[2].y)),abs(lDir[2].z))\n"
	"  );\n"
	"  float3 arg1 = float3(Arg1(0),Arg1(1),Arg1(2));//((far+near)/(far-near))\n"
	"  float3 arg2 = float3(Arg2(0),Arg2(1),Arg2(2));//((float3(-2.0,-2.0f,-2.0f)*far*near)/(far-near))\n"
	"  cOdist = (arg1 + (float3(1.0,1.0f,1.0f)/cOdist)*arg2+1.0)/2.0;\n"
	"  float Odist[3]; Odist[0]=cOdist.x;Odist[1]=cOdist.y;Odist[2]=cOdist.z;\n"
	"  \n"
	"  float visibility[nLights];\n"
	"  visibility[0] =  H_SAMPLE_CUBE_CMP(ShadowCube1,float3(-lDir[0].xyz),Odist[0] * BIAS);\n"
	"  visibility[1] =  H_SAMPLE_CUBE_CMP(ShadowCube2,float3(-lDir[1].xyz),Odist[1] * BIAS);\n"
	"  visibility[2] =  H_SAMPLE_CUBE_CMP(ShadowCube3,float3(-lDir[2].xyz),Odist[2] * BIAS);\n"
	"  \n"
	"  for(int i =0;i<nLights;++i)\n"
	"  {\n"
	"      float dirLen = dot(lDir[i],lDir[i]);\n"
	"      float3 normDir = lDir[i]*rsqrt(dirLen);\n"
	"      float light_intensity = saturate(dot(normDir,normal))*saturate(1.0f-dirLen/100.0f)*diffuse_factor;\n"
	"      FS_OUT_COLOR.xyz += (light_intensity*visibility[i] + ambient_factor)*LightColor(i);\n"
	"  }\n"
	"  FS_OUT_COLOR = saturate(float4(FS_OUT_COLOR.xyz*FS_IN(color).xyz*H_SAMPLE_2D(DiffuseTexture,FS_IN(tex)).xyz,1.0f));\n"
	"}\n";
	
	static const char* vt_test =
	"#define nLights				 3\n"
	"H_CONSTANT_BUFFER_BEGIN(TransformGlobal)\n"
	"    float4x4 model;\n"
	"    float4x4 inverse_modelview;\n"
	"    float4x4 modelview;\n"
	"    float4x4 modelview_projection;\n"
	"    float ambient_factor;\n"
	"    float diffuse_factor;\n"
	"H_CONSTANT_BUFFER_END\n"
	"\n"
	"H_CONSTANT_BUFFER_BEGIN(LightGlobal)\n"
	"	 float4 light_position_arg1[nLights];\n"
	"	 float4 light_color_arg2[nLights];\n"
	"H_CONSTANT_BUFFER_END\n"
	"\n"
	"H_CONSTANT_BUFFER_BEGIN(VTSettings)\n"
	"	 float4 tex_inner_cache_page_size__tex_cache_border_size;\n"
	"	 float4 tex_outer_cache_page_size__tex_cache_factor;\n"
	"H_CONSTANT_BUFFER_END\n"
	"\n"
	"H_TEXTURE_CUBE_CMP(ShadowCube1);\n"
	"H_TEXTURE_CUBE_CMP(ShadowCube2);\n"
	"H_TEXTURE_CUBE_CMP(ShadowCube3);\n"
	"H_TEXTURE_2D(VT_Cache);\n"
	"H_TEXTURE_2D(VT_Redirection);\n"
	"\n"
	"// cbuffer Transform : register(b0)\n"
	"#define Model					 model\n"
	"#define Modelview               modelview\n"
	"#define InverseModelview        inverse_modelview\n"
	"#define ModelviewProjection     modelview_projection\n"
	"#define LightPosition(x)		  (light_position_arg1[x].xyz)\n"
	"#define LightColor(x)		      (light_color_arg2[x].xyz)\n"
	"#define Arg1(x)				 (light_position_arg1[x].w)\n"
	"#define Arg2(x)		         (light_color_arg2[x].w)\n"
	"#define CacheInnerPageSize		 (tex_inner_cache_page_size__tex_cache_border_size.xy)\n"
	"#define CacheBorderSize		 (tex_inner_cache_page_size__tex_cache_border_size.zw)\n"
	"#define CacheOuterPageSize		 (tex_outer_cache_page_size__tex_cache_factor.xy)\n"
	"#define CacheFactor			 (tex_outer_cache_page_size__tex_cache_factor.zw)\n"
	"\n"
	"static const float BIAS = 0.9998f;\n"
	"\n"
	"VS_IN_BEGIN\n"
	"  DECL_VS_IN(float3,position);\n"
	"  DECL_VS_IN(float3,normal);\n"
	"  DECL_VS_IN(float3,color);\n"
	"  DECL_VS_IN(float2,texcoord);\n"
	"VS_IN_END\n"
	"VS_OUT_BEGIN\n"
	"  DECL_VS_OUT(float4,color);\n"
	"  DECL_VS_OUT(float3,world_position);\n"
	"  DECL_VS_OUT(float3,normal);\n"
	"  DECL_VS_OUT(float2,tex);\n"
	"  DECL_VS_POSITION;\n"
	"VS_OUT_END\n"
	"\n"
	"VERTEX_SHADER\n"
	"{	\n"
	"\n"
	"  VS_OUT_POSITION = mul( ModelviewProjection, float4( VS_IN(position), 1.0f ) );\n "
	"  float3 vPosition = mul( Model, float4( VS_IN(position), 1.0f ) ).xyz;\n "
	"  VS_OUT(world_position) = vPosition;\n "
	"  VS_OUT(color) = float4(VS_IN(color),1.0f);\n"
	"  VS_OUT(normal) = normalize(mul( Model, float4( VS_IN(normal), 1.0f ) ).xyz - mul( Model, float4( 0.0f,0.0f,0.0f, 1.0f ) ).xyz);\n"
	"  VS_OUT(tex) = VS_IN(texcoord);\n"
	"\n"
	"}"
	"FS_IN_BEGIN\n"
	"  DECL_FS_IN(float4,color);\n"
	"  DECL_FS_IN(float3,world_position);\n"
	"  DECL_FS_IN(float3,normal);\n"
	"  DECL_FS_IN(float2,tex);\n"
	"FS_IN_END\n"
	"FS_OUT_BEGIN\n"
	"  DECL_FS_COLOR;\n"
	"FS_OUT_END\n"
	"\n"
	"float mipmapLevelAni(float2 dx,float2 dy)\n"
	"{ \n"
	"  float Pmax = max(dot(dx,dx), dot(dy,dy));\n"
	"  float Pmin = min(dot(dx,dx), dot(dy,dy));\n"
	"  float N = min(ceil(Pmax/Pmin), 8.0f*8.0f);\n"
	"  return min(max(0.0f, ceil(-0.5f*log2(Pmax/N) -7.0f)),16.0f);\n"
	"} \n"
	"FRAGMENT_SHADER\n"
	"{\n"
	"  //float Sdist = texCUBE(ShadowCube,float3(VS_OUT.lDir.xyz)).r;\n"
	"  //if(Sdist <= VS_OUT.lDir.w) PS_OUT.color = float4(0.0f,0.0f,0.0,1.0f);\n"
	"  //else PS_OUT.color = VS_OUT.color;\n"/*
	"  float3 normal = normalize(FS_IN(normal).xyz);\n"
	"  FS_OUT_COLOR = float4(0.0f,0.0f,0.0f,1.0f);\n"
	"  float3 lDir[nLights];\n"
	"  for(int i =0;i<nLights;++i)\n"
	"  {\n"
	"    lDir[i] = LightPosition(i)-FS_IN(world_position).xyz;\n "
	"  }\n"
	"  \n"
	"  float3 cOdist = float3(\n"
	"    max(max(abs(lDir[0].x),abs(lDir[0].y)),abs(lDir[0].z)),\n"
	"    max(max(abs(lDir[1].x),abs(lDir[1].y)),abs(lDir[1].z)),\n"
	"    max(max(abs(lDir[2].x),abs(lDir[2].y)),abs(lDir[2].z))\n"
	"  );\n"
	"  float3 arg1 = float3(Arg1(0),Arg1(1),Arg1(2));//((far+near)/(far-near))\n"
	"  float3 arg2 = float3(Arg2(0),Arg2(1),Arg2(2));//((float3(-2.0,-2.0f,-2.0f)*far*near)/(far-near))\n"
	"  cOdist = (arg1 + (float3(1.0,1.0f,1.0f)/cOdist)*arg2+1.0)/2.0;\n"
	"  float Odist[3]; Odist[0]=cOdist.x;Odist[1]=cOdist.y;Odist[2]=cOdist.z;\n"
	"  \n"
	"  float visibility[nLights];\n"
	"  visibility[0] =  H_SAMPLE_CUBE_CMP(ShadowCube1,float3(-lDir[0].xyz),Odist[0] * BIAS);\n"
	"  visibility[1] =  H_SAMPLE_CUBE_CMP(ShadowCube2,float3(-lDir[1].xyz),Odist[1] * BIAS);\n"
	"  visibility[2] =  H_SAMPLE_CUBE_CMP(ShadowCube3,float3(-lDir[2].xyz),Odist[2] * BIAS);\n"
	"  \n"
	"  for(int i =0;i<nLights;++i)\n"
	"  {\n"
	"      float dirLen = dot(lDir[i],lDir[i]);\n"
	"      float3 normDir = lDir[i]*rsqrt(dirLen);\n"
	"      float light_intensity = saturate(dot(normDir,normal))*saturate(1.0f-dirLen/100.0f)*diffuse_factor;\n"
	"      FS_OUT_COLOR.xyz += (light_intensity*visibility[i] + ambient_factor)*LightColor(i);\n"
	"  }\n"*/
	"  float2 dx = ddx(FS_IN(tex));\n"
	"  float2 dy = ddy(FS_IN(tex));\n"
	"  float lambda = mipmapLevelAni(dx,dy);\n"
	"  uint val = asuint(H_SAMPLE_2D_LOD(VT_Redirection,FS_IN(tex),11-lambda));\n"
	"  float3 redir = float3( ((val>>13)&0x1f), ((val>>18)&0x1f), ((val>>23)&0xff) -113.0f );\n"
	"  float2 cache_coord = frac( FS_IN(tex).xy * exp2(redir.z) ) *CacheInnerPageSize+redir.xy*CacheOuterPageSize+CacheBorderSize;\n"
	"  float scale = exp2(redir.z)*CacheInnerPageSize;\n"
	"  float4 diffuse_color = saturate(H_SAMPLE_2D_GRAD(VT_Cache,cache_coord,dx*scale,dy*scale) );\n"
	"  FS_OUT_COLOR = diffuse_color;\n"//saturate(float4(FS_OUT_COLOR.xyz*FS_IN(color).xyz*diffuse_color.xyz,1.0f));\n"
	"}\n";
	
	struct light_constants
	{
		Vector4<>	LightPositionArg1[3];
		Vector4<>	LightColorArg2[3];
	};

	struct shadowcube_constants
	{
		Matrix4<>	CubeMatrix[6];
		Vector4<>	CubeCenter;
	};

	static const char* shadowmap_program =
	"H_CONSTANT_BUFFER_BEGIN(TransformGlobal)\n"
	"    float4x4 model;\n"
	"    float4x4 inverse_modelview;\n"
	"    float4x4 modelview;\n"
	"    float4x4 modelview_projection;\n"
	"    float ambient_factor;\n"
	"    float diffuse_factor;\n"
	"H_CONSTANT_BUFFER_END\n"
	"\n"
	"H_CONSTANT_BUFFER_BEGIN(GeometryCubeGlobal)\n"
	"    float4x4 cube_side[6];\n"
	"	 float4   cube_center;\n"
	"H_CONSTANT_BUFFER_END\n"
	"\n"
	"// cbuffer Transform : register(b0)\n"
	"#define Modelview               modelview\n"
	"#define InverseModelview        inverse_modelview\n"
	"#define ModelviewProjection     modelview_projection\n"
	"#define CubeSide				 cube_side\n"
	"#define CubeCenter				 cube_center\n"
	"\n"
	"VS_IN_BEGIN\n"
	"  DECL_VS_IN(float3,position);\n"
	"VS_IN_END\n"
	"VS_OUT_BEGIN\n"
	"  DECL_VS_POSITION;\n"
	"VS_OUT_END\n"
	"\n"
	"VERTEX_SHADER\n"
	"{	\n"
	"  VS_OUT_POSITION = mul( Modelview, float4(VS_IN(position), 1.0f) );\n "
	"}"
	"\n"
	"GS_IN_BEGIN\n"
	"  DECL_GS_IN_POSITION;\n"
	"GS_IN_END\n"
	"GS_OUT_BEGIN\n"
	"  DECL_GS_OUT(float4,vertex_color);\n"
	"  DECL_GS_OUT_POSITION;\n"
	"  DECL_GS_OUT_LAYER;\n"
	"GS_OUT_END\n"
	"\n"
	"void OutputTriangleToStream(int iStream,float4 input_triangle[3],GS_STREAM_OUT(TRIANGLE_OUT))\n"
	"{\n"
	"	GS_INIT_OUT;\n"
	"	for( int v = 0; v < 3; v++ )\n"
	"	{\n"
	"		GS_OUT_LAYER = iStream;\n"
	"		GS_OUT_POSITION =  mul( CubeSide[iStream], input_triangle[v] );\n"
	"		GS_OUT(vertex_color) = float4(GS_OUT_POSITION.w,GS_OUT_POSITION.w,GS_OUT_POSITION.w,1.0f);\n"
	"		GS_END_VERTEX;\n"
	"	}\n"
	"	GS_END_PRIMITIVE;\n"
	"}\n"
	"int GetCubeSide(float3 position)\n"
	"{\n"
	"	float3 abs_position = abs(position);\n"
	"	if(abs_position.x > abs_position.y && abs_position.x > abs_position.z) return (position.x>0.0f)?int(0):int(1);\n"
	"	else if(abs_position.y > abs_position.z) return (position.y>0.0f)?int(2):int(3);\n"
	"	else return (position.z>0.0f)?int(4):int(5);\n"
	"}\n"
	"\n"
	"\n"
	"GEOMETRY_SHADER(TRIANGLE_IN,TRIANGLE_OUT,18)\n"
	"{\n"
	"	float4 position[3];\n"
	"	position[0] = GS_IN_POSITION(0);\n"
	"	position[1] = GS_IN_POSITION(1);\n"
	"	position[2] = GS_IN_POSITION(2);\n"
	"	int Side1 = GetCubeSide(position[0].xyz-CubeCenter.xyz);\n"
	"	int Side2 = GetCubeSide(position[1].xyz-CubeCenter.xyz);\n"
	"	int Side3 = GetCubeSide(position[2].xyz-CubeCenter.xyz);\n"
	"	if( Side1 == Side2 && Side2 == Side3 )\n"
	"		OutputTriangleToStream(Side1,position,GS_PASS_STREAM_OUT);\n"
	"	else {\n"
	"		OutputTriangleToStream(0,position,GS_PASS_STREAM_OUT);OutputTriangleToStream(3,position,GS_PASS_STREAM_OUT);\n"
	"		OutputTriangleToStream(1,position,GS_PASS_STREAM_OUT);OutputTriangleToStream(4,position,GS_PASS_STREAM_OUT);\n"
	"		OutputTriangleToStream(2,position,GS_PASS_STREAM_OUT);OutputTriangleToStream(5,position,GS_PASS_STREAM_OUT);\n"
	"	}\n"
	"}\n\n"
	"\n"
	"FS_IN_BEGIN\n"
	"  DECL_FS_IN(float4,vertex_color);\n"
	"FS_IN_END\n"
	"FS_OUT_BEGIN\n"
	"  DECL_FS_COLOR;\n"
	"FS_OUT_END\n"
	"\n"
	"FRAGMENT_SHADER\n"
	"{\n"
	"  FS_OUT_COLOR = FS_IN(vertex_color);\n"
	"}\n";

	bool RenderingDomain::MessageProc(const Message* m)
	{
		if(IsMessageType(m->GetMessageCode(),MESSAGE_UI_UNKNOWN) || IsMessageType(m->GetMessageCode(),MESSAGE_INPUT_UNKNOWN))
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
			case MESSAGE_INPUT_KEYDOWN:
				{
					MessageInputKeydown* pMessage=(MessageInputKeydown*)m;
					switch(pMessage->GetKey())
					{
					case KEY_CODE_O:
						bShowOrbit = !bShowOrbit;
						return true;
					case KEY_CODE_F:
						_pPostprocessFilter->NextFilter();
						return true;
					case KEY_CODE_T:
						bToonShading = !bToonShading;
						return true;
					case KEY_CODE_HYPHEN:
						_pPostprocessFilter->SetPasses(_pPostprocessFilter->GetPasses()+1);
						return true;
					case KEY_CODE_MINUS:
						_pPostprocessFilter->SetPasses(_pPostprocessFilter->GetPasses()-1);
						return true;
					default:
						return false;
					}
				}
				break;
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

	void RenderingDomain::DomainStep(t64 time)
	{
		
		pWrapper->BeginFrame();
		
		static u32 lastFeedbackIndex = 0xffffffff;

		u32 currentIndex = _VT->GetId();

		if(currentIndex != lastFeedbackIndex)
		{
			lastFeedbackIndex = currentIndex;
			APIWEffect* pEffect = _VT->BeginFeedback(pWrapper);

			
			position_constants c;
			c.model	=					Matrix4<>::One();
			c.modelview =				GetViewMatrix().Transpose();
			c.inverse_modelview =		GetInvViewMatrix().Transpose();
			c.modelview_projection =	(pWrapper->GenerateRenderTargetProjection(0.1f,200000.0f,1.3f,1.3f)*c.modelview.Transpose()).Transpose();
			
			pEffect->SetConstant("TransformGlobal",_pConstants);

			EffectContainer effect(pWrapper,pEffect);
			auto result = GetFactory().ForEach<int,RenderingActor>( [&](RenderingActor* o) -> int {return o->Draw(&effect,c,_pConstants,true,false);} , guid_of<RenderingActor>::Get() ,true);

			 _VT->EndFeedback(pWrapper);
		}


		int max_lights = 0;
		
		auto result3 = GetFactory().ForEach<GLightOut,RenderingLight>( [this](RenderingLight* o) -> GLightOut {return o->Step(this);} , guid_of<RenderingLight>::Get() ,true);
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

			pWrapper->SetRenderTarget(RENDER_TARGET_NONE,_lightCubeDepth[i]);

			
			position_constants pc;
			pc.model	=				Matrix4<>::One();
			pc.modelview =				Matrix4<>::One();
			pc.inverse_modelview =		Matrix4<>::One();
			pc.modelview_projection =	pc.modelview;
			pc.ambientFactor = 0.0f;
			pc.diffuseFactor = 1.0f;

			shadowcube_constants sc;
			
			Vector3<> vEyePt = Vector3<>(0.0f,0.0f,0.0f);
			Vector3<> vLookDir;
			Vector3<> vUpDir;

			float near = 0.10f;
			float far = _light[i].Range;

			Matrix4<> projection = pWrapper->GenerateRenderTargetProjection(near,far,3.1415926536/2.0f,3.1415926536/2.0f);
			Matrix4<> light_position = Matrix4<>::Translate(-_light[i].Position);

			vLookDir = Vector3<>( 1.0f, 0.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[0] = (projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position)).Transpose();
			vLookDir = Vector3<>( -1.0f, 0.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[1] = (projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position)).Transpose();
			vLookDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 0.0f, -1.0f );
			sc.CubeMatrix[2] = (projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position)).Transpose();
			vLookDir = Vector3<>( 0.0f, -1.0f, 0.0f );
			vUpDir = Vector3<>( 0.0f, 0.0f, 1.0f );
			sc.CubeMatrix[3] = (projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position)).Transpose();
			vLookDir = Vector3<>( 0.0f, 0.0f, 1.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[4] = (projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position)).Transpose();
			vLookDir = Vector3<>( 0.0f, 0.0f, -1.0f );
			vUpDir = Vector3<>( 0.0f, 1.0f, 0.0f );
			sc.CubeMatrix[5] = (projection*(Matrix4<>::LookAt(vEyePt,vLookDir,vUpDir)*light_position)).Transpose();

			sc.CubeCenter = Vector4<>(_light[i].Position,1.0f);

			_pShadowcubeConstants->UpdateContent(&sc);
			
			auto result = GetFactory().ForEach<int,RenderingActor>( [&](RenderingActor* o) -> int {return o->Draw(_pShadowmapEffect,pc,_pConstants,true,false);} , guid_of<RenderingActor>::Get() ,true);
			auto result2 = GetFactory().ForEach<int,RenderingSheet>( [&](RenderingSheet* o) -> int {return o->Draw(_pShadowmapEffect,pc,_pConstants);} , guid_of<RenderingSheet>::Get() ,true);

			lc.LightPositionArg1[i] = Vector4<>(_light[i].Position,((far+near)/(far-near)));
			lc.LightColorArg2[i] = Vector4<>(_light[i].Color,((-2.0f*far*near)/(far-near)));
		}
		for(;i<max_lights;++i)
		{
			lc.LightPositionArg1[i] = Vector4<>(0.0f,0.0f,0.0f,0.0f);
			lc.LightColorArg2[i] = Vector4<>(0.0f,0.0f,0.0f,0.0f);
		}
		
		pWrapper->SetRenderTarget(RENDER_TARGET_DEFAULT,RENDER_TARGET_DEFAULT);
		
		_pPostprocessFilter->BeginSceneRendering();
		
		_projectionMatrix = pWrapper->GenerateProjectionMatrix(0.1f,200000.0f,1.3f,3.0f/4.0f*1.3f);

		position_constants c;
		c.model	=					Matrix4<>::One();
		c.modelview =				GetViewMatrix().Transpose();
		c.inverse_modelview =		GetViewMatrix().Invert();
		c.modelview_projection =	(GetProjectionMatrix()*c.modelview.Transpose()).Transpose();
		c.ambientFactor = 0.0f;
		c.diffuseFactor = 1.0f;
	/*
		c.model	=					Matrix4<>::One();
		c.modelview =				Matrix4<>::One();
		c.inverse_modelview =		Matrix4<>::One();
		c.modelview_projection =	c.modelview;*/
		
		_pLightConstants->UpdateContent(&lc);
		
		_pVTEffect->SetTexture("VT_Cache",_VT->GetCurrentVTCache());
		_pVTEffect->SetTexture("VT_Redirection",_VT->GetCurrentVTRedirection());
		if(_VT->GetSettings())
			_pVTEffect->SetConstant("VTSettings",_VT->GetSettings());

		if(bToonShading)
		{
			auto result = GetFactory().ForEach<int,RenderingActor>( [&](RenderingActor* o) -> int {return o->Draw(_pCelEffect,c,_pConstants,false,bShowOrbit);} , guid_of<RenderingActor>::Get() ,true);
			auto result2 = GetFactory().ForEach<int,RenderingSheet>( [&](RenderingSheet* o) -> int {return o->Draw(_pCelEffect,c,_pConstants);} , guid_of<RenderingSheet>::Get() ,true);
			auto result3 = GetFactory().ForEach<int,RenderingLight>( [&](RenderingLight* o) -> int {return o->Draw(_pCelEffect,c,_pConstants,false,bShowOrbit);} , guid_of<RenderingLight>::Get() ,true);
		}
		else
		{
			auto result = GetFactory().ForEach<int,RenderingActor>( [&](RenderingActor* o) -> int {return o->Draw(_pVTEffect,c,_pConstants,false,bShowOrbit);} , guid_of<RenderingActor>::Get() ,true);
			auto result2 = GetFactory().ForEach<int,RenderingSheet>( [&](RenderingSheet* o) -> int {return o->Draw(_pVTEffect,c,_pConstants);} , guid_of<RenderingSheet>::Get() ,true);
			auto result3 = GetFactory().ForEach<int,RenderingLight>( [&](RenderingLight* o) -> int {return o->Draw(_pVTEffect,c,_pConstants,false,bShowOrbit);} , guid_of<RenderingLight>::Get() ,true);
		}

		_pPostprocessFilter->EndSceneRendering();
		
		pInterface->Draw();

		pWrapper->PresentFrame();
	}

	RenderingDomain::RenderingDomain() :
		fCameraX(settings->getf32Setting("cam_x")),
		fCameraY(settings->getf32Setting("cam_y")),
		fCameraZ(settings->getf32Setting("cam_z")),
		bShowOrbit(true),bToonShading(false)
	{
		GetFactory().RegisterObjectType<RenderingActor>();
		GetFactory().RegisterObjectType<RenderingSheet>();
		GetFactory().RegisterObjectType<RenderingLight>();

		std::string api = settings->getstringSetting("rendering_api");
		APIWRendererType type;
		if(api == "direct3d")
			type = APIW_D3DWRAPPER;
		else if(api == "opengl")
			type = APIW_OGLWRAPPER;
		else
			type = APIW_DEFAULT;

		APIWDisplaySettings d_settings;
		d_settings.xRes = settings->getu32Setting("x_res");
		d_settings.yRes = settings->getu32Setting("y_res");
		d_settings.bFullscreen = settings->getBoolSetting("fullscreen");

		pWrapper = RenderingAPIWrapper::CreateRenderingWrapper(type,&d_settings);

		pWrapper->BeginAllocation();

		// Create the vertex buffer

		pInterface = new UserInterfaceRendering(pWrapper);

		// Create the Light Depth Cube
		
		//_light1Cube = pWrapper->CreateTexture(512,512,1,R32_FLOAT,TEXTURE_CUBE | TEXTURE_GPU_WRITE,nullptr);
		_lightCubeDepth[0] = pWrapper->CreateTexture(1024,1024,1,R16_UNORM,TEXTURE_CUBE | TEXTURE_GPU_DEPTH_STENCIL,nullptr);
		_lightCubeDepth[1] = pWrapper->CreateTexture(1024,1024,1,R16_UNORM,TEXTURE_CUBE | TEXTURE_GPU_DEPTH_STENCIL,nullptr);
		_lightCubeDepth[2] = pWrapper->CreateTexture(1024,1024,1,R16_UNORM,TEXTURE_CUBE | TEXTURE_GPU_DEPTH_STENCIL,nullptr);

		APIWSampler Samplers[5];

		strcpy(Samplers[0].SamplerName,"ShadowCube1");
		strcpy(Samplers[1].SamplerName,"ShadowCube2");
		strcpy(Samplers[2].SamplerName,"ShadowCube3");
		strcpy(Samplers[3].SamplerName,"VT_Cache");
		strcpy(Samplers[4].SamplerName,"VT_Redirection");

		Samplers[0].State = DefaultSamplerState;
		Samplers[0].State.ComparisonFunction = COMPARISON_LESS_EQUAL;
		Samplers[0].State.FilterFlags |= FILTER_COMPARISON;

		Samplers[2].State = (Samplers[1].State = Samplers[0].State);

		APIWRasterizerState wireframe = DefaultRasterizerState;
		wireframe.bWireframe = false;
		wireframe.CullMode = CULL_CCW; 

		_pEffect = new EffectContainer(pWrapper,default_program,&wireframe,&DefaultBlendState,1,false,Samplers,3);

		Samplers[3].State = DefaultSamplerState;
		Samplers[3].State.FilterFlags = FILTER_ANISOTROPIC;
		Samplers[3].State.MaxAnisotropy = 8.0f;
		Samplers[4].State = DefaultSamplerState;
		Samplers[4].State.FilterFlags = FILTER_MIN_POINT | FILTER_MAG_POINT | FILTER_MIP_POINT;
		Samplers[4].State.MipLODBias = 0;

		_pVTEffect = new EffectContainer(pWrapper,vt_test,&wireframe,&DefaultBlendState,1,false,Samplers,5);
		_pConstants = pWrapper->CreateConstantBuffer(sizeof(position_constants));
		_pLightConstants = pWrapper->CreateConstantBuffer(sizeof(light_constants));
		_pShadowcubeConstants = pWrapper->CreateConstantBuffer(sizeof(shadowcube_constants));

		_pEffect->SetConstant("TransformGlobal",_pConstants);
		_pEffect->SetConstant("LightGlobal",_pLightConstants);
		_pEffect->SetTexture("ShadowCube1",_lightCubeDepth[0]);
		_pEffect->SetTexture("ShadowCube2",_lightCubeDepth[1]);
		_pEffect->SetTexture("ShadowCube3",_lightCubeDepth[2]);
		
		_pVTEffect->SetConstant("TransformGlobal",_pConstants);
		_pVTEffect->SetConstant("LightGlobal",_pLightConstants);
		_pVTEffect->SetTexture("ShadowCube1",_lightCubeDepth[0]);
		_pVTEffect->SetTexture("ShadowCube2",_lightCubeDepth[1]);
		_pVTEffect->SetTexture("ShadowCube3",_lightCubeDepth[2]);
		
		_pPostprocessFilter = new PostprocessFilter(pWrapper);

		APIWRasterizerState shadowmapRasterizer = DefaultRasterizerState;
		shadowmapRasterizer.CullMode = CULL_CCW;

		_pShadowmapEffect = new EffectContainer(pWrapper,shadowmap_program,&shadowmapRasterizer);
		_pShadowmapEffect->SetConstant("TransformGlobal",_pConstants);
		_pShadowmapEffect->SetConstant("GeometryCubeGlobal",_pShadowcubeConstants);

		pWrapper->EndAllocation();

		_projectionMatrix = pWrapper->GenerateProjectionMatrix(0.1f,200000.0f,1.3f,3.0f/4.0f*1.3f);
		_viewMatrix = Matrix4<>::Translate(Vector3<>(0.0,0.0,fCameraZ))*Matrix4<>::AngleRotation(Vector3<>(1.0,0.0,0.0),fCameraX)*Matrix4<>::AngleRotation(Vector3<>(0.0,1.0,0.0),fCameraY);
		_invViewMatrix = Matrix4<>::AngleRotation(Vector3<>(0.0,1.0,0.0),-fCameraY)*Matrix4<>::AngleRotation(Vector3<>(1.0,0.0,0.0),-fCameraX)*Matrix4<>::Translate(Vector3<>(0.0,0.0,-fCameraZ));

		//_Map = Resource->OpenResource<IMeshData>("@world.MPQ\\world\\maps\\Azeroth\\Azeroth_40_40.adt");
		
		_VT = GetResource().OpenResource<IVirtualTexture>("t_landscape.hsvt");

		printf("Init Rendering\n");
	}

	RenderingDomain::~RenderingDomain()
	{
		printf("Destroy Rendering\n");
		if(_pPostprocessFilter)delete _pPostprocessFilter;
		if(_pShadowmapEffect)delete _pShadowmapEffect;
		if(_pEffect)delete _pEffect;
		if(_pConstants)delete _pConstants;
		if(_pLightConstants)delete _pLightConstants;
		if(_pShadowcubeConstants)delete _pShadowcubeConstants;
		if(pInterface)delete pInterface;

		if(pWrapper)delete pWrapper;
	}
}
