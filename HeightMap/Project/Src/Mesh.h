//
// Mesh.h : �޽� ���� ���
//
// DirectX ���� ���� : DX11
//
// 2003.11.4. Kihong Kim / mad_dog@hanmail.net
// 2010.12.01. Update. (DX11.Jun.2010)
// 2016.12.27. Update. (DX11. Windows SDK 8.1)
//

#pragma once

 
// ���� ����ü ��
struct VERTEX
{
	float x, y, z; 			//��ǥ(Position)
	float r, g, b, a;		//����(Diffuse Color)
	//float nx, ny, nz;		//��� : Normal 	 
};


extern VERTEX g_VtxData[];		//���������� : "�޽�" ��
extern DWORD g_VtxCnt;			//��������.��


/****************** end of file "Mesh.h" ***********************************/