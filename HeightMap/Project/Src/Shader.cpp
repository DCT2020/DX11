//
// Shader.cpp : �⺻ ������/�����ӿ� �ҽ�
//
// DirectX ���� ���� : DX11
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

#include "d3dcompiler.h"				//DX ���̴� �����Ϸ� ���.
#pragma comment(lib, "d3dcompiler")		//DX ���̴� �����Ϸ� ���̺귯��.  D3DCompiler.dll �ʿ�.
#include "Shader.h"


/////////////////////////////////////////////////////////////////////////////
//
//  ���� �����͵�.
//  
extern LPDXDC g_pDXDC;


//���̴� �� ��ü ����.
ID3D11VertexShader*     g_pVS = nullptr;		//���� ���̴� Vertex Shader �������̽�.
ID3D11PixelShader*      g_pPS = nullptr;		//�ȼ� ���̴� Pixel Shader �������̽�.

//���� ���̴� ������ �ڵ� ��ü.(�ӽ�)
ID3DBlob* g_pVSCode = nullptr;

//���̴� ��� ����.
ID3D11Buffer*	g_pCB = nullptr;

// ������ۿ� ������
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
// ���̴� �ε�. 
//
HRESULT ShaderLoad()
{	 
	WCHAR* filename = L"./fx/Demo.fx";
	

	//--------------------------
	// ���� ���̴� ����.
	//--------------------------
	// ���� ���̴� ������ Compile a VertexShader
	ID3DBlob* pVSCode = nullptr;
	HRESULT hr = ShaderCompile(filename, "VS_Main", "vs_5_0", &pVSCode);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"[����] ShaderLoad :: Vertex Shader ������ ����", L"Error", MB_OK | MB_ICONERROR);
		return hr;
	}
	// ���� ���̴� ��ü ���� Create a VS Object 
	hr = g_pDevice->CreateVertexShader(	pVSCode->GetBufferPointer(), 
									    pVSCode->GetBufferSize(), 
									    nullptr, 
	  								    &g_pVS	    
			                            );
	if (FAILED(hr))
	{
		SAFE_RELEASE(pVSCode);			//�ӽ� ��ü ����.
		return hr;
	}

	//���� ���� : : Input Layout ���۽� �ʿ���.
	g_pVSCode = pVSCode;


	//--------------------------
	// �ȼ� ���̴� ����.
	//--------------------------
	// �ȼ� ���̴� ������ Compile a PixelShader
	ID3DBlob* pPSCode = nullptr;
	hr = ShaderCompile( filename, "PS_Main", "ps_5_0", &pPSCode);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"[����] ShaderLoad :: Pixel Shader ������ ����", L"Error", MB_OK | MB_ICONERROR);
		return hr;
	}
	// �ȼ� ���̴� ��ü ���� Create a PS Object 
	hr = g_pDevice->CreatePixelShader(	pPSCode->GetBufferPointer(), 
										pPSCode->GetBufferSize(), 
										nullptr,
										&g_pPS
									 );

	SAFE_RELEASE(pPSCode);				//�ӽ� ��ü ����.	
	if (FAILED(hr))	return hr;

	return hr;

}




////////////////////////////////////////////////////////////////////////////// 
//
// ���̴� ����. 
//
void ShaderRelease()
{
	SAFE_RELEASE(g_pVS);			//���� ���̴� ����.
	SAFE_RELEASE(g_pPS);			//�ȼ� ���̴� ����.
	SAFE_RELEASE(g_pVSCode);		//���� ���̴� ������ �ڵ� (����) ����.
	SAFE_RELEASE(g_pCB);			//������� ����.
}





////////////////////////////////////////////////////////////////////////////// 
//
// ���̴� �ҽ� ������ : ���̴� �ҽ�(*.fx)�� GPU �� ����� ��ȯ�մϴ�. 
//
HRESULT ShaderCompile(
						WCHAR* FileName,		//�ҽ������̸�.
						char* EntryPoint,		//�����Լ� ������.
						char* ShaderModel,		//���̴� ��.
						ID3DBlob** ppCode		//[���] �����ϵ� ���̴� �ڵ�.
						)
{
	HRESULT hr = S_OK; 
	ID3DBlob* pError = nullptr;

	//������ �ɼ�1.
	UINT Flags = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;		//���켱 ��� ó��. ���� DX9 ���������� �������� ���. �ӵ��� �䱸�ȴٸ�, "��켱" ���� ó���� ��.
	//UINT Flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;	//��켱 ��� ó��. �ӵ��� ����� ������, ����� ��ġ �� GPU �� �����ؾ� �մϴ�.
	//UINT Flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#ifdef _DEBUG
	Flags |=  D3DCOMPILE_DEBUG;							//����� ���� �ɼ� �߰�.
#endif

	//���̴� �ҽ� ������.
	hr = D3DCompileFromFile(FileName, 
							nullptr, nullptr, 
							EntryPoint, 
							ShaderModel,
							Flags,				//������ �ɼ�1
							0,					//������ �ɼ�2,  Effect ���� �����Ͻ� �����. �̿ܿ��� ���õ�.
							ppCode,				//[���] �����ϵ� ���̴� �ڵ�.
							&pError				//[���] ������ ���� �ڵ�.
							);
	if (FAILED(hr))
	{
		//������ ����Ȯ���� ���� pError �� ����մϴ�.
		ynErrorW(TRUE, L"���̴� ������ ����", hr, pError, FileName, EntryPoint, ShaderModel );
	}
	
	SAFE_RELEASE(pError);
	return hr;
}

 



