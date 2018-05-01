//
// Render.cpp : �⺻ ������/�����ӿ� �ҽ�
//
// DirectX ���� ���� : DX11
//
// 2003.11.4. Kihong Kim / mad_dog@hanmail.net
// 2010.12.01. Update. (DX11.Jun.2010)
// 2016.12.27. Update. (DX11. Windows SDK 8.1)
//
#pragma warning(disable:4996)
#include "Windows.h" 
#include "Device.h"
#include "Shader.h"		//���̴� ���� ���.
#include "Render.h"
#include "Mesh.h"		//��

#include "HeightTerrain.h"

TCHAR* g_WindowName = L"Yena::D3D T05 Transform 16 Sphere";



/////////////////////////////////////////////////////////////////////////////
//
//  ���� �����͵�.
//  

/*// ���� ����ü  --> Mesh.h ����.��
struct COLVTX
{
	float x, y, z; 			//��ǥ(Position)
	float r, g, b, a;		//����(Diffuse Color)
};*/

// �𵨿� ���Ϲ���.
ID3D11Buffer*		g_pVB = nullptr;		//���� ���� ������.
ID3D11InputLayout*  g_pVBLayout = nullptr;	//���� �Է±���


// �� ���� ���� 
XMFLOAT3 g_vPos(0, 0, 0);		//��ġ : "���� ����"
XMFLOAT3 g_vRot(0, 0, 0);		//ȸ�� 
XMFLOAT3 g_vScale(1, 1, 1);		//������ 
XMMATRIX g_mTM;					//���庯ȯ���.


int  ObjLoad();
void ObjRelease();
void ObjUpdate(float dTime);
void ObjDraw(float dTime);

void DummyDraw();
void DummyDraw(float x, float y, float z);


///////////////////////////////////////////////////////////////////////////// 
//
// ť�� ������.
//
// - �� ť��� �ﰢ�� ����Ʈ ���·� ����.
// - �� '��' ���� face 2�� ����.
// - ����(Normal) ������ ������ �� ���� '��'�� �ǹ�.
// - ��Ȯ�� ����(���� ó��)�� ���ؼ��� ������ �ߺ��� �Ұ���.
//
// [�׸�] �� ���� ���� ����.
//  v0 --- v1
//   |    / | 
//   |   /  |
//   |  /   |
//   | /    |
//  v2 --- v3
//
/* Mesh.cpp ��� ��
COLVTX	g_vtxCube[] = {			  
	//... 
}; 
*/


////////////////////////////////////////////////////////////////////////////// 
//
// ������ ���� ���� ������/�Լ���.
//

BOOL g_bCullBack = TRUE;		//�޸� ����...On/Off.
BOOL g_bWireFrame = FALSE;		//���̾� ������ On/Off.
BOOL g_bZEnable = TRUE;			//���� ���� ���� On/Off.
BOOL g_bZWrite = TRUE;			//���̹��� ���� On/Off.	

//����
COLOR g_ClearColor(0, 0.125f, 0.3f, 1.0f);
//COLOR g_ClearColor2(0.2f, 0.2f, 0.2f, 1.0);



// 
// �����Ͷ����� ���� ��ü Rasterizer State Objects
// 
enum {
	RS_SOLID,				//�ﰢ�� ä��� : Fill Mode - Soild.
	RS_WIREFRM,				//�ﰢ�� ä��� : Fill Mode - Wireframe.
	RS_CULLBACK,			//�޸� �ø� (ON) : BackFaceCulling - "CCW" 
	RS_WIRECULLBACK,		//���̾� ������ + �޸� �ø� (ON) 

	RS_MAX_
};
//�����Ͷ����� ���� ��ü �迭
ID3D11RasterizerState*	g_RState[RS_MAX_] = { NULL, };

void RasterStateCreate();
void RasterStateRelease();
void RenderModeUpdate();


