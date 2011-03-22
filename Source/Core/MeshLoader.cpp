#include "HAGE.h"
#include "../../ply/rply.h"


#include "CMeshLoader.h"

namespace HAGE
{

	static const char* program =
	"H_CONSTANT_BUFFER_BEGIN(transformGlobal)\n"
	"    float4x4 inverse_modelview;\n"
	"    float4x4 modelview;\n"
	"    float4x4 modelview_projection;\n"
	"	 float4 light_position_view;\n"
	"	 float4 light_color;\n"
	"H_CONSTANT_BUFFER_END\n"
	"\n"
	"\n"
	"VS_IN_BEGIN\n"
	"  DECL_VS_IN(float3,position);\n"
	"  DECL_VS_IN(float3,normal);\n"
	"  DECL_VS_IN(float3,color);\n"
	"VS_IN_END\n"
	"VS_OUT_BEGIN\n"
	"  DECL_VS_POSITION;\n"
	"  DECL_VS_OUT(float4,color);\n"
	"VS_OUT_END\n"
	"\n"
	"\n"
	"VERTEX_SHADER\n"
	"{	\n"
	"  VS_OUT_POSITION = mul( modelview_projection, float4( VS_IN(position), 1.0f ) );\n "
	"  float3 lPosition = light_position_view.xyz;\n "
	"  float3 lDir = lPosition-VS_IN(position).xyz;\n "
	"  lDir = lDir/sqrt(lDir.x*lDir.x+lDir.y*lDir.y+lDir.z*lDir.z);\n"
	"  float light_intensity = dot(lDir,VS_IN(normal));\n"
	"  float3 this_color = light_intensity*VS_IN(color)*light_color.xyz;\n"
	"  VS_OUT(color) = float4(this_color,1.0f);\n"
	"}"
	"\n"
	"FS_IN_BEGIN\n"
	"  DECL_FS_IN(float4,color);\n"
	"FS_IN_END\n"
	"FS_OUT_BEGIN\n"
	"  DECL_FS_COLOR;\n"
	"FS_OUT_END\n"
	"\n"
	"FRAGMENT_SHADER\n"
	"{\n"
	"  FS_OUT_COLOR = FS_IN(color);\n"
	"}\n";


	struct testConstants
	{
		Matrix4<>	inverse_modelview;
		Matrix4<>	modelview;
		Matrix4<>	modelview_projection;
		Vector4<>	LightPosition;
		Vector4<>	LightColor;
	};

	static const char szDefFormat[] = "DefaultVertexFormat";
	struct DefaultVertexFormat
	{
	public:
		Vector3<>	position;
		Vector2<>	texcoord0;
		Vector3<>	normal;
		Vector3<>	color;

		static const char* name;
	};
	const char* DefaultVertexFormat::name = szDefFormat;

	VertexDescriptionEntry DefFormatDescriptor[] = {
		{"Position",	1,	R32G32B32_FLOAT},
		{"Texcoord",	1,	R32G32_FLOAT},
		{"Normal",		1,	R32G32B32_FLOAT},
		{"Color",		1,	R32G32B32_FLOAT},
	};
	
	static const float p = (1.0f + sqrtf(5.0f))/4.0f;
	static const float _1 = 0.5f;