////////////////////////////////////////////////////////////////////////////// 
//
// (����) ��� ���� ����
//
HRESULT CreateConstantBuffer( UINT size, ID3D11Buffer** ppCB ) 
{	 
	HRESULT hr = S_OK;
	ID3D11Buffer* pCB = nullptr;

	//��� ���� ���� ����.
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size;  
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;


	//��� ���� ����.
	hr = g_pDevice->CreateBuffer(&bd, nullptr, &pCB);		
	if (FAILED(hr))
	{
		ynError(hr, L"CreateConstantBuffer : ������� ���� ����");		 
		return hr;
	}

	//�ܺη� ����.
	*ppCB = pCB;
	
	return hr;
}





////////////////////////////////////////////////////////////////////////////// 
//
// (����) ��� ���� ����
//
HRESULT CreateDynamicConstantBuffer(UINT size, LPVOID pData, ID3D11Buffer** ppCB)
{
	HRESULT hr = S_OK;
	ID3D11Buffer* pCB = nullptr;

	//��� ���� ���� ����.
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;				//���� �������� ����.
	bd.ByteWidth = size;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//CPU ���� ����.

	//���긮�ҽ� ����.
	D3D11_SUBRESOURCE_DATA sd;
	sd.pSysMem = pData;				//(�ܺ�) ��� ������ ����.
	sd.SysMemPitch = 0;
	sd.SysMemSlicePitch = 0;

	//��� ���� ����.
	hr = g_pDevice->CreateBuffer(&bd, &sd, &pCB);
	if (FAILED(hr))
	{
		ynError(hr, L"CreateDynamicConstantBuffer : ���� ������� ���� ����");
		return hr;
	}

	//�ܺη� ����.
	*ppCB = pCB;

	return hr;
}




////////////////////////////////////////////////////////////////////////////// 
//
// ���� ���� ����.
//
HRESULT UpdateDynamicConstantBuffer(LPDXDC pDXDC, ID3D11Resource* pBuff, LPVOID pData, UINT size )
{
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE mr;
	ZeroMemory(&mr, sizeof(mr));
	
	//������� ����
	hr = pDXDC->Map( pBuff,	0, D3D11_MAP_WRITE_DISCARD,	0, &mr);
	if (FAILED(hr))
	{
		ynError(hr, L"UpdateDynamicConstantBuffer : Map ����");
		return hr;
	}

	//��� ���� ����.
	memcpy(mr.pData, pData, size);

	//������� �ݱ�.
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
// ��� ���� �������̽� ����.
//
////////////////////////////////////////////////////////////////////////////// 

//���̴� ��� ���� ����
#define UpdateCB() \
	UpdateDynamicConstantBuffer(g_pDXDC, g_pCB, &g_CBuffer, sizeof(ConstBuffer));



////////////////////////////////////////////////////////////////////////////// 
//
void SetTransform(DWORD type, XMMATRIX* pTM)
{
	g_CBuffer.mTM[type] = *pTM;
	
	//���̴� �� ������� ���� 
	//�� ��� �����ø��� �����ϴ� ���� ��ȿ����������
	//������ �������� ���� �̰����� ó���ϰڽ��ϴ�.
	ShaderUpdate();
	//UpdateCB();
}



/*
////////////////////////////////////////////////////////////////////////////// 
//
// ���� ��ȯ ��� ����.
//
void ConstBuffer::SetTM(XMMATRIX* pmWorld)
{
	mTM[TS_WORLD] = *pmWorld;

	UpdateCB();
}


////////////////////////////////////////////////////////////////////////////// 
//
// �� ��ȯ��� ����.
//
void ConstBuffer::SetView(XMMATRIX* pmView)
{
	mTM[TS_VIEW] = *pmView;

	UpdateCB();
}



////////////////////////////////////////////////////////////////////////////// 
//
// ���� ��ȯ ��� ����
//
void ConstBuffer::SetProj(XMMATRIX* pmProj)
{
	mTM[TS_PROJ] = *pmProj;

	UpdateCB();
}
*/



////////////////////////////////////////////////////////////////////////////// 
//
// �⺻ ���̴� �ý��� ����.
//
void ShaderCreate()
{
	// �⺻ ���̴� �ε� & ����.
	ShaderLoad();

	// ��� ���� ����.
	ZeroMemory(&g_CBuffer, sizeof(ConstBuffer)); 
	CreateDynamicConstantBuffer(sizeof(ConstBuffer), &g_CBuffer, &g_pCB); 
		
	//���̴� ��� ���� ����.(��������)
	//UpdateDynamicConstantBuffer(g_pDXDC, g_pCB, &g_CBuffer, sizeof(ConstBuffer));
	
	//���̴� ����.
	g_pDXDC->VSSetShader(g_pVS, nullptr, 0);
	g_pDXDC->PSSetShader(g_pPS, nullptr, 0);
	//���̴� ������� ����.
	g_pDXDC->VSSetConstantBuffers(0, 1, &g_pCB);

}




////////////////////////////////////////////////////////////////////////////// 
//
// ���̴� �� ������� ����
//
void ShaderUpdate()
{
	//���̴� ����.
	g_pDXDC->VSSetShader(g_pVS, nullptr, 0);
	g_pDXDC->PSSetShader(g_pPS, nullptr, 0);

	//���̴� ��� ���� ����.(��������)
	UpdateDynamicConstantBuffer(g_pDXDC, g_pCB, &g_CBuffer, sizeof(ConstBuffer));
	//�������̴��� ��� ���� ����.
	g_pDXDC->VSSetConstantBuffers(0, 1, &g_pCB);	

}



	


/****************** end of file "Shader.cpp" *********************************/



