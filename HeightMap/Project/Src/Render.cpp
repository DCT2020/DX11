//
// Render.cpp : 기본 렌더링/프레임웍 소스
//
// DirectX 기준 버전 : DX11
//
// 2003.11.4. Kihong Kim / mad_dog@hanmail.net
// 2010.12.01. Update. (DX11.Jun.2010)
// 2016.12.27. Update. (DX11. Windows SDK 8.1)
//
#pragma warning(disable:4996)
#include "Windows.h" 
#include "Device.h"
#include "Shader.h"		//셰이더 관련 헤더.
#include "Render.h"
#include "Mesh.h"		//★

#include "HeightTerrain.h"

TCHAR* g_WindowName = L"Yena::D3D T05 Transform 16 Sphere";



/////////////////////////////////////////////////////////////////////////////
//
//  전역 데이터들.
//  

/*// 정점 구조체  --> Mesh.h 참조.★
struct COLVTX
{
	float x, y, z; 			//좌표(Position)
	float r, g, b, a;		//색상(Diffuse Color)
};*/

// 모델용 기하버퍼.
ID3D11Buffer*		g_pVB = nullptr;		//정점 버퍼 포인터.
ID3D11InputLayout*  g_pVBLayout = nullptr;	//정점 입력구조


// 모델 기하 정보 
XMFLOAT3 g_vPos(0, 0, 0);		//위치 : "월드 공간"
XMFLOAT3 g_vRot(0, 0, 0);		//회전 
XMFLOAT3 g_vScale(1, 1, 1);		//스케일 
XMMATRIX g_mTM;					//월드변환행렬.


int  ObjLoad();
void ObjRelease();
void ObjUpdate(float dTime);
void ObjDraw(float dTime);

void DummyDraw();
void DummyDraw(float x, float y, float z);


///////////////////////////////////////////////////////////////////////////// 
//
// 큐브 데이터.
//
// - 각 큐브는 삼각형 리스트 형태로 제작.
// - 각 '면' 별로 face 2개 설정.
// - 법선(Normal) 벡터의 방향은 각 면의 '밖'을 의미.
// - 정확한 법선(음영 처리)을 위해서는 정점의 중복은 불가피.
//
// [그림] 각 면의 정점 구성.
//  v0 --- v1
//   |    / | 
//   |   /  |
//   |  /   |
//   | /    |
//  v2 --- v3
//
/* Mesh.cpp 사용 ★
COLVTX	g_vtxCube[] = {			  
	//... 
}; 
*/


////////////////////////////////////////////////////////////////////////////// 
//
// 렌더링 상태 조절 데이터/함수들.
//

BOOL g_bCullBack = TRUE;		//뒷면 제거...On/Off.
BOOL g_bWireFrame = FALSE;		//와이어 프레임 On/Off.
BOOL g_bZEnable = TRUE;			//깊이 버퍼 연산 On/Off.
BOOL g_bZWrite = TRUE;			//깊이버퍼 쓰기 On/Off.	

//배경색
COLOR g_ClearColor(0, 0.125f, 0.3f, 1.0f);
//COLOR g_ClearColor2(0.2f, 0.2f, 0.2f, 1.0);



// 
// 레스터라이져 상태 객체 Rasterizer State Objects
// 
enum {
	RS_SOLID,				//삼각형 채우기 : Fill Mode - Soild.
	RS_WIREFRM,				//삼각형 채우기 : Fill Mode - Wireframe.
	RS_CULLBACK,			//뒷면 컬링 (ON) : BackFaceCulling - "CCW" 
	RS_WIRECULLBACK,		//와이어 프레임 + 뒷면 컬링 (ON) 

	RS_MAX_
};
//레스터라이져 상태 객체 배열
ID3D11RasterizerState*	g_RState[RS_MAX_] = { NULL, };

void RasterStateCreate();
void RasterStateRelease();
void RenderModeUpdate();


//렌더링 모드 : 다수의 렌더링 모드 조합 및 운용을 위한 정의.
enum {
	RM_SOLID		= 0x0000,		// 삼각형채우기 : ON, Solid
	RM_WIREFRAME	= 0x0001,		// 삼각형채우기 : OFF, Wire-frame
	RM_CULLBACK		= 0x0002,		// 뒷면 컬링 : ON, "CCW"
	