	static const DefaultVertexFormat vertices[] =
	{
		{Vector3<>(_1,  0.0f, p),		Vector2<>(0.0f,0.0f),	Vector3<>(_1,  0.0f, p),		Vector3<>(1.0f, 0.5f, 0.5f)},
		{Vector3<>(-_1,  0.0f, p),		Vector2<>(0.0f,0.0f),	Vector3<>(-_1,  0.0f, p),		Vector3<>(0.5f, 1.0f, 0.5f)},
		{Vector3<>(_1,   0.0f, -p),		Vector2<>(0.0f,0.0f),	Vector3<>(_1,   0.0f, -p),		Vector3<>(0.5f, 0.5f, 1.0f)},
		{Vector3<>(-_1,  0.0f, -p),		Vector2<>(0.0f,0.0f),	Vector3<>(-_1,  0.0f, -p),		Vector3<>(0.0f, 0.5f, 0.5f)},
		{Vector3<>(0.0f, p,	   _1),		Vector2<>(0.0f,0.0f),	Vector3<>(0.0f, p,	   _1),		Vector3<>(0.5f, 0.0f, 0.5f)},
		{Vector3<>(0.0f, -p,   _1),		Vector2<>(0.0f,0.0f),	Vector3<>(0.0f, -p,   _1),		Vector3<>(0.5f, 0.0f, 0.5f)},
		{Vector3<>(0.0f, p,	   -_1),	Vector2<>(0.0f,0.0f),	Vector3<>(0.0f, p,	   -_1),	Vector3<>(0.5f, 0.0f, 1.0f)},
		{Vector3<>(0.0f, -p,   -_1),	Vector2<>(0.0f,0.0f),	Vector3<>(0.0f, -p,   -_1),		Vector3<>(1.0f, 0.0f, 0.5f)},
		{Vector3<>(p,	 _1,	0.0f),	Vector2<>(0.0f,0.0f),	Vector3<>(p,	 _1,	0.0f),	Vector3<>(0.0f, 1.0f, 0.5f)},
		{Vector3<>(-p,   _1,	0.0f),	Vector2<>(0.0f,0.0f),	Vector3<>(-p,   _1,	0.0f),		Vector3<>(0.0f, 0.5f, 1.0f)},
		{Vector3<>(p,	 -_1,	0.0f),	Vector2<>(0.0f,0.0f),	Vector3<>(p,	 -_1,	0.0f),	Vector3<>(1.0f, 0.5f, 0.0f)},
		{Vector3<>(-p,   -_1,	0.0f),	Vector2<>(0.0f,0.0f),	Vector3<>(-p,   -_1,	0.0f),	Vector3<>(0.5f, 1.0f, 0.0f)},
	};

