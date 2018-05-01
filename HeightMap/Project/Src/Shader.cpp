//
// Shader.cpp : 기본 렌더링/프레임웍 소스
//
// DirectX 기준 버전 : DX11
//
// 2003.11.4. Kihong Kim / mad_dog@hanmail.net
// 2010.12.01. Update. (DX11.Jun.2010)
// 2016.12.27. Update. (DX11. Windows SDK 8.1)
//
#pragma warning(disable:4996)
#include "Windows.h" 
#include "stdio.h"
#include "Device.h"
#include "Render.h"

#include "d3dcompiler.h"				//DX 셰이더 컴파일러 헤더.
#pragma comment(lib, "d3dcompiler")		//DX 셰이더 컴파일러 라이브러리.  D3DCompiler.dll 필요.
#include "Shader.h"


/////////////////////////////////////////////////////////////////////////////
//
//  전역 데이터들.
//  
extern LPDXDC g_pDXDC;


//셰이더 별 객체 선언.
ID3D11VertexShader*     g_pVS = nullptr;		//정점 셰이더 Vertex Shader 인터페이스.
ID3D11PixelShader*      g_pPS = nullptr;		//픽셀 셰이더 Pixel Shader 인터페이스.

//정점 셰이더 컴파일 코드 개체.(임시)
ID3DBlob* g_pVSCode = nullptr;

//셰이더 상수 버퍼.
ID3D11Buffer*	g_pCB = nullptr;

// 상수버퍼용 데이터
ConstBuffer g_CBuffer;



//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//
// Shader Functions
//
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 


////////////////////////////////////////////////////////////////////////////// 
//
// 셰이더 로드. 
//
HRESULT ShaderLoad()
{	 
	WCHAR* filename = L"./fx/Demo.fx";
	

	//--------------------------
	// 정점 셰이더 생성.
	//--------------------------
	// 정점 셰이더 컴파일 Compile a VertexShader
	ID3DBlob* pVSCode = nullptr;
	HRESULT hr = ShaderCompile(filename, "VS_Main", "vs_5_0", &pVSCode);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"[실패] ShaderLoad :: Vertex Shader 컴파일 실패", L"Error", MB_OK | MB_ICONERROR);
		return hr;
	}
	// 정점 셰이더 객체 생성 Create a VS Object 
	hr = g_pDevice->CreateVertexShader(	pVSCode->GetBufferPointer(), 
									    pVSCode->GetBufferSize(), 
									    nullptr, 
	  								    &g_pVS	    
			                            );
	if (FAILED(hr))
	{
		SAFE_RELEASE(pVSCode);			//임시 개체 제거.
		return hr;
	}

	//전역 참조 : : Input Layout 제작시 필요함.
	g_pVSCode = pVSCode;


	//--------------------------
	// 픽셀 셰이더 생성.
	//--------------------------
	// 픽셀 셰이더 컴파일 Compile a PixelShader
	ID3DBlob* pPSCode = nullptr;
	hr = ShaderCompile( filename, "PS_Main", "ps_5_0", &pPSCode);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"[실패] ShaderLoad :: Pixel Shader 컴파일 실패", L"Error", MB_OK | MB_ICONERROR);
		return hr;
	}
	// 픽셀 셰이더 객체 생성 Create a PS Object 
	hr = g_pDevice->CreatePixelShader(	pPSCode->GetBufferPointer(), 
										pPSCode->GetBufferSize(), 
										nullptr,
										&g_pPS
									 );

	SAFE_RELEASE(pPSCode);				//임시 개체 제거.	
	if (FAILED(hr))	return hr;

	return hr;

}




////////////////////////////////////////////////////////////////////////////// 
//
// 셰이더 제거. 
//
void ShaderRelease()
{
	SAFE_RELEASE(g_pVS);			//정점 셰이더 제거.
	SAFE_RELEASE(g_pPS);			//픽셀 셰이더 제거.
	SAFE_RELEASE(g_pVSCode);		//정점 셰이더 컴파일 코드 (버퍼) 제거.
	SAFE_RELEASE(g_pCB);			//상수버퍼 제거.
}





