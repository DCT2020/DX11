#pragma once

struct HeightTerrain
{
	XMFLOAT3 m_vPos;		//위치 : "월드 공간"
	XMFLOAT3 m_vRot;		//회전 
	XMFLOAT3 m_vScale;		//스케일 
	XMFLOAT4X4 m_mTM;					//월드변환행렬.
};

bool TerrainCreate(const TCHAR* rawFile);
bool TerrainUpdate(float dTime);
bool TerrainDraw(float dTime);
void TerrainRelease();

extern HeightTerrain* g_pHTerrain;

