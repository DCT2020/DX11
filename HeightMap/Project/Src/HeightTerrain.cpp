//
//
// Filename : HeightTerrain.cpp
//
#include "Device.h"
#include "Shader.h"
#include "HeightTerrain.h"

#include <fstream>

struct VTXTERRAIN
{
	float x, y, z;
	float r, g, b, a;
};


ID3D11Buffer* g_pVB = nullptr;
ID3D11InputLayout*  g_pInputLayout = nullptr;

HeightTerrain* g_pHTerrain = nullptr;
VTXTERRAIN* g_pVtxData = nullptr;
int* g_pIndices  = nullptr;
DWORD g_VtxCnt = 0;

UINT g_Width = 0;
UINT g_Length = 0;
UCHAR* g_pHeight = nullptr;


bool TerrainCreate(const TCHAR* rawFile)
{
	if (g_pHTerrain != nullptr)
		SAFE_DELETE(g_pHTerrain);

	g_pHTerrain = new HeightTerrain;
	g_pHTerrain->m_vPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_pHTerrain->m_vRot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_pHTerrain->m_vScale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	if (!Initialize(rawFile))
		return false;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VTXTERRAIN) * g_VtxCnt;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA rd;
	ZeroMemory(&rd, sizeof(rd));
	rd.pSysMem = g_pVtxData;

	if (FAILED(g_pDevice->CreateBuffer(&bd, &rd, &g_pVB)))
		return false;

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	
	if (FAILED(g_pDevice->CreateInputLayout(layout,
		numElements,
		g_pVSCode->GetBufferPointer(),
		g_pVSCode->GetBufferSize(),
		&g_pInputLayout)))
	{
		return false;
	}

	return true;
}

bool TerrainUpdate(float dTime)
{

	return true;
}

bool TerrainDraw(float dTime)
{

	return true;
}

void TerrainRelease()
{
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pVB);

	SAFE_DELARRY(g_pHeight);
	SAFE_DELARRY(g_pVtxData);
	SAFE_DELARRY(g_pIndices);
	SAFE_DELETE(g_pHTerrain);
}


bool Initialize(const TCHAR* rawFile)
{
	if (!LoadHeightMap(rawFile))
	{
		return false;
	}

	if (!CreateVertex())
	{
		return false;
	}

	int numIndex = CreateIndex();
	if (numIndex == 0)
	{
		ynError(NULL, L"�ε��� ���� ����");
		return false;
	}
}

bool LoadHeightMap(const TCHAR* rawFile)
{
	// ���̸� �ε�
	std::ifstream heightStream;
	heightStream.open(rawFile);
	if (heightStream.fail())
	{
		ynError(NULL, L"���� �� ������ ���� �� ����");
		return false;
	}

	// ������ ������ ȹ��
	heightStream.seekg(0, std::ios::end);
	g_VtxCnt = heightStream.tellg();
	heightStream.seekg(0, std::ios::beg);

	// �޸𸮸� �Ҵ��ϰ� �ڷḦ �о����.
	g_pHeight = new UCHAR[g_VtxCnt];
	heightStream.read((char*)g_pHeight, g_VtxCnt);
	heightStream.close();

	// ���̸� ũ�⸦ ����(���簢���̶� ����)
	g_Width = (int)sqrt((float)g_VtxCnt);
	g_Length = g_Width;

	return true;
}

bool CreateVertex()
{
	if (g_pVtxData != nullptr)
		SAFE_DELARRY(g_pVtxData);

	g_pVtxData = new VTXTERRAIN[g_VtxCnt];

	VTXTERRAIN* CurrentVtx = nullptr;
	float halfWidth = (g_Width * -1.0f) * 0.5f;
	float halfLength = (g_Length * -1.0f) * 0.5f;
	for (int z = 0; z < g_Length; ++z)
	{
		for (int x = 0; x < g_Width; ++x)
		{
			CurrentVtx = &g_pVtxData[z * g_Length + x];
			
			CurrentVtx->x = (float)x + halfWidth;
			CurrentVtx->y = (float)g_pHeight[z * g_Length + x];
			CurrentVtx->z = (float)z + halfLength;

			CurrentVtx->a = CurrentVtx->r = CurrentVtx->g = CurrentVtx->b = 1;
		}
	}

	return true;
}

int CreateIndex()
{
	int numIndices = (g_Width * 2) * (g_Length - 1) + (g_Length - 2);
	
	if (g_pIndices != nullptr)
		SAFE_DELARRY(g_pIndices);

	g_pIndices = new int[numIndices];
	
	int index = 0;
	for (int z = 0; z < g_Length; ++z)
	{
		if (z % 2 == 0)
		{
			//¦���� �϶�.(���� -> ������)
			int x;
			for (x = 0; x < g_Width; ++x)
			{
				g_pIndices[index++] = x + (z * g_Width);
				g_pIndices[index++] = x + (z * g_Width) + g_Width;
			}
			// Insert degenerate(��ȭ��) vertex if this isn't the last row
			if (z != g_Length - 2)
			{
				g_pIndices[index++] = --x + (z * g_Width);
			}
		}
		else
		{
			//Ȧ���� �϶�.(������ -> ����)
			int x;
			for (x = g_Width - 1; x <= 0; --x)
			{
				g_pIndices[index++] = x + (z*g_Width);
				g_pIndices[index++] = x + (z*g_Width) + g_Width;
			}
			// Insert degenerate(��ȭ��) vertex if this isn't the last row
			if (z != g_Length - 2)
			{
				g_pIndices[index++] = ++x + (z*g_Width);
			}
		}
	}

	return numIndices;
}

