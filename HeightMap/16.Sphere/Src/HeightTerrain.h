#pragma once

struct HeightTerrain
{
	XMFLOAT3 m_vPos;		//��ġ : "���� ����"
	XMFLOAT3 m_vRot;		//ȸ�� 
	XMFLOAT3 m_vScale;		//������ 
	XMFLOAT4X4 m_mTM;					//���庯ȯ���.
};

bool TerrainCreate(const TCHAR* rawFile);
bool TerrainUpdate(float dTime);
bool TerrainDraw(float dTime);
void TerrainRelease();

extern HeightTerrain* g_pHTerrain;

