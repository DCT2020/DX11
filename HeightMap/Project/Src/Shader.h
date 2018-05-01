//
// Shader.h : �⺻ ���̴� �����ӿ� ���
//
// DirectX ���� ���� : DX11
//
// 2003.11.4. Kihong Kim / mad_dog@hanmail.net
// 2010.12.01. Update. (DX11.Jun.2010)
// 2016.12.27. Update. (DX11. Windows SDK 8.1)
//

#pragma once

 
HRESULT ShaderLoad();
void	ShaderRelease();
HRESULT ShaderCompile(
						WCHAR* FileName,		//�ҽ������̸�.
						char* EntryPoint,		//�����Լ� ������.
						char* ShaderModel,		//���̴� ��.
						ID3DBlob** ppCode		//[���] �����ϵ� ���̴� �ڵ�.
					 );
void ShaderCreate();
void ShaderUpdate();



//���̴� ����.
extern ID3D11VertexShader*     g_pVS;
extern ID3D11PixelShader*      g_pPS;

//���� ���̴� ������ �ڵ� ��ü.(�ӽ�)
extern ID3DBlob* g_pVSCode;

//��� ���� ���Ż��.
//���� (DX9) ��ũ�� (D3DTS_WORLD ��..) �� ������ ������Ǹ� ����
//�������� ��Ŀ������ ���Ӽ��� �̾�ڽ��ϴ�.
//�������� ������� ����� ���� Ʃ�丮�󿡼� ó���� ���Դϴ�.
enum {
	TS_WORLD,		//D3DTS_WORLD ����.
	TS_VIEW,		//D3DTS_VIEW ����.
	TS_PROJ,		//D3DTS_PROJ ����.
	TS_MAX
};


// ��� ���ۿ� ����ü : ���̴� ���� ���꿡 ���� �����͵�. 
// GPU �� �����͸� ������ ����� �����͸� �����մϴ�.  
struct ConstBuffer
{
	//XMMATRIX mTM;		//"World" ��ȯ ��� : DirectXMath, 16����Ʈ ���� ����. 
	//XMMATRIX mView;	//"View" ��ȯ ���
	//XMMATRIX mProj;	//"Projection" ��ȯ ���

	//��ȯ���.
	XMMATRIX mTM[TS_MAX];	//DirectXMath, 16����Ʈ ���� ����. 
	

	/*//��� ���� �޼ҵ�.
	void SetTM(XMMATRIX* pmWorld);
	void SetView(XMMATRIX* pmView);
	void SetProj(XMMATRIX* pmProj);*/
};

// ������ۿ� ������
//extern ConstBuffer g_CBuffer;

//��� ���� �޼ҵ�.
void SetTransform(DWORD type, XMMATRIX* pTM);
	

//���̴� ��� ����.�������̽�.
extern ID3D11Buffer*	g_pCB;

//(����) ��� ���� ����.
HRESULT CreateConstantBuffer(UINT size, ID3D11Buffer** ppCB);
//���� ������� ����.
HRESULT CreateDynamicConstantBuffer(UINT size, LPVOID pData, ID3D11Buffer** ppCB);
//���� ������� ����.
HRESULT UpdateDynamicConstantBuffer(LPDXDC pDXDC, ID3D11Resource* pBuff, LPVOID pData, UINT size);

/****************** end of file "Shader.h" ***********************************/