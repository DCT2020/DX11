//
// Mesh.h : 메쉬 정보 헤더
//
// DirectX 기준 버전 : DX11
//
// 2003.11.4. Kihong Kim / mad_dog@hanmail.net
// 2010.12.01. Update. (DX11.Jun.2010)
// 2016.12.27. Update. (DX11. Windows SDK 8.1)
//

#pragma once

 
// 정점 구조체 ★
struct VERTEX
{
	float x, y, z; 			//좌표(Position)
	float r, g, b, a;		//색상(Diffuse Color)
	//float nx, ny, nz;		//노멀 : Normal 	 
};


extern VERTEX g_VtxData[];		//정점데이터 : "메쉬" ★
extern DWORD g_VtxCnt;			//정점개수.★


/****************** end of file "Mesh.h" ***********************************/