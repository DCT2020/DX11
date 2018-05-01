//
// Shader.h : 기본 셰이더 프레임웍 헤더
//
// DirectX 기준 버전 : DX11
//
// 2003.11.4. Kihong Kim / mad_dog@hanmail.net
// 2010.12.01. Update. (DX11.Jun.2010)
// 2016.12.27. Update. (DX11. Windows SDK 8.1)
//

#pragma once

 
HRESULT ShaderLoad();
void	ShaderRelease();
HRESULT ShaderCompile(
						WCHAR* FileName,		//소스파일이름.
						char* EntryPoint,		//메인함수 진입점.
						char* ShaderModel,		//셰이더 모델.
						ID3DBlob** ppCode		//[출력] 컴파일된 셰이더 코드.
					 );
void ShaderCreate();
void ShaderUpdate();



//셰이더 관련.
extern ID3D11VertexShader*     g_pVS;
extern ID3D11PixelShader*      g_pPS;

//정점 셰이더 컴파일 코드 개체.(임시)
extern ID3DBlob* g_pVSCode;

//행렬 접근 열거상수.
//구형 (DX9) 매크로 (D3DTS_WORLD 등..) 와 유사한 상수정의를 통해
//전통적인 행렬운용방식의 연속성울 이어가겠습니다.
//본격적인 상수버퍼 운용은 다음 튜토리얼에서 처리할 것입니다.
enum {
	TS_WORLD,		//D3DTS_WORLD 대응.
	TS_VIEW,		//D3DTS_VIEW 대응.
	TS_PROJ,		//D3DTS_PROJ 대응.
	TS_MAX
};


// 상수 버퍼용 구조체 : 셰이더 내부 연산에 사용될 데이터들. 
// GPU 로 데이터를 전송할 사용자 데이터를 정의합니다.  
struct ConstBuffer
{
	//XMMATRIX mTM;		//"World" 변환 행렬 : DirectXMath, 16바이트 정렬 버전. 
	//XMMATRIX mView;	//"View" 변환 행렬
	//XMMATRIX mProj;	//"Projection" 변환 행렬

	//변환행렬.
	XMMATRIX mTM[TS_MAX];	//DirectXMath, 16바이트 정렬 버전. 
	

	/*//행렬 설정 메소드.
	void SetTM(XMMATRIX* pmWorld);
	void SetView(XMMATRIX* pmView);
	void SetProj(XMMATRIX* pmProj);*/
};

// 상수버퍼용 데이터
//extern ConstBuffer g_CBuffer;

//행렬 설정 메소드.
void SetTransform(DWORD type, XMMATRIX* pTM);
	

//셰이더 상수 버퍼.인터페이스.
extern ID3D11Buffer*	g_pCB;

//(정적) 상수 버퍼 생성.
HRESULT CreateConstantBuffer(UINT size, ID3D11Buffer** ppCB);
//동적 상수버퍼 생성.
HRESULT CreateDynamicConstantBuffer(UINT size, LPVOID pData, ID3D11Buffer** ppCB);
//동적 상수버퍼 갱신.
HRESULT UpdateDynamicConstantBuffer(LPDXDC pDXDC, ID3D11Resource* pBuff, LPVOID pData, UINT size);

/****************** end of file "Shader.h" ***********************************/