////////////////////////////////////////////////////////////////////////////// 
//
// 셰이더 소스 컴파일 : 셰이더 소스(*.fx)를 GPU 용 기계어로 변환합니다. 
//
HRESULT ShaderCompile(
						WCHAR* FileName,		//소스파일이름.
						char* EntryPoint,		//메인함수 진입점.
						char* ShaderModel,		//셰이더 모델.
						ID3DBlob** ppCode		//[출력] 컴파일된 셰이더 코드.
						)
{
	HRESULT hr = S_OK; 
	ID3DBlob* pError = nullptr;

	//컴파일 옵션1.
	UINT Flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;		//열우선 행렬 처리. 구형 DX9 이전까지의 전통적인 방식. 속도가 요구된다면, "행우선" 으로 처리할 것.
	//UINT Flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;	//행우선 행렬 처리. 속도의 향상이 있지만, 행렬을 전치 후 GPU 에 공급해야 합니다.
	//UINT Flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#ifdef _DEBUG
	Flags |=  D3DCOMPILE_DEBUG;							//디버깅 모드시 옵션 추가.
#endif

	//셰이더 소스 컴파일.
	hr = D3DCompileFromFile(FileName, 
							nullptr, nullptr, 
							EntryPoint, 
							ShaderModel,
							Flags,				//컴파일 옵션1
							0,					//컴파일 옵션2,  Effect 파일 컴파일시 적용됨. 이외에는 무시됨.
							ppCode,				//[출력] 컴파일된 셰이더 코드.
							&pError				//[출력] 컴파일 에러 코드.
							);
	if (FAILED(hr))
	{
		//컴파일 에러확인을 위해 pError 를 출력합니다.
		ynErrorW(TRUE, L"셰이더 컴파일 실패", hr, pError, FileName, EntryPoint, ShaderModel );
	}
	
	SAFE_RELEASE(pError);
	return hr;
}

 



////////////////////////////////////////////////////////////////////////////// 
//
// (정적) 상수 버퍼 생성
//
HRESULT CreateConstantBuffer( UINT size, ID3D11Buffer** ppCB ) 
{	 
	HRESULT hr = S_OK;
	ID3D11Buffer* pCB = nullptr;

	//상수 버퍼 정보 설정.
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size;  
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;


	//상수 버퍼 생성.
	hr = g_pDevice->CreateBuffer(&bd, nullptr, &pCB);		
	if (FAILED(hr))
	{
		ynError(hr, L"CreateConstantBuffer : 상수버퍼 생성 실패");		 
		return hr;
	}

	//외부로 전달.
	*ppCB = pCB;
	
	return hr;
}





////////////////////////////////////////////////////////////////////////////// 
//
// (동적) 상수 버퍼 생성
//
HRESULT CreateDynamicConstantBuffer(UINT size, LPVOID pData, ID3D11Buffer** ppCB)
{
	HRESULT hr = S_OK;
	ID3D11Buffer* pCB = nullptr;

	//상수 버퍼 정보 설정.
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;				//동적 정점버퍼 설정.
	bd.ByteWidth = size;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//CPU 접근 설정.

	//서브리소스 설정.
	D3D11_SUBRESOURCE_DATA sd;
	sd.pSysMem = pData;				//(외부) 상수 데이터 설정.
	sd.SysMemPitch = 0;
	sd.SysMemSlicePitch = 0;

	//상수 버퍼 생성.
	hr = g_pDevice->CreateBuffer(&bd, &sd, &pCB);
	if (FAILED(hr))
	{
		ynError(hr, L"CreateDynamicConstantBuffer : 동적 상수버퍼 생성 실패");
		return hr;
	}

	//외부로 전달.
	*ppCB = pCB;

	return hr;
}