	//렌더링 기본모드 : Solid + Cull-On.
	RM_DEFAULT		= RM_SOLID | RM_CULLBACK,	
	
};
DWORD g_RMode = RM_DEFAULT;		//'현재' 렌더링 모드.





/////////////////////////////////////////////////////////////////////////////
//
int DataLoading()
{ 
	// 데이터 로딩/생성 코드는 여기에...
	// ...	
	
	//오브젝트 로딩.
	ObjLoad();

	//레스터 상태 개체 생성.
	RasterStateCreate();

	//그리드 ON, 방향축 OFF.
	ynGridAxisState(TRUE, FALSE);

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
//
void DataRelease()
{
	// 데이터 해제 코드는 여기에..
	// ... 
	
	ObjRelease();			//오브젝트 삭제 
	RasterStateRelease();	//레스터 상태 개체 제거.
	
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
//  오브젝트 : 생성 및 필요 리소스 로딩.
//
int ObjLoad()
{
	HRESULT hr = S_OK;
	
	/*// 기하 도형을 위한 정점 배열 
	// 일반적으로는 외부(HDD) 에서 로딩합니다.
	COLVTX	Vertices[] = {
		// 정점의 좌표값은 모델 공간(Model Space, Local Space)좌표.
		// 좌표 (x, y, z)    색상(R G B A), Alpha 는 기본값 1.
		//Face 0 : 정삼각형.(CW) 
		{ -1.0f,  0.0f, 0.0f,   1, 0, 0, 1 },	//빨강. 모델의 크기 변경.
		{  0.0f,  2.0f, 0.0f,   0, 1, 0, 1 },	//초록.  
		{  1.0f,  0.0f, 0.0f,   0, 0, 1, 1 },	//파랑.  
		//Face 1 : 역삼각형.(CCW) 
		{ -0.5f,  0.0f, 0.0f,   1, 0, 0, 1 },	//빨강.
		{  0.0f, -1.0f, 0.0f,   0, 1, 0, 1 },	//초록.  
		{  0.5f,  0.0f, 0.0f,   0, 0, 1, 1 },	//파랑.
	};*/
	

	// 정점 버퍼 Vertex Buffer 생성
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;					//버퍼 사용방식
	bd.ByteWidth = g_VtxCnt * sizeof(VERTEX);	//버퍼 크기 ★
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;		//버퍼 용도 : "정점 버퍼" 용로 설정 
	bd.CPUAccessFlags = 0;
	
	D3D11_SUBRESOURCE_DATA rd;				
	ZeroMemory(&rd, sizeof(rd));
	rd.pSysMem = g_VtxData;					//버퍼에 들어갈 데이터 설정 : "정점들"..★
	
	//정점 버퍼 생성.
	hr = g_pDevice->CreateBuffer(&bd, &rd, &g_pVB);
	if (FAILED(hr))	return hr;




	// 정점 입력구조 Input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		//  Sementic          format                       offset         classification             
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// 정접 입력구조 객체 생성 Create the input layout
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
// 오브젝트 : 제거 
//
void ObjRelease()
{
	SAFE_RELEASE(g_pVB);
	SAFE_RELEASE(g_pVBLayout);
}



////////////////////////////////////////////////////////////////////////////// 
//
// 오브젝트 갱신
//
void ObjUpdate(float dTime)
{
	if (IsKeyDown(VK_F9))		//모델 정보 리셋.
	{
		g_vPos = g_vRot = XMFLOAT3(0, 0, 0);
		g_vScale = XMFLOAT3(1, 1, 1);
	}

	//자동회전처리..
	//static bool bAni = true;
	static bool bAutoRot = true;
	if(IsKeyUp('R')){  bAutoRot ^= true; }	

	
	//스케일 처리.
	XMMATRIX mScale = XMMatrixScaling(g_vScale.z, g_vScale.y, g_vScale.z);

	//이동 처리.
	float mov = 5.0f * dTime;		// 5.0 / sec 이동량.
	if (IsKeyDown(VK_LEFT))  g_vPos.x -= mov;
	if (IsKeyDown(VK_RIGHT)) g_vPos.x += mov;
	if (IsKeyDown(VK_UP))	 g_vPos.z += mov;
	if (IsKeyDown(VK_DOWN))  g_vPos.z -= mov;
	XMMATRIX mTrans = XMMatrixTranslation(g_vPos.x, g_vPos.y, g_vPos.z);

	//회전 처리.  
	float rot = XM_PI * 0.5f * dTime;		//90º/sec 씩 회전.(라디안)
	if (IsKeyDown('A'))  g_vRot.y += rot;
	if (IsKeyDown('D'))  g_vRot.y -= rot;
	if (IsKeyDown('W'))  g_vRot.x += rot;
	if (IsKeyDown('S'))  g_vRot.x -= rot;	
	if(bAutoRot) g_vRot.y += rot * 0.5f;				
	//g_vRot.y += XMConvertToRadians(90) * dTime;	//90º/sec 씩 회전.(DirectXMath 사용)	
	XMMATRIX mRot = XMMatrixRotationRollPitchYaw(g_vRot.x, g_vRot.y, g_vRot.z);
	//XMMATRIX mRot = XMMatrixRotationY(g_vRot.y);
	 
	
	//-----------------------
	// 변환 행렬 결합 
	//-----------------------
	g_mTM = mScale * mRot * mTrans;

}




////////////////////////////////////////////////////////////////////////////// 
//
// 오브젝트 그리기.
//
void ObjDraw()
{
	//정점 버퍼 설정
	UINT stride = sizeof(VERTEX);	//★
	UINT offset = 0;
	g_pDXDC->IASetVertexBuffers(0, 1, &g_pVB, &stride, &offset);

	//입력 레이아웃 설정. Set the input layout
	g_pDXDC->IASetInputLayout(g_pVBLayout);

	// 기하 위상 구조 설정 Set primitive topology
	g_pDXDC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	

	//변환 행렬 설정.
	SetTransform(TS_WORLD, &g_mTM);


	//그리기! Render a triangle 
	g_pDXDC->Draw(g_VtxCnt, 0);		//★


	//장치 상태 리셋 
	//g_pDXDC->ClearState();
}





///////////////////////////////////////////////////////////////////////////////
//
// 더미 그리기 
//
void DummyDraw()
{ 	  
	 
	//새 오브제의 새 위치 - '월드' 행렬을 새로 적용. 
 	//월드(5, 0, 0) 에 그리기. 
	DummyDraw(5.0f, 0.0f, 0.0f); 
  		
	//이하, 동일한 오브제를 여러곳에 그려 봅시다.
	//... 
		 
	//새 오브제의 새 위치 - '월드' 행렬을 새로 적용. 
	//월드(-5, 0, 0) 에 그리기. 
	DummyDraw(-5.0f, 0.0f, 0.0f); 
	 
	
	//새 오브제의 새 위치 - '월드' 행렬을 새로 적용. 
	//월드(5, 0, 5) 에 그리기. 
	DummyDraw( 5.0f, 0.0f, 5.0f); 
	 


	//새 오브제의 새 위치 - '월드' 행렬을 새로 적용. 
	//월드(-5, 0, 10) 에 그리기. 
	DummyDraw(-5.0f, 0.0f, 10.0f);

 
}






///////////////////////////////////////////////////////////////////////////////
//
// 더미 그리기 : -Z 축 방향으로 그리기.
//
void DummyDraw(float x, float y, float z)
{ 

	// --------------------------------------------------------------------- 
	// 더미용 모델 데이터 설정 
	// 이전에 그려진  모델(기하도형)과 동일하다면, 새로이 설정할 필요는 없습니다.
	// ---------------------------------------------------------------------
	// ... 생략 ...


	// --------------------------------------------------------------------- 
	// 더미 그리기 : 렌더링에 필요한 정보는 Draw 전에 GPU 에 전달되어야 합니다.
	// --------------------------------------------------------------------- 
	//변환 행렬 준비.
	MATRIXA mDummy = XMMatrixTranslation(x, y, z); 
	//변환 행렬 설정.
	SetTransform(TS_WORLD, &mDummy);
	//그리기.
	g_pDXDC->Draw(g_VtxCnt, 0);	//★

}





////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
//
// [DirectX 수학라이브러리 사용 안내] 
//	
// 예제의 수학/벡터 연산은 CPU 의 빠른 부동소수 연산가속(SSE/SSE2)을 위해 
// SIMD 지원 자료형 (XMMATRIX, XMVECTOR) 을 사용하고 있습니다.
// 대신 개별 정보처리 -  데이터 저장, 읽기, 쓰기 - 와 자료형 변환 등의 불편한 
// 추가 작업이 요구되며 이로 인해 코드의 간결함이나 가독성도 떨어집니다.
//
// 비 SIMD 자료형 (XMFLAOT4X4, XMFLOAT3 ...) 등을 사용한다면, 개별 정보 처리가 용이해지지만
// SIMD 가속을 받을 수 없고, 대부분의 DirectXMath 함수등이 SIMD 자료형에 맞추어 설계되어 있어 
// 여러 연산자 오버로딩을 사용자가 직접 추가해야 합니다.
//
// DirectXTK / SimpleMath 를 사용하면 위의 2가지 장단점을 적절히 차용, 보다 직관적이고 가독성 높은 
// 코드를 유지 할 수 있습니다. 단 DirectXMath 사용시 보다 약간의 성능 저하가 발생합니다.
//
// 좀더 자세한 것은 아래를 참조하십시오.
//
// 링크1 : DirectXMath 가이드 https://msdn.microsoft.com/ko-kr/library/windows/desktop/ee415571(v=vs.85).aspx
// 링크2 : DirectX와 SSE 가속 https://blogs.msdn.microsoft.com/chuckw/2012/09/11/directxmath-sse-sse2-and-arm-neon/
// 링크3 : DirectXTK / SimeMath https://github.com/Microsoft/DirectXTK/wiki/SimpleMath
//	
////////////////////////////////////////////////////////////////////////////// 
//
// 장면 공통 객체 : 카메라, 조명 등등..
//
////////////////////////////////////////////////////////////////////////////// 
XMFLOAT3		g_vEye(0.0f, 3.0f, -13.0f);		//카메라 위치.(Position)
XMFLOAT3		g_vLookAt(0.0f, 0.0f, 0.0f);	//바라보는 곳.(Position)
XMFLOAT3		g_vUp(0.0f, 1.0f, 0.0f);		//카메라 상방 벡터1.(Direction)

// 투영 변환 정보. 
float g_fFov   = XMConvertToRadians(45);	//기본 FOV 앵글. Field of View (Y) 
float g_fAspect =  1.333f;		//가로:세로 비율. 800:600 = 1.33:1 (4:3)   960:600 = 1.6:1 (16:10 HD)
float g_fZnear = 1.0f;			//시야 최소 거리 (1.0m) 
float g_fZfar = 100.0f;			//시야 최대 거리 (100m) 


/////////////////////////////////////////////////////////////////////////////// 
//
// 카메라 갱신.
//
void CameraUpdate(float dTime)
{
 
	//주인공 주시 테스트
	//g_vLookAt = g_vPos;	

	/*//카메라, 주인공과 함께 움직이기.
	XMFLOAT3 cpos(0, 2, -10);
	g_vEye = g_vPos + cpos;			//카메라 위치 : 주인공 '현재 위치로 부터 일정거리 떨어짐.
	g_vLookAt = g_vPos;				//카메라 시점 : 주인공 '현재' 위치.
	*/


	// 카메라 몸체의 "위치" 와 "방향" 정보 
	XMVECTOR eye	= XMLoadFloat3(&g_vEye);	//카메라 위치 
	XMVECTOR lookat = XMLoadFloat3(&g_vLookAt);	//바라보는 곳.위치.
	XMVECTOR up		= XMLoadFloat3(&g_vUp);		//카메라 상방 벡터.	
	// 뷰 변환 행렬 생성 :  View Transform 
	XMMATRIX mView = XMMatrixLookAtLH(eye, lookat, up);			

 
	// 카메라 "렌즈" 및 "촬영 영역" 정보.  
	g_fAspect = g_Mode.Width/(float)g_Mode.Height;	 //[테스트0] 현재 해상도 기준 설정.
	//g_fAspect = 800/(float)600;					 //[테스트1] 4:3 해상도 설정(오류확인).
	//g_fAspect = 600/(float)600;					 //[테스트2] 1:1 해상도 설정(오류확인).
	//g_fFov = XMConvertToRadians(15);		//[테스트3] FOV 테스트.(Zoom-In) 
	//g_fFov = XMConvertToRadians(90);		//[테스트4] FOV 테스트.(Zoom-Out) 
	//g_fFov = XMConvertToRadians(120);		//[테스트5] FOV 테스트.(Zoom-Out) 
	//g_fZnear = 0;							//[테스트6] 근평면 테스트.
	//g_fZnear = 10;						//[테스트7] 근평면 테스트.
	//g_fZfar = 20;							//[테스트8] 원평면 테스트.
		
	// 원근 투영 변환 행렬 생성 : Projection Transform.
	XMMATRIX mProj = XMMatrixPerspectiveFovLH(g_fFov, g_fAspect, g_fZnear, g_fZfar);	

	

	//장치에 변환행렬 설정.
	SetTransform(TS_VIEW, &mView);
	SetTransform(TS_PROJ, &mProj);


	//Yena 로 카메라 정보를 전달 
	//Yena 공통 객체들은 갱신시 카메라 정보가 필요합니다. 
	//사용자 카메라를 사용한다면, 반드시 Yena 로 전달해야 합니다.
	yn_View = mView;
	yn_Proj = mProj;

}





////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
//
// 렌더링 상태 객체 관련 함수들.
//
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
// 
// 레스터 상태 객체 생성.
//
void RasterStateCreate()
{
	//[상태객체 1] 기본 렌더링 상태 개체.
	D3D11_RASTERIZER_DESC rd;
	rd.FillMode = D3D11_FILL_SOLID;		//삼각형 색상 채우기.(기본값)
	rd.CullMode = D3D11_CULL_NONE;		//컬링 없음. (기본값은 컬링 Back)		
	rd.FrontCounterClockwise = false;   //이하 기본값...
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0;
	rd.SlopeScaledDepthBias = 0;
	rd.DepthClipEnable = true;
	rd.ScissorEnable = false;
	rd.MultisampleEnable = false; 
	rd.AntialiasedLineEnable = false;
	//레스터라이져 객체 생성.
	g_pDevice->CreateRasterizerState(&rd, &g_RState[RS_SOLID]);


	//[상태객체2] 와이어 프레임 그리기. 
	rd.FillMode = D3D11_FILL_WIREFRAME; 
	rd.CullMode = D3D11_CULL_NONE;
	g_pDevice->CreateRasterizerState(&rd, &g_RState[RS_WIREFRM]);

	//[상태객체3] 컬링 On! "CCW"
	rd.FillMode = D3D11_FILL_SOLID; 
	rd.CullMode = D3D11_CULL_BACK;
	g_pDevice->CreateRasterizerState(&rd, &g_RState[RS_CULLBACK]);
	
	//[상태객체4] 와이어 프레임 + 컬링 On! "CCW"
	rd.FillMode = D3D11_FILL_WIREFRAME; 
	rd.CullMode = D3D11_CULL_BACK; 
	g_pDevice->CreateRasterizerState(&rd, &g_RState[RS_WIRECULLBACK]);

}




////////////////////////////////////////////////////////////////////////////// 
//
// 레스터 상태 객체 제거 : 어플리케이션 종료시 호출.
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
// 엔진-시스템 공통 데이터  갱신.. : 그려려니 합시다. 차차 공부할 것입니다.
// 
float EngineUpdate()
{
	//엔진 시간 / 타이머 얻기.
	float dTime = GetEngineTime();

	//Yena 갱신 : 불필요한 기능은 주석처리하여 기능을 비활성화 하십시오.
	//ynCameraUpdate(dTime);		// 카메라 갱신	
	//ynLightsUpdate(dTime);		// 조명 갱신						
	ynGridAxisUpdate(dTime);		// 방향축, 그리드 갱신.
	//YenaUpdate(dTime);			// 예나 시스템 전체 일괄 갱신.


	// 렌더링 옵션 조절 	 
	if (IsKeyUp(VK_SPACE))	g_bWireFrame ^= TRUE;
	if (IsKeyUp(VK_F4))		g_bCullBack ^= TRUE;
	if (IsKeyUp(VK_F5))		g_bZEnable ^= TRUE;		
	if (IsKeyUp(VK_F6))		g_bZWrite ^= TRUE;		


	// 배경색 설정.
	if (g_bWireFrame) g_ClearColor = COLOR(0.2f, 0.2f, 0.2f, 1.0f);
	else			  g_ClearColor = COLOR(0, 0.125f, 0.3f, 1.0f); 
	
	// 렌더링 모드 전환.	  
	RenderModeUpdate();
	 	
	// 깊이 연산 모드 전환.	 
	if (g_bZEnable)
	{
		if (g_bZWrite)  
			  g_pDXDC->OMSetDepthStencilState(g_DSState[DS_DEPTH_ON], 0);			//깊이 버퍼 동작 (기본값)
		else  g_pDXDC->OMSetDepthStencilState(g_DSState[DS_DEPTH_WRITE_OFF], 0);	//깊이 버퍼  : Z-Test On + Z-Write Off.
	}
	else  g_pDXDC->OMSetDepthStencilState(g_DSState[DS_DEPTH_OFF], 0);	//깊이 버퍼 비활성화 : Z-Test Off + Z-Write Off.
	

	return dTime;
}



/////////////////////////////////////////////////////////////////////////////// 
//
// 렌더링 모드 체크 : 각 렌더링 모드 상태별 On/Off 처리.
//
#define CheckRMode(k, v) if((k)) g_RMode |= (v); else g_RMode &= ~(v);


/////////////////////////////////////////////////////////////////////////////// 
//
// 렌더링 모드 전환
//
void RenderModeUpdate()
{	
	// 렌더링 모드 체크 : 사용자가 지정한 렌더링 상태 조합.
	CheckRMode(g_bCullBack, RM_CULLBACK);
	CheckRMode(g_bWireFrame, RM_WIREFRAME);


	// 레스터 모드 전환 : 지정된 모드의 조합을 통해 렌더링 상태를 조절.
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
//  도움말. 
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
		ynTextDraw(x, y, col, L"■ %s", g_WindowName);

		ynTextDraw(x, y += 14, col2, L"> 전체 렌더링 파이프라인의 흐름의 이해 ");
		ynTextDraw(x, y += 14, col,  L"> 정점 파이프라인 - 변환 Transform 의 이해");
		ynTextDraw(x, y += 14, col,  L"1. 월드 변환 (World Transform) 연습");
		ynTextDraw(x, y += 14, col,  L"2. 뷰 변환 (View Transform) 연습");
		ynTextDraw(x, y += 14, col,  L"3. 투영 변환 (Projection Transform) 연습"); 
		ynTextDraw(x, y += 14, col2, L"4.깊이 버퍼(Depth Buffer) 의 운용.");
		ynTextDraw(x, y += 14, col2, L"5.깊이 테스트(Depth Test) 의 이해.");			  
		ynTextDraw(x, y += 14, col*0.4f, L"6.W-Buffer 에 대하여 연구합니다.");
		ynTextDraw(x, y += 14, col2, L"7.뒷면제거(Back-Face Culling) 의 이해");			
		ynTextDraw(x, y += 14, col,  L"8.클립핑(Clipping) 의 이해와 연구");
		
		y = g_Mode.Height /12 * 8;
		ynTextDraw(x, y += 14, col2, L"> 기하도형 그리기"); 
		ynTextDraw(x, y += 14, col,  L"1. Box");
		ynTextDraw(x, y += 14, col2, L"2. Sphere");
		ynTextDraw(x, y += 14, col2, L"* 정점색은, 기본색(흰색) 으로 설정.");
		//y = g_Mode.Height - 200;
		//ynTextDraw(x, y += 14, col2, L" * 사용자 입력 처리 및 모델 변환 연습 * ");

	}



	//시스템 정보.
	{
		int x = 1, y = 14;
		COLOR col(0, 1, 0, 1);
		SystemInfo(x, y, col); 
	}

	//추가 도움말..
	int x = 1, y = 120;
	COLOR col(1, 1, 1, 1);
	COLOR col2(1, 1, 0, 1);
	y += 14;
	ynTextDraw(x, y += 14, col, L"채우기: Space (%s)", g_bWireFrame?L"WIRE":L"SOLID");
	ynTextDraw(x, y += 14, col, L"뒷면제거: F4 (%s)", g_bCullBack?L"ON":L"OFF");
	ynTextDraw(x, y += 14, col2, L"깊이테스트: F5 (%s)", g_bZEnable?L"ON":L"OFF");
	ynTextDraw(x, y += 14, g_bZEnable?col2:col2*0.5f, L"깊이쓰기: F6 (%s)", g_bZWrite?L"ON":L"OFF");

	//모델 정보 출력.
	y += 14;
	ynTextDraw(x, y += 14, col, L"[Model]");
	ynTextDraw(x, y += 14, col, L"리셋 : F9");
	ynTextDraw(x, y += 14, col, L"이동 : 방향키");
	ynTextDraw(x, y += 14, col, L"회전 : ADWS");
	ynTextDraw(x, y += 14, col, L"자동 : R");
	
	col = COLOR(0, 1, 1, 1);
	y = g_Mode.Height - 14 * 5;
	ynTextDraw(x, y += 14, col, L"[Model]");
	ynTextDraw(x, y += 14, col, L"Pos = { %.2f, %.2f, %.2f }", g_vPos.x, g_vPos.y, g_vPos.z);	
	ynTextDraw(x, y += 14, col, L"Rot = { %.2f, %.2f, %.2f }", g_vRot.x, g_vRot.y, g_vRot.z);	
	ynTextDraw(x, y += 14, col, L"Scl = { %.2f, %.2f, %.2f }", g_vScale.x, g_vScale.y, g_vScale.z);	
	 

	//카메라 정보 출력.
	x = 200;  y = g_Mode.Height - 14 * 5;
	//col = COLOR(1, 1, 0, 1)*0.8f;
	ynTextDraw(x, y+=14, col, L"[카메라]");  
	ynTextDraw(x, y+=14, col, L"Eye={%.2f, %.2f, %.2f}", g_vEye.x, g_vEye.y, g_vEye.z);  
	ynTextDraw(x, y+=14, col, L"Look={%.2f, %.2f, %.2f}", g_vLookAt.x, g_vLookAt.y, g_vLookAt.z);  
	ynTextDraw(x, y+=14, col, L"Up={%.2f, %.2f, %.2f}", g_vUp.x, g_vUp.y, g_vUp.z);  

	//카메라 - "렌즈" : 투영변환 정보 출력.
	x = 400;  y = g_Mode.Height - 14 * 5;
	//col = COLOR(1, 1, 0, 1);
	ynTextDraw(x, y+=14, col, L"[렌즈]");  
	ynTextDraw(x, y+=14, col, L"FOV=%.2f (%.2fº)", g_fFov, XMConvertToDegrees(g_fFov)); 
	ynTextDraw(x, y+=14, col, L"Aspect=%.2f", g_fAspect);  
	ynTextDraw(x, y+=14, col, L"Zn=%.1f  Zf = %.1f", g_fZnear, g_fZfar);

}

 


////////////////////////////////////////////////////////////////////////////// 
//
// 전체 장면 그리기
//
void SceneRender()
{
	//-------------------------------
	// 엔진/시스템 갱신.
	//------------------------------- 
	float dTime = EngineUpdate();

	// 장면 갱신 
	//
	CameraUpdate(dTime);	  //카메라갱신 : 뷰-투영 변환행렬 계산 
	
	// 주인공, 몬스터, 오브제 이동, 충돌검사 등등..
	// ...
	ObjUpdate(dTime);

	// 게임 로직, 이벤트, 층돌, 점수계산..
	// ...	 
	

	//-------------------------------
	// 장면 그리기 시작.. 
	//-------------------------------
 	//렌더타겟(백버퍼)/ 깊이버퍼 지우기..
	ClearBackBuffer(D3D11_CLEAR_DEPTH, g_ClearColor);
		
	//시스템 객체 출력.
	//ynGridAxisDraw();

	//개체 렌더링 : 주인공, 몬스터, 지형.. 
  	ObjDraw();
	DummyDraw();		//더미 그리기 : +Z 축 방향 


	//시스템 객체 & 도움말 출력..
	ynGridAxisDraw();
	ShowInfo();

	//-------------------------------
	// 장면 그리기 종료.
	//------------------------------- 
	Flip();
	

}//end of void SceneRender()






/****************** end of file "Render.cpp" *********************************/