	static const DefaultVertexFormat boxVertices[] =
	{
		// -x wall
		{Vector3<>(-1.0f,-1.0f,-1.0f),	Vector2<>(0.0f,0.0f),	Vector3<>(1.0f,0.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(-1.0f,1.0f,-1.0f),	Vector2<>(1.0f,0.0f),	Vector3<>(1.0f,0.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(-1.0f,1.0f,1.0f),	Vector2<>(1.0f,1.0f),	Vector3<>(1.0f,0.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(-1.0f,-1.0f,1.0f),	Vector2<>(0.0f,1.0f),	Vector3<>(1.0f,0.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		// x wall
		{Vector3<>(1.0f,-1.0f,-1.0f),	Vector2<>(0.0f,0.0f),	Vector3<>(-1.0f,0.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,1.0f,-1.0f),	Vector2<>(1.0f,0.0f),	Vector3<>(-1.0f,0.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,1.0f,1.0f),		Vector2<>(1.0f,1.0f),	Vector3<>(-1.0f,0.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,-1.0f,1.0f),	Vector2<>(0.0f,1.0f),	Vector3<>(-1.0f,0.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		// -y wall
		{Vector3<>(-1.0f,-1.0f,-1.0f),	Vector2<>(0.0f,0.0f),	Vector3<>(0.0f,1.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,-1.0f,-1.0f),	Vector2<>(1.0f,0.0f),	Vector3<>(0.0f,1.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,-1.0f,1.0f),	Vector2<>(1.0f,1.0f),	Vector3<>(0.0f,1.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(-1.0f,-1.0f,1.0f),	Vector2<>(0.0f,1.0f),	Vector3<>(0.0f,1.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		// y wall
		{Vector3<>(-1.0f,1.0f,-1.0f),	Vector2<>(0.0f,0.0f),	Vector3<>(0.0f,-1.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,1.0f,-1.0f),	Vector2<>(1.0f,0.0f),	Vector3<>(0.0f,-1.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,1.0f,1.0f),		Vector2<>(1.0f,1.0f),	Vector3<>(0.0f,-1.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(-1.0f,1.0f,1.0f),	Vector2<>(0.0f,1.0f),	Vector3<>(0.0f,-1.0f,0.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		// -z wall
		{Vector3<>(-1.0f,1.0f,-1.0f),	Vector2<>(0.0f,0.0f),	Vector3<>(0.0f,0.0f,1.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,1.0f,-1.0f),	Vector2<>(1.0f,0.0f),	Vector3<>(0.0f,0.0f,1.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,-1.0f,-1.0f),	Vector2<>(1.0f,1.0f),	Vector3<>(0.0f,0.0f,1.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(-1.0f,-1.0f,-1.0f),	Vector2<>(0.0f,1.0f),	Vector3<>(0.0f,0.0f,1.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		// z wall
		{Vector3<>(-1.0f,1.0f,1.0f),	Vector2<>(0.0f,0.0f),	Vector3<>(0.0f,0.0f,-1.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,1.0f,1.0f),		Vector2<>(1.0f,0.0f),	Vector3<>(0.0f,0.0f,-1.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(1.0f,-1.0f,1.0f),	Vector2<>(1.0f,1.0f),	Vector3<>(0.0f,0.0f,-1.0f),		Vector3<>(1.0f,1.0f,1.0f)},
		{Vector3<>(-1.0f,-1.0f,1.0f),	Vector2<>(0.0f,1.0f),	Vector3<>(0.0f,0.0f,-1.0f),		Vector3<>(1.0f,1.0f,1.0f)},
	};

	static const u32	boxIndices[] = 
	{	
		0x0,0x1,0x2,0x0,0x2,0x3,
		0x6,0x5,0x4,0x7,0x6,0x4,
		0xa,0x9,0x8,0xb,0xa,0x8,
		0xc,0xd,0xe,0xc,0xe,0xf,
		0x12,0x11,0x10,0x13,0x12,0x10,
		0x14,0x15,0x16,0x14,0x16,0x17
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

IResourceLoader* CDrawableMeshLoader::Initialize(IDataStream* pStream,const IResource* pPrev)
{
	return new CDrawableMeshLoader(pStream,pPrev);
}

CDrawableMeshLoader::CDrawableMeshLoader(IDataStream* pStream,const IResource* pPrev) :_pDrawableMesh(nullptr)
{
	_pDependancies = new std::pair<std::string,guid>[1];
	_pDependancies[0] = std::pair<std::string,guid>(pStream->GetIdentifierString(),guid_of<IMeshData>::Get());
}
CDrawableMeshLoader::~CDrawableMeshLoader()
{
	if(_pDependancies)
		delete [] _pDependancies;
}

IResource* CDrawableMeshLoader::Finalize(const IResource** dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
{
	if(_pDrawableMesh == nullptr&&nDependanciesInOut == 0)
	{
		*pDependanciesOut = _pDependancies;
		nDependanciesInOut = 1;
		return nullptr;
	}
	else if(_pDrawableMesh == nullptr)
	{
		assert(nDependanciesInOut == 1);
		_pDrawableMesh = new CDrawableMesh((const IMeshData*)dependanciesIn[0]);
		delete [] _pDependancies;
		_pDependancies = nullptr;
	}
	
	return _pDrawableMesh->Finalize(dependanciesIn,pDependanciesOut,nDependanciesInOut);
}

CDrawableMeshLoader::CDrawableMesh::CDrawableMesh(const IMeshData* pData) : _pVertexBuffer(nullptr),_pEffect(nullptr)
{
	assert(pData);
	RenderingAPIAllocator* pAlloc = RenderingAPIAllocator::QueryAPIAllocator();
	assert(pAlloc);
	pAlloc->BeginAllocation();


	static bool registered = false;
	if(!registered)
	{
		pAlloc->RegisterVertexFormat(szDefFormat,DefFormatDescriptor,sizeof(DefFormatDescriptor)/sizeof(VertexDescriptionEntry));
		registered =true;
	}

	u32 nVertices = pData->GetNumVertices();
	u32 nIndices = pData->GetNumIndices();

	u8* pVertexData = new u8[sizeof(DefaultVertexFormat)*nVertices];
	u8* pIndexData = new u8[sizeof(u32)*nIndices];

	const u8* pPosition,*pTexcoord,*pNormal,*pColor;
	u32 iPositionStride,iTexcoordStride,iNormalStride,iColorStride;

	pData->GetVertexData(&pPosition,iPositionStride,IMeshData::POSITION);
	pData->GetVertexData(&pTexcoord,iTexcoordStride,IMeshData::TEXCOORD0);
	pData->GetVertexData(&pNormal,iNormalStride,IMeshData::NORMAL);
	pData->GetVertexData(&pColor,iColorStride,IMeshData::COLOR);

	DefaultVertexFormat* pWriteOut = (DefaultVertexFormat*)pVertexData;
	for(int i = 0;i <nVertices;++i)
	{
		pWriteOut[i].position = *((Vector3<>*)&pPosition[iPositionStride*i]) ;
		if(pTexcoord)
			pWriteOut[i].texcoord0 = *((Vector2<>*)&pTexcoord[iTexcoordStride*i]) ;
		else
			pWriteOut[i].texcoord0 = Vector2<>(0.0f,0.0f);
		pWriteOut[i].normal = *((Vector3<>*)&pNormal[iNormalStride*i]) ;
		pWriteOut[i].color = *((Vector3<>*)&pColor[iColorStride*i]) ;
	}

	const u8* pIndexSource;
	pData->GetIndexData(&pIndexSource);
	memcpy(pIndexData,pIndexSource,sizeof(u32)*nIndices);

	_pVertexBuffer = pAlloc->CreateVertexBuffer(DefaultVertexFormat::name,pVertexData,pData->GetNumVertices());
	_pVertexArray = pAlloc->CreateVertexArray(nIndices/3,PRIMITIVE_TRIANGLELIST,&_pVertexBuffer,1,(const u32*)pIndexData);
	_pEffect = pAlloc->CreateEffect(program);
	_pConstants = pAlloc->CreateConstantBuffer(sizeof(testConstants));
	_nTextures = 0;
	_ppTextures = nullptr;

	pAlloc->EndAllocation();

	int i = 0;
	const char* name;
	while(name = pData->GetTextureName(i))
		++i;

	_nTextures = i;
	_pTextureNames = new std::pair<std::string,guid>[_nTextures];
	i = 0;
	while(name = pData->GetTextureName(i))
	{
		_pTextureNames[i] = std::pair<std::string,guid>(name,guid_of<ITextureImage>::Get());	
		++i;
	}

	
	delete [] pVertexData;
	delete [] pIndexData;
}

IResource* CDrawableMeshLoader::CDrawableMesh::Finalize(const IResource** dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
{
	if(_nTextures == 0)
		return (IResource*)this;

	if(nDependanciesInOut != _nTextures)
	{
		*pDependanciesOut = _pTextureNames;
		nDependanciesInOut = _nTextures;
		return nullptr;
	}

	assert(nDependanciesInOut == _nTextures);

	_ppTextures = new const APIWTexture*[_nTextures];
	for(int i = 0;i<_nTextures;++i)
		_ppTextures[i] = ((const ITextureImage**)dependanciesIn)[i]->GetTexture();

	delete [] _pTextureNames;
	_pTextureNames = nullptr;
	return (IResource*)this;
}

CDrawableMeshLoader::CDrawableMesh::~CDrawableMesh()
{
	if(_ppTextures)
	{
		for(int i = 0;i<_nTextures;++i)
			delete _ppTextures[i];
		delete [] _ppTextures;
	}
	if(_pVertexBuffer)delete _pVertexBuffer;
	if(_pVertexArray)delete _pVertexArray;
	if(_pEffect)delete _pEffect;
	if(_pConstants)delete _pConstants;
}

void CDrawableMeshLoader::CDrawableMesh::Draw(const Vector3<>& position, const Matrix4<>& view,const Matrix4<>& inv_view, const Matrix4<>& proj,const Vector3<>& lPosition, const Vector3<>& lColor) const
{
	Matrix4<> Model = Matrix4<>::Translate(position);
	testConstants c;
	c.modelview =				view*Matrix4<>::Translate(position);
	c.inverse_modelview =		Matrix4<>::Translate(-position)*inv_view;
	c.modelview_projection =	proj*c.modelview;
	c.LightPosition = c.inverse_modelview*Vector4<>(lPosition,1.0f);
	c.LightColor = Vector4<>(lColor,1.0f);
	_pConstants->UpdateContent(&c);
	_pEffect->SetConstant("transformGlobal",_pConstants);
	_pEffect->Draw(_pVertexArray);
}


IResourceLoader* CMeshDataLoader::Initialize(IDataStream* pStream,const IResource* pPrev)
{
	CMeshDataLoader* pMeshLoader = new CMeshDataLoader(pStream,pPrev);
	if(!pMeshLoader->IsValid())
	{
		delete pMeshLoader;
		pMeshLoader = nullptr;
	}
	return pMeshLoader;
}

CMeshDataLoader::CMeshDataLoader(IDataStream* pStream,const IResource* pPrev)
{
	_pMeshData = new CMeshData(pStream);
	if(!_pMeshData->IsValid())
	{
		delete _pMeshData;
		_pMeshData = nullptr;
	}
}
CMeshDataLoader::~CMeshDataLoader()
{
}

IResource* CMeshDataLoader::Finalize(const IResource** dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
{
	return _pMeshData->Finalize(dependanciesIn,pDependanciesOut,nDependanciesInOut);
}

static int vertex_cb(p_ply_argument argument) {
    long eol;
	std::vector<float>* pV;
    ply_get_argument_user_data(argument, (void**)&pV, &eol);
	pV->push_back((float)ply_get_argument_value(argument));
    return 1;
}

static int face_cb(p_ply_argument argument) {
    long length, value_index;
	static int indices[3];
	std::vector<u32>* pV;
    ply_get_argument_user_data(argument, (void**)&pV, NULL);
    ply_get_argument_property(argument, NULL, &length, &value_index);
	switch (value_index) {
        case 0:
        case 1: 
			indices[value_index] = ply_get_argument_value(argument);
            break;
        case 2:
			indices[value_index] = ply_get_argument_value(argument);
			pV->push_back((u32)indices[0]);
			pV->push_back((u32)indices[1]);
			pV->push_back((u32)indices[2]);
            break;
        default: 
            break;
    }
	
    return 1;
}

CMeshDataLoader::CMeshData::CMeshData(IDataStream* pData) : _MD2(false),_bValid(false)
{
	_pData = pData;
	if(pData->IsValid())
	{
		p_ply ply = ply_open(pData->GetIdentifierString().c_str(), NULL);
		if(ply && ply_read_header(ply))
		{
			std::vector<float> vX,vY,vZ;
			std::vector<u32> tI;
			ply_set_read_cb(ply, "vertex", "x", vertex_cb, &vX, 0);
			ply_set_read_cb(ply, "vertex", "y", vertex_cb, &vY, 0);
			ply_set_read_cb(ply, "vertex", "z", vertex_cb, &vZ, 1);
			ply_set_read_cb(ply, "face", "vertex_indices", face_cb, &tI, 0);
			assert(ply_read(ply));
			ply_close(ply);
		
			_nIndices = tI.size();
			_nVertices = vX.size();
			DefaultVertexFormat* pVertexData = new DefaultVertexFormat[_nVertices];
			Vector3<> min(0.0f,0.0f,0.0f),max(0.0f,0.0f,0.0f);
			for(int i =0;i<_nVertices;++i)
			{
				pVertexData[i].position = Vector3<>(vX[i],vY[i],vZ[i]);
				pVertexData[i].normal   = Vector3<>(0,0,0);
				pVertexData[i].texcoord0 = Vector2<>(0.0f,0.0f);
				if(pVertexData[i].position.x <= min.x)
					min.x = pVertexData[i].position.x;
				if(pVertexData[i].position.y <= min.y)
					min.y = pVertexData[i].position.y;
				if(pVertexData[i].position.z <= min.z)
					min.z = pVertexData[i].position.z;
				if(pVertexData[i].position.x >= max.x)
					max.x = pVertexData[i].position.x;
				if(pVertexData[i].position.y >= max.y)
					max.y = pVertexData[i].position.y;
				if(pVertexData[i].position.z >= max.z)
					max.z = pVertexData[i].position.z;
			}

			_pVertexData=(u8*)pVertexData;
			u32* pIndexData = new u32[_nIndices];
			memcpy(pIndexData,&tI[0],sizeof(u32)*_nIndices);

			for(int i=0;i<_nIndices/3;++i)
			{
				Vector3<> d1	= pVertexData[pIndexData[i*3+1]].position-pVertexData[pIndexData[i*3+0]].position;
				Vector3<> d2	= pVertexData[pIndexData[i*3+2]].position-pVertexData[pIndexData[i*3+0]].position;
				Vector3<> face_normal = d1 % d2;
				face_normal = face_normal / sqrtf(!face_normal);
				pVertexData[pIndexData[i*3+0]].normal+=face_normal;
				pVertexData[pIndexData[i*3+1]].normal+=face_normal;
				pVertexData[pIndexData[i*3+2]].normal+=face_normal;
			}

			Vector3<> avg = (max+min)/2.0f;
			for(int i =0;i<_nVertices;++i)
			{
				pVertexData[i].position = (pVertexData[i].position-avg) | (max-avg);
				pVertexData[i].color = Vector3<>(1.0f,1.0f,1.0f);
				pVertexData[i].normal = pVertexData[i].normal / sqrtf(!pVertexData[i].normal);
			}
			_pIndexData=(u8*)pIndexData;
			_bValid = true;
			return;
		}
		
		pData->Seek(0,IDataStream::ORIGIN_BEGINNING);
		if(TryLoadM2(pData))
		{
			_MD2 = true;
			_bValid = true;
			return;
		}
	}
	
	if(pData->GetIdentifierString() == std::string("Box"))
	{
		_vertexSize = sizeof(DefaultVertexFormat);
		_pVertexData = (u8*)boxVertices; 
		_pIndexData = (u8*)boxIndices;
		_nVertices = 4*6;
		_nIndices = 6*6;	
		_bValid = true;
	}
	else if(pData->GetIdentifierString() == std::string("Null"))
	{
		_vertexSize = sizeof(DefaultVertexFormat);
		_pVertexData = (u8*)vertices; 
		_pIndexData = (u8*)indices;
		_nVertices = 12;
		_nIndices = 60;	
		_bValid = true;
	}
	else
	{
		_vertexSize = 0;
		_pVertexData = nullptr; 
		_pIndexData = nullptr;
		_nVertices = 0;
		_nIndices = 0;	
		_bValid = false;
	}
}

struct Sphere
{
        /*0x00*/ Vector3<> min;
        /*0x0C*/ Vector3<> max;
        /*0x18*/ float radius;
};

struct ModelHeader {
        u8 id[4];
        u8 version[4];
        u32 nameLength;
        u32 nameOfs;
        u32 GlobalModelFlags; // 1: tilt x, 2: tilt y, 4:, 8: add another field in header, 16: ; (no other flags as of 3.1.1);

        u32 nGlobalSequences; // AnimationRelated
        u32 ofsGlobalSequences; // A list of timestamps.
        u32 nAnimations; // AnimationRelated
        u32 ofsAnimations; // Information about the animations in the model.
        u32 nAnimationLookup; // AnimationRelated
        u32 ofsAnimationLookup; // Mapping of global IDs to the entries in the Animation sequences block.
        //u32 nD;
        //u32 ofsD;
        u32 nBones; // BonesAndLookups
        u32 ofsBones; // Information about the bones in this model.
        u32 nKeyBoneLookup; // BonesAndLookups
        u32 ofsKeyBoneLookup; // Lookup table for key skeletal bones.

        u32 nVertices; // GeometryAndRendering
        u32 ofsVertices; // Vertices of the model.
        u32 nViews; // GeometryAndRendering
        //u32 ofsViews; // Views (LOD) are now in .skins.

        u32 nColors; // ColorsAndTransparency
        u32 ofsColors; // Color definitions.

        u32 nTextures; // TextureAndTheifAnimation
        u32 ofsTextures; // Textures of this model.

        u32 nTransparency; // H,  ColorsAndTransparency
        u32 ofsTransparency; // Transparency of textures.
        //u32 nI;   // always unused ?
        //u32 ofsI;
        u32 nTexAnims;       // J, TextureAndTheifAnimation
        u32 ofsTexAnims;
        u32 nTexReplace; // TextureAndTheifAnimation
        u32 ofsTexReplace; // Replaceable Textures.

        u32 nTexFlags; // Render Flags
        u32 ofsTexFlags; // Blending modes / render flags.
        u32 nBoneLookup; // BonesAndLookups
        u32 ofsBoneLookup; // A bone lookup table.

        u32 nTexLookup; // TextureAndTheifAnimation
        u32 ofsTexLookup; // The same for textures.

        u32 nTexUnitLookup;          // L, TextureAndTheifAnimation, seems gone after Cataclysm
        u32 ofsTexUnitLookup; // And texture units. Somewhere they have to be too.
        u32 nTransparencyLookup; // M, ColorsAndTransparency
        u32 ofsTransparencyLookup; // Everything needs its lookup. Here are the transparencies.
        u32 nTexAnimLookup; // TextureAndTheifAnimation
        u32 ofsTexAnimLookup; // Wait. Do we have animated Textures? Wasn't ofsTexAnims deleted? oO

        Sphere collisionSphere;
        Sphere boundSphere;

        u32 nBoundingTriangles; // Miscellaneous
        u32 ofsBoundingTriangles;
        u32 nBoundingVertices; // Miscellaneous
        u32 ofsBoundingVertices;
        u32 nBoundingNormals; // Miscellaneous
        u32 ofsBoundingNormals;

        u32 nAttachments; // O, Miscellaneous
        u32 ofsAttachments; // Attachments are for weapons etc.
        u32 nAttachLookup; // P, Miscellaneous
        u32 ofsAttachLookup; // Of course with a lookup.
        u32 nEvents; //
        u32 ofsEvents; // Used for playing sounds when dying and a lot else.
        u32 nLights; // R
        u32 ofsLights; // Lights are mainly used in loginscreens but in wands and some doodads too.
        u32 nCameras; // S, Miscellaneous
        u32 ofsCameras; // The cameras are present in most models for having a model in the Character-Tab.
        u32 nCameraLookup; // Miscellaneous
        u32 ofsCameraLookup; // And lookup-time again, unit16
        u32 nRibbonEmitters; // U, Effects
        u32 ofsRibbonEmitters; // Things swirling around. See the CoT-entrance for light-trails.
        u32 nParticleEmitters; // V, Effects
        u32 ofsParticleEmitters; // Spells and weapons, doodads and loginscreens use them. Blood dripping of a blade? Particles.
};

struct ModelVertex {
        Vector3<f32> pos;
        u8 weights[4];
        u8 bones[4];
        Vector3<f32> normal;
        Vector2<f32> texcoords;
        i32 unk1, unk2; // always 0,0 so this is probably unused
};

struct ModelView {
    u8 id[4];                              // Signature
    u32 nIndex;
    u32 ofsIndex; // int16, Vertices in this model (index into vertices[])
    u32 nTris;
    u32 ofsTris;  // int16[3], indices
    u32 nProps;
    u32 ofsProps; // int32, additional vtx properties
    u32 nSub;
    u32 ofsSub;   // ModelGeoset, materials/renderops/submeshes
    u32 nTex;
    u32 ofsTex;   // ModelTexUnit, material properties/textures
    i32 lod;                               // LOD bias?
};

struct ModelGeoset {
        u32 id;              // mesh part id?
        u16 vstart;  // first vertex, Starting vertex number.
        u16 vcount;  // num vertices, Number of vertices.
        u16 istart;  // first index, Starting triangle index (that's 3* the number of triangles drawn so far).
        u16 icount;  // num indices, Number of triangle indices.
        u16 nSkinnedBones;   // number of bone indices, Number of elements in the bone lookup table.
        u16 StartBones;              // ? always 1 to 4, Starting index in the bone lookup table.
        u16 rootBone;                // root bone?
        u16 nBones;          //
        Vector3<f32> BoundingBox[2];
        f32 radius;
};

struct ModelTextureDef {
        u32 type;
        u32 flags;
        u32 nameLen;
        u32 nameOfs;
};

struct ModelTexUnit{
        // probably the texture units
        // size always >=number of materials it seems
        u16 flags;           // Usually 16 for static textures, and 0 for animated textures.
        u16 shading;         // If set to 0x8000: shaders. Used in skyboxes to ditch the need for depth buffering. See below.
        u16 op;                      // Material this texture is part of (index into mat)
        u16 op2;                     // Always same as above?
        i16 colorIndex;       // A Color out of the Colors-Block or -1 if none.
        u16 flagsIndex;      // RenderFlags (index into render flags, TexFlags)
        u16 texunit;         // Index into the texture unit lookup table.
        u16 mode;            // See below.
        u16 textureid;       // Index into Texture lookup table
        u16 texunit2;        // copy of texture unit value?
        u16 transid;         // Index into transparency lookup table.
        u16 texanimid;       // Index into uvanimation lookup table.
};

inline static void forceRead(IDataStream* pData,u64 size,void* pOut)
{
	assert(pData->Read(size,(u8*)pOut) == size);
}

inline static Vector3<> fixCoordSystem(Vector3<> v)
{
        return Vector3<>(v.x, v.z, -v.y);
}

bool CMeshDataLoader::CMeshData::TryLoadM2(IDataStream* pData)
{
	ModelHeader header;
	if(pData->Read(sizeof(header),(u8*)&header)!=sizeof(header))
		return false;
	static const char md2Magic[4] = {'M','D','2','0'};
	if(memcmp(md2Magic,header.id,4) !=0)
		return false;
	ModelVertex* origVertices = new ModelVertex[header.ofsVertices];
	pData->Seek(header.ofsVertices,IDataStream::ORIGIN_BEGINNING);
	forceRead(pData,sizeof(ModelVertex)*header.nVertices,origVertices);
	
	_vertexSize = sizeof(DefaultVertexFormat);
	_nVertices = header.nVertices;

	DefaultVertexFormat* pVertices = new DefaultVertexFormat[header.nVertices];

	for(int i = 0;i<_nVertices;++i)
	{
		pVertices[i].color = Vector3<>(1.0f,1.0f,1.0f);
		pVertices[i].normal = fixCoordSystem(origVertices[i].normal.normalize());
		pVertices[i].position = fixCoordSystem(origVertices[i].pos);
		pVertices[i].texcoord0 = origVertices[i].texcoords;
	}

	_pVertexData = (u8*)pVertices; 

	delete [] origVertices;

	//add dependancy to skin file for index data

	std::string skin_filename = pData->GetIdentifierString();

	u32 last = skin_filename.find_last_of('.');
	skin_filename = skin_filename.substr(0,last);
	skin_filename.append("00.skin");

	_depencancies.push_back(std::pair<std::string,guid>(skin_filename,guid_of<IRawData>::Get()));

	pData->Seek(header.ofsTextures,IDataStream::ORIGIN_BEGINNING);
	ModelTextureDef def[32];
	assert(header.nTextures < 32);
	forceRead(pData,header.nTextures*sizeof(ModelTextureDef),def);
	char tmp[1025];
	for(int i = 0;i<header.nTextures;++i)
		if(def[i].type == 0 && def[i].nameLen > 1)
		{
			pData->Seek(def[i].nameOfs,IDataStream::ORIGIN_BEGINNING);
			forceRead(pData,def[i].nameLen,tmp);
			tmp[def[i].nameLen] = '\0';
			_textureNames.push_back(std::string("@art.mpq\\")+std::string(tmp));
		}
	return true;
}

void CMeshDataLoader::CMeshData::FinalizeLoadM2(const IResource** dependanciesIn,u32 nDependanciesIn)
{
	assert(nDependanciesIn == _depencancies.size());
	IRawData* pSkinData = (IRawData*)dependanciesIn[0];
	assert(pSkinData->GetSize() >= sizeof(ModelView));
	const u8* p;
	u32 size = pSkinData->GetData(&p);
	const ModelView* view=(const ModelView*)p;

	static const char skinMagic[4] = {'S','K','I','N'};
	assert(memcmp(skinMagic,view->id,4) ==0);

	const u16* indexLookup = (const u16*)(p + view->ofsIndex);
	const u16* triangles = (const u16*)(p + view->ofsTris);
	_nIndices = view->nTris;
	u32* indices = new u32[_nIndices];
	for(int i =0;i<_nIndices;++i)
		indices[i] = indexLookup[triangles[i]];

	_pIndexData = (u8*)indices;
}


IResource* CMeshDataLoader::CMeshData::Finalize(const IResource** dependanciesIn,const std::pair<std::string,guid>** pDependanciesOut,u32& nDependanciesInOut)
{
	if(nDependanciesInOut == 0 && _depencancies.size())
	{		
		if(pDependanciesOut)
		{
			if(_depencancies.size() > 0)
				*pDependanciesOut = _depencancies.data();
			else
				*pDependanciesOut = nullptr;
		}
		nDependanciesInOut = _depencancies.size();
		return nullptr;
	}
	else
	{
		if(_MD2)
			FinalizeLoadM2(dependanciesIn,nDependanciesInOut);

		_depencancies.clear();
		_pData = nullptr;
		return (IResource*)this;
	}
}


CMeshDataLoader::CMeshData::~CMeshData()
{
	
}
u32 CMeshDataLoader::CMeshData::GetNumVertices() const
{
	return _nVertices;
}


u32 CMeshDataLoader::CMeshData::GetVertexData(const u8** pDataOut,u32& pOutStride,VERTEX_DATA_TYPE type) const
{
	switch(type)
	{
	case POSITION:
		*pDataOut = (const u8*)&((DefaultVertexFormat*)_pVertexData)->position;
		break;
	case TEXCOORD0:
		*pDataOut = (const u8*)&((DefaultVertexFormat*)_pVertexData)->texcoord0;
		break;
	case NORMAL:
		*pDataOut = (const u8*)&((DefaultVertexFormat*)_pVertexData)->normal;
		break;
	case COLOR:
		*pDataOut = (const u8*)&((DefaultVertexFormat*)_pVertexData)->color;
		break;
	default:
		return 0;
	}

	pOutStride = sizeof(DefaultVertexFormat);
}
u32	CMeshDataLoader::CMeshData::GetNumIndices() const
{
	return _nIndices;
}
u32 CMeshDataLoader::CMeshData::GetIndexData(const u8** pDataOut) const
{
	if(pDataOut)
	{
		*pDataOut = _pIndexData;
		return _nIndices;
	}
	else
	{
		return _nIndices;
	}
}
const char* CMeshDataLoader::CMeshData::GetTextureName(u32 index) const
{
	if(index < _textureNames.size())
		return _textureNames[index].c_str();
	else
		return nullptr;
}

const void* CMeshDataLoader::CMeshData::GetExtendedData(EXTENDED_DATA_TYPE type) const
{
	return nullptr;
}

}