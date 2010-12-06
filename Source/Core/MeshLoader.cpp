#include "HAGE.h"
#include "../../ply/rply.h"

namespace HAGE
{

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
		Vector3<>	normal;
		Vector3<>	color;

		static const char* name;
	};
	const char* DefaultVertexFormat::name = szDefFormat;

	VertexDescriptionEntry DefFormatDescriptor[] = {
		{"Position",	1,	R32G32B32_FLOAT},
		{"Normal",		1,	R32G32B32_FLOAT},
		{"Color",		1,	R32G32B32_FLOAT},
	};

	static const float p = (1.0f + sqrtf(5.0f))/4.0f;
	static const float _1 = 0.5f;

	static const DefaultVertexFormat vertices[] =
	{
		{Vector3<>(_1,  0.0f, p),		Vector3<>(_1,  0.0f, p),		Vector3<>(1.0f, 0.5f, 0.5f)},
		{Vector3<>(-_1,  0.0f, p),		Vector3<>(-_1,  0.0f, p),		Vector3<>(0.5f, 1.0f, 0.5f)},
		{Vector3<>(_1,   0.0f, -p),		Vector3<>(_1,   0.0f, -p),		Vector3<>(0.5f, 0.5f, 1.0f)},
		{Vector3<>(-_1,  0.0f, -p),		Vector3<>(-_1,  0.0f, -p),		Vector3<>(0.0f, 0.5f, 0.5f)},
		{Vector3<>(0.0f, p,	   _1),		Vector3<>(0.0f, p,	   _1),		Vector3<>(0.5f, 0.0f, 0.5f)},
		{Vector3<>(0.0f, -p,   _1),		Vector3<>(0.0f, -p,   _1),		Vector3<>(0.5f, 0.0f, 0.5f)},
		{Vector3<>(0.0f, p,	   -_1),	Vector3<>(0.0f, p,	   -_1),	Vector3<>(0.5f, 0.0f, 1.0f)},
		{Vector3<>(0.0f, -p,   -_1),	Vector3<>(0.0f, -p,   -_1),		Vector3<>(1.0f, 0.0f, 0.5f)},
		{Vector3<>(p,	 _1,	0.0f),	Vector3<>(p,	 _1,	0.0f),	Vector3<>(0.0f, 1.0f, 0.5f)},
		{Vector3<>(-p,   _1,	0.0f),	Vector3<>(-p,   _1,	0.0f),		Vector3<>(0.0f, 0.5f, 1.0f)},
		{Vector3<>(p,	 -_1,	0.0f),	Vector3<>(p,	 -_1,	0.0f),	Vector3<>(1.0f, 0.5f, 0.0f)},
		{Vector3<>(-p,   -_1,	0.0f),	Vector3<>(-p,   -_1,	0.0f),	Vector3<>(0.5f, 1.0f, 0.0f)},
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

CDrawableMeshLoader::CDrawableMeshLoader(IDataStream* pStream,const IResource* pPrev)
{
	_pDependancies = new std::pair<std::string,guid>[1];
	_pDependancies[0] = std::pair<std::string,guid>(pStream->GetIdentifierString(),guid_of<IMeshData>::Get());
}
CDrawableMeshLoader::~CDrawableMeshLoader()
{
	if(_pDependancies)
		delete [] _pDependancies;
}

u32 CDrawableMeshLoader::GetDependancies(const std::pair<std::string,guid>** pDependanciesOut)
{
	*pDependanciesOut = _pDependancies;
	return 1;
}

IResource* CDrawableMeshLoader::Finalize(const IResource** dependanciesIn,u32 nDependanciesIn)
{
	assert(nDependanciesIn == 1);
	return (IResource*)new CDrawableMesh((const IMeshData*)dependanciesIn[0]);
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

	const u8* pVertexData;
	const u8* pIndexData;

	pData->GetVertexData(&pVertexData);
	pData->GetIndexData(&pIndexData);

	_pVertexBuffer = pAlloc->CreateVertexBuffer(DefaultVertexFormat::name,pVertexData,pData->GetNumVertices());
	_pVertexArray = pAlloc->CreateVertexArray(pData->GetNumIndices()/3,PRIMITIVE_TRIANGLELIST,&_pVertexBuffer,1,(const u32*)pIndexData);
	_pEffect = pAlloc->CreateEffect(vertex_program,fragment_program);
	_pConstants = pAlloc->CreateConstantBuffer(sizeof(testConstants));

	pAlloc->EndAllocation();
}

CDrawableMeshLoader::CDrawableMesh::~CDrawableMesh()
{
	if(_pVertexBuffer)delete _pVertexBuffer;
	if(_pVertexArray)delete _pVertexArray;
	if(_pEffect)delete _pEffect;
	if(_pConstants)delete _pConstants;
}

void CDrawableMeshLoader::CDrawableMesh::Draw(const Vector3<>& position, const Matrix4<>& view, const Matrix4<>& proj) const
{
	Matrix4<> Model = Matrix4<>::Translate(position);
	testConstants c;
	c.modelview =				view*Matrix4<>::Translate(position);
	c.inverse_modelview =		c.modelview;
	c.modelview_projection =	proj*c.modelview;
	_pConstants->UpdateContent(&c);
	_pEffect->Draw(_pVertexArray,&_pConstants);
}


IResourceLoader* CMeshDataLoader::Initialize(IDataStream* pStream,const IResource* pPrev)
{
	return new CMeshDataLoader(pStream,pPrev);
}

CMeshDataLoader::CMeshDataLoader(IDataStream* pStream,const IResource* pPrev)
{
	_pMeshData = new CMeshData(pStream);
	pStream->Close();
}
CMeshDataLoader::~CMeshDataLoader()
{
}

u32 CMeshDataLoader::GetDependancies(const std::pair<std::string,guid>** pDependanciesOut)
{
	return 0;
}

IResource* CMeshDataLoader::Finalize(const IResource** dependanciesIn,u32 nDependanciesIn)
{
	return (IResource*)_pMeshData;
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

CMeshDataLoader::CMeshData::CMeshData(const IDataStream* pData): _pVertexData((u8*)vertices),_pIndexData((u8*)indices)
{
	if(pData->GetIdentifierString() == std::string("Null"))
	{
		_vertexSize = sizeof(DefaultVertexFormat);
		_nVertices = 12;
		_nIndices = 60;
	}
	else
	{
		p_ply ply = ply_open(pData->GetIdentifierString().c_str(), NULL);
		assert(ply);
		assert(ply_read_header(ply));
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
	}
}

CMeshDataLoader::CMeshData::~CMeshData()
{

}
u32 CMeshDataLoader::CMeshData::GetNumVertices() const
{
	return _nVertices;
}
u32 CMeshDataLoader::CMeshData::GetVertexSize() const
{
	return _vertexSize;
}
u32 CMeshDataLoader::CMeshData::GetVertexData(const u8** pDataOut) const
{
	if(pDataOut)
	{
		*pDataOut = _pVertexData;
		return _nVertices*_vertexSize;
	}
	else
	{
		return _nVertices*_vertexSize;
	}
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
		return _nIndices*sizeof(u32);
	}
	else
	{
		return _nIndices*sizeof(u32);
	}
}

}