////////////////////////////////////////////////////////////////////////////// 
//
// 동적 버퍼 갱신.
//
HRESULT UpdateDynamicConstantBuffer(LPDXDC pDXDC, ID3D11Resource* pBuff, LPVOID pData, UINT size )
{
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE mr;
	ZeroMemory(&mr, sizeof(mr));
	
	//상수버퍼 접근
	hr = pDXDC->Map( pBuff,	0, D3D11_MAP_WRITE_DISCARD,	0, &mr);
	if (FAILED(hr))
	{
		ynError(hr, L"UpdateDynamicConstantBuffer : Map 실패");
		return hr;
	}

	//상수 버퍼 갱신.
	memcpy(mr.pData, pData, size);

	//상수버퍼 닫기.
	pDXDC->Unmap(pBuff, 0);
	  

	return hr;
}






////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
//
// 상수 버퍼 인터페이스 정의.
//
////////////////////////////////////////////////////////////////////////////// 

//셰이더 상수 버퍼 갱신
#define UpdateCB() \
	UpdateDynamicConstantBuffer(g_pDXDC, g_pCB, &g_CBuffer, sizeof(ConstBuffer));



////////////////////////////////////////////////////////////////////////////// 
//
void SetTransform(DWORD type, XMMATRIX* pTM)
{
	g_CBuffer.mTM[type] = *pTM;
	
	//셰이더 및 상수버퍼 갱신 
	//각 행렬 설정시마다 갱신하는 것은 비효율적이지만
	//예제의 간결함을 위해 이곳에서 처리하겠습니다.
	ShaderUpdate();
	//UpdateCB();
}



/*
////////////////////////////////////////////////////////////////////////////// 
//
// 월드 변환 행렬 설정.
//
void ConstBuffer::SetTM(XMMATRIX* pmWorld)
{
	mTM[TS_WORLD] = *pmWorld;

	UpdateCB();
}


////////////////////////////////////////////////////////////////////////////// 
//
// 뷰 변환행렬 설정.
//
void ConstBuffer::SetView(XMMATRIX* pmView)
{
	mTM[TS_VIEW] = *pmView;

	UpdateCB();
}



////////////////////////////////////////////////////////////////////////////// 
//
// 투영 변환 행렬 설정
//
void ConstBuffer::SetProj(XMMATRIX* pmProj)
{
	mTM[TS_PROJ] = *pmProj;

	UpdateCB();
}
*/



////////////////////////////////////////////////////////////////////////////// 
//
// 기본 셰이더 시스템 구성.
//
void ShaderCreate()
{
	// 기본 셰이더 로드 & 설정.
	ShaderLoad();

	// 상수 버퍼 생성.
	ZeroMemory(&g_CBuffer, sizeof(ConstBuffer)); 
	CreateDynamicConstantBuffer(sizeof(ConstBuffer), &g_CBuffer, &g_pCB); 
		
	//셰이더 상수 버퍼 갱신.(동적버퍼)
	//UpdateDynamicConstantBuffer(g_pDXDC, g_pCB, &g_CBuffer, sizeof(ConstBuffer));
	
	//셰이더 설정.
	g_pDXDC->VSSetShader(g_pVS, nullptr, 0);
	g_pDXDC->PSSetShader(g_pPS, nullptr, 0);
	//셰이더 상수버퍼 설정.
	g_pDXDC->VSSetConstantBuffers(0, 1, &g_pCB);

}




////////////////////////////////////////////////////////////////////////////// 
//
// 셰이더 및 상수버퍼 갱신
//
void ShaderUpdate()
{
	//셰이더 설정.
	g_pDXDC->VSSetShader(g_pVS, nullptr, 0);
	g_pDXDC->PSSetShader(g_pPS, nullptr, 0);

	//셰이더 상수 버퍼 갱신.(동적버퍼)
	UpdateDynamicConstantBuffer(g_pDXDC, g_pCB, &g_CBuffer, sizeof(ConstBuffer));
	//정점셰이더에 상수 버퍼 설정.
	g_pDXDC->VSSetConstantBuffers(0, 1, &g_pCB);	

}



	


/****************** end of file "Shader.cpp" *********************************/