//������ ��� : �ټ��� ������ ��� ���� �� ����� ���� ����.
enum {
	RM_SOLID		= 0x0000,		// �ﰢ��ä��� : ON, Solid
	RM_WIREFRAME	= 0x0001,		// �ﰢ��ä��� : OFF, Wire-frame
	RM_CULLBACK		= 0x0002,		// �޸� �ø� : ON, "CCW"
	
	//������ �⺻��� : Solid + Cull-On.
	RM_DEFAULT		= RM_SOLID | RM_CULLBACK,	
	
};
DWORD g_RMode = RM_DEFAULT;		//'����' ������ ���.





/////////////////////////////////////////////////////////////////////////////
//
int DataLoading()
{ 
	// ������ �ε�/���� �ڵ�� ���⿡...
	// ...	
	
	//������Ʈ �ε�.
	ObjLoad();

	//������ ���� ��ü ����.
	RasterStateCreate();

	//�׸��� ON, ������ OFF.
	ynGridAxisState(TRUE, FALSE);

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
//
void DataRelease()
{
	// ������ ���� �ڵ�� ���⿡..
	// ... 
	
	ObjRelease();			//������Ʈ ���� 
	RasterStateRelease();	//������ ���� ��ü ����.
	
}




//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//
// Rendering Object.
//
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 

/////////////////////////////////////////////////////////////////////////////
//
//  ������Ʈ : ���� �� �ʿ� ���ҽ� �ε�.
//
int ObjLoad()
{
	HRESULT hr = S_OK;
	
	/*// ���� ������ ���� ���� �迭 
	// �Ϲ������δ� �ܺ�(HDD) ���� �ε��մϴ�.
	COLVTX	Vertices[] = {
		// ������ ��ǥ���� �� ����(Model Space, Local Space)��ǥ.
		// ��ǥ (x, y, z)    ����(R G B A), Alpha �� �⺻�� 1.
		//Face 0 : ���ﰢ��.(CW) 
		{ -1.0f,  0.0f, 0.0f,   1, 0, 0, 1 },	//����. ���� ũ�� ����.
		{  0.0f,  2.0f, 0.0f,   0, 1, 0, 1 },	//�ʷ�.  
		{  1.0f,  0.0f, 0.0f,   0, 0, 1, 1 },	//�Ķ�.  
		//Face 1 : ���ﰢ��.(CCW) 
		{ -0.5f,  0.0f, 0.0f,   1, 0, 0, 1 },	//����.
		{  0.0f, -1.0f, 0.0f,   0, 1, 0, 1 },	//�ʷ�.  
		{  0.5f,  0.0f, 0.0f,   0, 0, 1, 1 },	//�Ķ�.
	};*/
	

	// ���� ���� Vertex Buffer ����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;					//���� �����
	bd.ByteWidth = g_VtxCnt * sizeof(VERTEX);	//���� ũ�� ��
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;		//���� �뵵 : "���� ����" ��� ���� 
	bd.CPUAccessFlags = 0;
	
	D3D11_SUBRESOURCE_DATA rd;				
	ZeroMemory(&rd, sizeof(rd));
	rd.pSysMem = g_VtxData;					//���ۿ� �� ������ ���� : "������"..��
	
	//���� ���� ����.
	hr = g_pDevice->CreateBuffer(&bd, &rd, &g_pVB);
	if (FAILED(hr))	return hr;




	// ���� �Է±��� Input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		//  Sementic          format                       offset         classification             
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// ���� �Է±��� ��ü ���� Create the input layout
	hr = g_pDevice->CreateInputLayout(  layout,
										numElements,
										g_pVSCode->GetBufferPointer(),
										g_pVSCode->GetBufferSize(),
										&g_pVBLayout
									);
	if (FAILED(hr))	return hr;

	
	return TRUE;
}



////////////////////////////////////////////////////////////////////////////// 
//
// ������Ʈ : ���� 
//
void ObjRelease()
{
	SAFE_RELEASE(g_pVB);
	SAFE_RELEASE(g_pVBLayout);
}



////////////////////////////////////////////////////////////////////////////// 
//
// ������Ʈ ����
//
void ObjUpdate(float dTime)
{
	if (IsKeyDown(VK_F9))		//�� ���� ����.
	{
		g_vPos = g_vRot = XMFLOAT3(0, 0, 0);
		g_vScale = XMFLOAT3(1, 1, 1);
	}

	//�ڵ�ȸ��ó��..
	//static bool bAni = true;
	static bool bAutoRot = true;
	if(IsKeyUp('R')){  bAutoRot ^= true; }	

	
	//������ ó��.
	XMMATRIX mScale = XMMatrixScaling(g_vScale.z, g_vScale.y, g_vScale.z);

	//�̵� ó��.
	float mov = 5.0f * dTime;		// 5.0 / sec �̵���.
	if (IsKeyDown(VK_LEFT))  g_vPos.x -= mov;
	if (IsKeyDown(VK_RIGHT)) g_vPos.x += mov;
	if (IsKeyDown(VK_UP))	 g_vPos.z += mov;
	if (IsKeyDown(VK_DOWN))  g_vPos.z -= mov;
	XMMATRIX mTrans = XMMatrixTranslation(g_vPos.x, g_vPos.y, g_vPos.z);

	//ȸ�� ó��.  
	float rot = XM_PI * 0.5f * dTime;		//90��/sec �� ȸ��.(����)
	if (IsKeyDown('A'))  g_vRot.y += rot;
	if (IsKeyDown('D'))  g_vRot.y -= rot;
	if (IsKeyDown('W'))  g_vRot.x += rot;
	if (IsKeyDown('S'))  g_vRot.x -= rot;	
	if(bAutoRot) g_vRot.y += rot * 0.5f;				
	//g_vRot.y += XMConvertToRadians(90) * dTime;	//90��/sec �� ȸ��.(DirectXMath ���)	
	XMMATRIX mRot = XMMatrixRotationRollPitchYaw(g_vRot.x, g_vRot.y, g_vRot.z);
	//XMMATRIX mRot = XMMatrixRotationY(g_vRot.y);
	 
	
	//-----------------------
	// ��ȯ ��� ���� 
	//-----------------------
	g_mTM = mScale * mRot * mTrans;

}




////////////////////////////////////////////////////////////////////////////// 
//
// ������Ʈ �׸���.
//
void ObjDraw()
{
	//���� ���� ����
	UINT stride = sizeof(VERTEX);	//��
	UINT offset = 0;
	g_pDXDC->IASetVertexBuffers(0, 1, &g_pVB, &stride, &offset);

	//�Է� ���̾ƿ� ����. Set the input layout
	g_pDXDC->IASetInputLayout(g_pVBLayout);

	// ���� ���� ���� ���� Set primitive topology
	g_pDXDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	

	//��ȯ ��� ����.
	SetTransform(TS_WORLD, &g_mTM);


	//�׸���! Render a triangle 
	g_pDXDC->Draw(g_VtxCnt, 0);		//��


	//��ġ ���� ���� 
	//g_pDXDC->ClearState();
}





///////////////////////////////////////////////////////////////////////////////
//
// ���� �׸��� 
//
void DummyDraw()
{ 	  
	 
	//�� �������� �� ��ġ - '����' ����� ���� ����. 
 	//����(5, 0, 0) �� �׸���. 
	DummyDraw(5.0f, 0.0f, 0.0f); 
  		
	//����, ������ �������� �������� �׷� ���ô�.
	//... 
		 
	//�� �������� �� ��ġ - '����' ����� ���� ����. 
	//����(-5, 0, 0) �� �׸���. 
	DummyDraw(-5.0f, 0.0f, 0.0f); 
	 
	
	//�� �������� �� ��ġ - '����' ����� ���� ����. 
	//����(5, 0, 5) �� �׸���. 
	DummyDraw( 5.0f, 0.0f, 5.0f); 
	 


	//�� �������� �� ��ġ - '����' ����� ���� ����. 
	//����(-5, 0, 10) �� �׸���. 
	DummyDraw(-5.0f, 0.0f, 10.0f);

 
}






///////////////////////////////////////////////////////////////////////////////
//
// ���� �׸��� : -Z �� �������� �׸���.
//
void DummyDraw(float x, float y, float z)
{ 

	// --------------------------------------------------------------------- 
	// ���̿� �� ������ ���� 
	// ������ �׷���  ��(���ϵ���)�� �����ϴٸ�, ������ ������ �ʿ�� �����ϴ�.
	// ---------------------------------------------------------------------
	// ... ���� ...


	// --------------------------------------------------------------------- 
	// ���� �׸��� : �������� �ʿ��� ������ Draw ���� GPU �� ���޵Ǿ�� �մϴ�.
	// --------------------------------------------------------------------- 
	//��ȯ ��� �غ�.
	MATRIXA mDummy = XMMatrixTranslation(x, y, z); 
	//��ȯ ��� ����.
	SetTransform(TS_WORLD, &mDummy);
	//�׸���.
	g_pDXDC->Draw(g_VtxCnt, 0);	//��

}





////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
//
// [DirectX ���ж��̺귯�� ��� �ȳ�] 
//	
// ������ ����/���� ������ CPU �� ���� �ε��Ҽ� ���갡��(SSE/SSE2)�� ���� 
// SIMD ���� �ڷ��� (XMMATRIX, XMVECTOR) �� ����ϰ� �ֽ��ϴ�.
// ��� ���� ����ó�� -  ������ ����, �б�, ���� - �� �ڷ��� ��ȯ ���� ������ 
// �߰� �۾��� �䱸�Ǹ� �̷� ���� �ڵ��� �������̳� �������� �������ϴ�.
//
// �� SIMD �ڷ��� (XMFLAOT4X4, XMFLOAT3 ...) ���� ����Ѵٸ�, ���� ���� ó���� ������������
// SIMD ������ ���� �� ����, ��κ��� DirectXMath �Լ����� SIMD �ڷ����� ���߾� ����Ǿ� �־� 
// ���� ������ �����ε��� ����ڰ� ���� �߰��ؾ� �մϴ�.
//
// DirectXTK / SimpleMath �� ����ϸ� ���� 2���� ������� ������ ����, ���� �������̰� ������ ���� 
// �ڵ带 ���� �� �� �ֽ��ϴ�. �� DirectXMath ���� ���� �ణ�� ���� ���ϰ� �߻��մϴ�.
//
// ���� �ڼ��� ���� �Ʒ��� �����Ͻʽÿ�.
//
// ��ũ1 : DirectXMath ���̵� https://msdn.microsoft.com/ko-kr/library/windows/desktop/ee415571(v=vs.85).aspx
// ��ũ2 : DirectX�� SSE ���� https://blogs.msdn.microsoft.com/chuckw/2012/09/11/directxmath-sse-sse2-and-arm-neon/
// ��ũ3 : DirectXTK / SimeMath https://github.com/Microsoft/DirectXTK/wiki/SimpleMath
//	
////////////////////////////////////////////////////////////////////////////// 
//
// ��� ���� ��ü : ī�޶�, ���� ���..
//
////////////////////////////////////////////////////////////////////////////// 
XMFLOAT3		g_vEye(0.0f, 3.0f, -13.0f);		//ī�޶� ��ġ.(Position)
XMFLOAT3		g_vLookAt(0.0f, 0.0f, 0.0f);	//�ٶ󺸴� ��.(Position)
XMFLOAT3		g_vUp(0.0f, 1.0f, 0.0f);		//ī�޶� ��� ����1.(Direction)

// ���� ��ȯ ����. 
float g_fFov   = XMConvertToRadians(45);	//�⺻ FOV �ޱ�. Field of View (Y) 
float g_fAspect =  1.333f;		//����:���� ����. 800:600 = 1.33:1 (4:3)   960:600 = 1.6:1 (16:10 HD)
float g_fZnear = 1.0f;			//�þ� �ּ� �Ÿ� (1.0m) 
float g_fZfar = 100.0f;			//�þ� �ִ� �Ÿ� (100m) 


/////////////////////////////////////////////////////////////////////////////// 
//
// ī�޶� ����.
//
void CameraUpdate(float dTime)
{
 
	//���ΰ� �ֽ� �׽�Ʈ
	//g_vLookAt = g_vPos;	

	/*//ī�޶�, ���ΰ��� �Բ� �����̱�.
	XMFLOAT3 cpos(0, 2, -10);
	g_vEye = g_vPos + cpos;			//ī�޶� ��ġ : ���ΰ� '���� ��ġ�� ���� �����Ÿ� ������.
	g_vLookAt = g_vPos;				//ī�޶� ���� : ���ΰ� '����' ��ġ.
	*/


	// ī�޶� ��ü�� "��ġ" �� "����" ���� 
	XMVECTOR eye	= XMLoadFloat3(&g_vEye);	//ī�޶� ��ġ 
	XMVECTOR lookat = XMLoadFloat3(&g_vLookAt);	//�ٶ󺸴� ��.��ġ.
	XMVECTOR up		= XMLoadFloat3(&g_vUp);		//ī�޶� ��� ����.	
	// �� ��ȯ ��� ���� :  View Transform 
	XMMATRIX mView = XMMatrixLookAtLH(eye, lookat, up);			

 
	// ī�޶� "����" �� "�Կ� ����" ����.  
	g_fAspect = g_Mode.Width/(float)g_Mode.Height;	 //[�׽�Ʈ0] ���� �ػ� ���� ����.
	//g_fAspect = 800/(float)600;					 //[�׽�Ʈ1] 4:3 �ػ� ����(����Ȯ��).
	//g_fAspect = 600/(float)600;					 //[�׽�Ʈ2] 1:1 �ػ� ����(����Ȯ��).
	//g_fFov = XMConvertToRadians(15);		//[�׽�Ʈ3] FOV �׽�Ʈ.(Zoom-In) 
	//g_fFov = XMConvertToRadians(90);		//[�׽�Ʈ4] FOV �׽�Ʈ.(Zoom-Out) 
	//g_fFov = XMConvertToRadians(120);		//[�׽�Ʈ5] FOV �׽�Ʈ.(Zoom-Out) 
	//g_fZnear = 0;							//[�׽�Ʈ6] ����� �׽�Ʈ.
	//g_fZnear = 10;						//[�׽�Ʈ7] ����� �׽�Ʈ.
	//g_fZfar = 20;							//[�׽�Ʈ8] ����� �׽�Ʈ.
		
	// ���� ���� ��ȯ ��� ���� : Projection Transform.
	XMMATRIX mProj = XMMatrixPerspectiveFovLH(g_fFov, g_fAspect, g_fZnear, g_fZfar);	

	

	//��ġ�� ��ȯ��� ����.
	SetTransform(TS_VIEW, &mView);
	SetTransform(TS_PROJ, &mProj);


	//Yena �� ī�޶� ������ ���� 
	//Yena ���� ��ü���� ���Ž� ī�޶� ������ �ʿ��մϴ�. 
	//����� ī�޶� ����Ѵٸ�, �ݵ�� Yena �� �����ؾ� �մϴ�.
	yn_View = mView;
	yn_Proj = mProj;

}





////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
//
// ������ ���� ��ü ���� �Լ���.
//
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
// 
// ������ ���� ��ü ����.
//
void RasterStateCreate()
{
	//[���°�ü 1] �⺻ ������ ���� ��ü.
	D3D11_RASTERIZER_DESC rd;
	rd.FillMode = D3D11_FILL_SOLID;		//�ﰢ�� ���� ä���.(�⺻��)
	rd.CullMode = D3D11_CULL_NONE;		//�ø� ����. (�⺻���� �ø� Back)		
	rd.FrontCounterClockwise = false;   //���� �⺻��...
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0;
	rd.SlopeScaledDepthBias = 0;
	rd.DepthClipEnable = true;
	rd.ScissorEnable = false;
	rd.MultisampleEnable = false; 
	rd.AntialiasedLineEnable = false;
	//�����Ͷ����� ��ü ����.
	g_pDevice->CreateRasterizerState(&rd, &g_RState[RS_SOLID]);


	//[���°�ü2] ���̾� ������ �׸���. 
	rd.FillMode = D3D11_FILL_WIREFRAME; 
	rd.CullMode = D3D11_CULL_NONE;
	g_pDevice->CreateRasterizerState(&rd, &g_RState[RS_WIREFRM]);

	//[���°�ü3] �ø� On! "CCW"
	rd.FillMode = D3D11_FILL_SOLID; 
	rd.CullMode = D3D11_CULL_BACK;
	g_pDevice->CreateRasterizerState(&rd, &g_RState[RS_CULLBACK]);
	
	//[���°�ü4] ���̾� ������ + �ø� On! "CCW"
	rd.FillMode = D3D11_FILL_WIREFRAME; 
	rd.CullMode = D3D11_CULL_BACK; 
	g_pDevice->CreateRasterizerState(&rd, &g_RState[RS_WIRECULLBACK]);

}




////////////////////////////////////////////////////////////////////////////// 
//
// ������ ���� ��ü ���� : ���ø����̼� ����� ȣ��.
//
void RasterStateRelease()
{
	for (int i = 0; i<RS_MAX_; i++)
	{
		SAFE_RELEASE(g_RState[i]);
	}

}


 

//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//
// System Functions
//
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//
// ����-�ý��� ���� ������  ����.. : �׷����� �սô�. ���� ������ ���Դϴ�.
// 
float EngineUpdate()
{
	//���� �ð� / Ÿ�̸� ���.
	float dTime = GetEngineTime();

	//Yena ���� : ���ʿ��� ����� �ּ�ó���Ͽ� ����� ��Ȱ��ȭ �Ͻʽÿ�.
	//ynCameraUpdate(dTime);		// ī�޶� ����	
	//ynLightsUpdate(dTime);		// ���� ����						
	ynGridAxisUpdate(dTime);		// ������, �׸��� ����.
	//YenaUpdate(dTime);			// ���� �ý��� ��ü �ϰ� ����.


	// ������ �ɼ� ���� 	 
	if (IsKeyUp(VK_SPACE))	g_bWireFrame ^= TRUE;
	if (IsKeyUp(VK_F4))		g_bCullBack ^= TRUE;
	if (IsKeyUp(VK_F5))		g_bZEnable ^= TRUE;		
	if (IsKeyUp(VK_F6))		g_bZWrite ^= TRUE;		


	// ���� ����.
	if (g_bWireFrame) g_ClearColor = COLOR(0.2f, 0.2f, 0.2f, 1.0f);
	else			  g_ClearColor = COLOR(0, 0.125f, 0.3f, 1.0f); 
	
	// ������ ��� ��ȯ.	  
	RenderModeUpdate();
	 	
	// ���� ���� ��� ��ȯ.	 
	if (g_bZEnable)
	{
		if (g_bZWrite)  
			  g_pDXDC->OMSetDepthStencilState(g_DSState[DS_DEPTH_ON], 0);			//���� ���� ���� (�⺻��)
		else  g_pDXDC->OMSetDepthStencilState(g_DSState[DS_DEPTH_WRITE_OFF], 0);	//���� ����  : Z-Test On + Z-Write Off.
	}
	else  g_pDXDC->OMSetDepthStencilState(g_DSState[DS_DEPTH_OFF], 0);	//���� ���� ��Ȱ��ȭ : Z-Test Off + Z-Write Off.
	

	return dTime;
}



/////////////////////////////////////////////////////////////////////////////// 
//
// ������ ��� üũ : �� ������ ��� ���º� On/Off ó��.
//
#define CheckRMode(k, v) if((k)) g_RMode |= (v); else g_RMode &= ~(v);


/////////////////////////////////////////////////////////////////////////////// 
//
// ������ ��� ��ȯ
//
void RenderModeUpdate()
{	
	// ������ ��� üũ : ����ڰ� ������ ������ ���� ����.
	CheckRMode(g_bCullBack, RM_CULLBACK);
	CheckRMode(g_bWireFrame, RM_WIREFRAME);


	// ������ ��� ��ȯ : ������ ����� ������ ���� ������ ���¸� ����.
	switch (g_RMode)
	{
	default:
	case RM_SOLID:
		g_pDXDC->RSSetState(g_RState[RS_SOLID]);
		break;
	case RM_WIREFRAME:
		g_pDXDC->RSSetState(g_RState[RS_WIREFRM]);
		break;
	case RM_CULLBACK:
		g_pDXDC->RSSetState(g_RState[RS_CULLBACK]);
		break;
	case RM_WIREFRAME | RM_CULLBACK:
		g_pDXDC->RSSetState(g_RState[RS_WIRECULLBACK]);
		break;
	} 

}





/////////////////////////////////////////////////////////////////////////////// 
//
//  ����. 
//
void ShowInfo()
{ 
	PutFPS(0, 0);

	static bool bShow = true;
	if (IsKeyUp(VK_F1)) bShow ^= true;

	if (!bShow)
	{
		//ynTextDraw(1,20, COLOR(1, 1, 0, 1), "[Help] F1"); 
		return;
	}


	// Today's Topic.
	{
		int x = g_Mode.Width/2-150, y = 1;
		COLOR col(1, 1, 1, 1);
		COLOR col2(1, 1, 0, 1);
		ynTextDraw(x, y, col, L"�� %s", g_WindowName);

		ynTextDraw(x, y += 14, col2, L"> ��ü ������ ������������ �帧�� ���� ");
		ynTextDraw(x, y += 14, col,  L"> ���� ���������� - ��ȯ Transform �� ����");
		ynTextDraw(x, y += 14, col,  L"1. ���� ��ȯ (World Transform) ����");
		ynTextDraw(x, y += 14, col,  L"2. �� ��ȯ (View Transform) ����");
		ynTextDraw(x, y += 14, col,  L"3. ���� ��ȯ (Projection Transform) ����"); 
		ynTextDraw(x, y += 14, col2, L"4.���� ����(Depth Buffer) �� ���.");
		ynTextDraw(x, y += 14, col2, L"5.���� �׽�Ʈ(Depth Test) �� ����.");			  
		ynTextDraw(x, y += 14, col*0.4f, L"6.W-Buffer �� ���Ͽ� �����մϴ�.");
		ynTextDraw(x, y += 14, col2, L"7.�޸�����(Back-Face Culling) �� ����");			
		ynTextDraw(x, y += 14, col,  L"8.Ŭ����(Clipping) �� ���ؿ� ����");
		
		y = g_Mode.Height /12 * 8;
		ynTextDraw(x, y += 14, col2, L"> ���ϵ��� �׸���"); 
		ynTextDraw(x, y += 14, col,  L"1. Box");
		ynTextDraw(x, y += 14, col2, L"2. Sphere");
		ynTextDraw(x, y += 14, col2, L"* ��������, �⺻��(���) ���� ����.");
		//y = g_Mode.Height - 200;
		//ynTextDraw(x, y += 14, col2, L" * ����� �Է� ó�� �� �� ��ȯ ���� * ");

	}



	//�ý��� ����.
	{
		int x = 1, y = 14;
		COLOR col(0, 1, 0, 1);
		SystemInfo(x, y, col); 
	}

	//�߰� ����..
	int x = 1, y = 120;
	COLOR col(1, 1, 1, 1);
	COLOR col2(1, 1, 0, 1);
	y += 14;
	ynTextDraw(x, y += 14, col, L"ä���: Space (%s)", g_bWireFrame?L"WIRE":L"SOLID");
	ynTextDraw(x, y += 14, col, L"�޸�����: F4 (%s)", g_bCullBack?L"ON":L"OFF");
	ynTextDraw(x, y += 14, col2, L"�����׽�Ʈ: F5 (%s)", g_bZEnable?L"ON":L"OFF");
	ynTextDraw(x, y += 14, g_bZEnable?col2:col2*0.5f, L"���̾���: F6 (%s)", g_bZWrite?L"ON":L"OFF");

	//�� ���� ���.
	y += 14;
	ynTextDraw(x, y += 14, col, L"[Model]");
	ynTextDraw(x, y += 14, col, L"���� : F9");
	ynTextDraw(x, y += 14, col, L"�̵� : ����Ű");
	ynTextDraw(x, y += 14, col, L"ȸ�� : ADWS");
	ynTextDraw(x, y += 14, col, L"�ڵ� : R");
	
	col = COLOR(0, 1, 1, 1);
	y = g_Mode.Height - 14 * 5;
	ynTextDraw(x, y += 14, col, L"[Model]");
	ynTextDraw(x, y += 14, col, L"Pos = { %.2f, %.2f, %.2f }", g_vPos.x, g_vPos.y, g_vPos.z);	
	ynTextDraw(x, y += 14, col, L"Rot = { %.2f, %.2f, %.2f }", g_vRot.x, g_vRot.y, g_vRot.z);	
	ynTextDraw(x, y += 14, col, L"Scl = { %.2f, %.2f, %.2f }", g_vScale.x, g_vScale.y, g_vScale.z);	
	 

	//ī�޶� ���� ���.
	x = 200;  y = g_Mode.Height - 14 * 5;
	//col = COLOR(1, 1, 0, 1)*0.8f;
	ynTextDraw(x, y+=14, col, L"[ī�޶�]");  
	ynTextDraw(x, y+=14, col, L"Eye={%.2f, %.2f, %.2f}", g_vEye.x, g_vEye.y, g_vEye.z);  
	ynTextDraw(x, y+=14, col, L"Look={%.2f, %.2f, %.2f}", g_vLookAt.x, g_vLookAt.y, g_vLookAt.z);  
	ynTextDraw(x, y+=14, col, L"Up={%.2f, %.2f, %.2f}", g_vUp.x, g_vUp.y, g_vUp.z);  

	//ī�޶� - "����" : ������ȯ ���� ���.
	x = 400;  y = g_Mode.Height - 14 * 5;
	//col = COLOR(1, 1, 0, 1);
	ynTextDraw(x, y+=14, col, L"[����]");  
	ynTextDraw(x, y+=14, col, L"FOV=%.2f (%.2f��)", g_fFov, XMConvertToDegrees(g_fFov)); 
	ynTextDraw(x, y+=14, col, L"Aspect=%.2f", g_fAspect);  
	ynTextDraw(x, y+=14, col, L"Zn=%.1f  Zf = %.1f", g_fZnear, g_fZfar);

}

 


////////////////////////////////////////////////////////////////////////////// 
//
// ��ü ��� �׸���
//
void SceneRender()
{
	//-------------------------------
	// ����/�ý��� ����.
	//------------------------------- 
	float dTime = EngineUpdate();

	// ��� ���� 
	//
	CameraUpdate(dTime);	  //ī�޶󰻽� : ��-���� ��ȯ��� ��� 
	
	// ���ΰ�, ����, ������ �̵�, �浹�˻� ���..
	// ...
	ObjUpdate(dTime);

	// ���� ����, �̺�Ʈ, ����, �������..
	// ...	 
	

	//-------------------------------
	// ��� �׸��� ����.. 
	//-------------------------------
 	//����Ÿ��(�����)/ ���̹��� �����..
	ClearBackBuffer(D3D11_CLEAR_DEPTH, g_ClearColor);
		
	//�ý��� ��ü ���.
	//ynGridAxisDraw();

	//��ü ������ : ���ΰ�, ����, ����.. 
  	ObjDraw();
	DummyDraw();		//���� �׸��� : +Z �� ���� 


	//�ý��� ��ü & ���� ���..
	ynGridAxisDraw();
	ShowInfo();

	//-------------------------------
	// ��� �׸��� ����.
	//------------------------------- 
	Flip();
	

}//end of void SceneRender()






/****************** end of file "Render.cpp" *********************************/