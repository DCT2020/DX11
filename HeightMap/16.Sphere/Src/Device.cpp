//
// Device.cpp : D3D ��ü �� ����̽� ����/���� �ҽ�.
//
// DirectX ���� ���� : DX11
//
// 2003.11.4. Kihong Kim / mad_dog@hanmail.net
// 2010.12.01. Update. (DX11.Jun.2010)
// 2016.12.27. Update. (DX11. Windows SDK 8.1)
//
#pragma warning(disable:4996)
#include "Device.h"
#include "Tchar.h"
#include "Shader.h"


///////////////////////////////////////////////////////////////////////////////
//
// DX �� ��ġ ���� ���� �����͵�  
//
///////////////////////////////////////////////////////////////////////////////

// D3D ���� ��ü �������̽� ������.
ID3D11Device*           g_pDevice = NULL;
ID3D11DeviceContext*	g_pDXDC = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRTView = NULL;

//���� ���ٽ� ���� ����.
ID3D11Texture2D*		 g_pDS = NULL;			//����-���ٽ� ����.
ID3D11DepthStencilView*  g_pDSView = NULL;		//����-���ٽ� ��.



//��ġ ���� �⺻ ����.
DISPLAYMODE g_Mode = { 960, 600, 0, 1, DXGI_FORMAT_R8G8B8A8_UNORM };

//D3D ��� ���� (Direct3D feature level) 
//���� ����� DX ���� ����. DX ������ ��ġ�� ȣȯ�� ���
D3D_FEATURE_LEVEL g_FeatureLevels = D3D_FEATURE_LEVEL_11_0;		//DX11 ����.
TCHAR* g_strFeatureLevel = L"N/A";

//��ġ ����
DXGI_ADAPTER_DESC1 g_Adc;



//��üȭ�� ��뿩��.
BOOL g_bWindowMode = TRUE;
//��������ȭ ��뿩��.
BOOL g_bVSync = FALSE;


// �ʱ� ��ġ ���� ���� �Լ���.
//
HRESULT CreateDeviceSwapChain(HWND hwnd);
HRESULT CreateRenderTarget();
HRESULT CreateDepthStencil();
void	SetViewPort();



///////////////////////////////////////////////////////////////////////////////
//
// ����/���ٽ� ���� ��ü ���� 
//
///////////////////////////////////////////////////////////////////////////////

//����/���ٽ� ���� ��ü�� : ���� ��ü ������.
ID3D11DepthStencilState* g_DSState[DS_MAX_];

int  StateObjectCreate();
void StateObjectRelease();




 

///////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////// 

///////////////////////////////////////////////////////////////////////////// 
//
// DX �� ���� ��ü �ʱ�ȭ : ���ø����̼� ���۽� 1ȸ ȣ��.
//
// 1.����̽� ���� 
// 2.����ü�� �� ����Ÿ�� ����.
// 3.����/���ٽ� ���� ����.
// 4.��Ÿ ������ ���� ��ġ ����.
//
int DXSetup(HWND hwnd)
{
	
	//----------------------------------------
	// 1�ܰ� : ������ ��ġ ���� �ʼ� ����
	//----------------------------------------
	// D3D ������ ��ġ Device �� ����ü�� Swap Chain ����. 
	CreateDeviceSwapChain(hwnd);

	// ��ġ-����ü���� ����Ÿ��(�����) ȹ��
	CreateRenderTarget();
	
	// ����/���ٽ� ���� ����.
	CreateDepthStencil();
	
	// ��ġ ��º��ձ�(Output Merger) �� ���͸� Ÿ�� �� ����-���ٽ� ���� ���.
	g_pDXDC->OMSetRenderTargets(
				1,				// ����Ÿ�� ����.(max: D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT)
				&g_pRTView,		// ����Ÿ��("�����") ���.	
				g_pDSView		// ����/���ٽ� ���� ���.
				);
	
	// ����Ʈ ����.
	SetViewPort();


	//��ġ ���� ȹ��
	GetDeviceInfo();


	//----------------------------------------
	// 2�ܰ� : ��Ÿ ������ ���� �߰� ����.
	//----------------------------------------
	// �������� �ʿ��� ����� ��ü���� ����/���� �մϴ�.
	// ī�޶�, ����, ��Ʈ, ���̴� ���...
	
	//����-���ٽ� ������ ���� ��ü ����.
	StateObjectCreate();


	// ��Ʈ ����.--> Yena �� ���Ե�.
	//ynFontCreate(g_pDevice);
	
	// �⺻ ���̴� �ε� & ����.
	ShaderCreate();
	
	 
	// Yena �¾� : �ý��� ���� ��ü ���� 
	// ���� �⺻ ī�޶�, ����, ��Ʈ, ������, �׸���, ���..
	YenaSetup(g_pDevice, g_Mode, 5.0f, 100, 2.5f);
	
	 
	//�۾� �Ϸ�, ��ġ �غ� �Ϸ�.
	return YN_OK;
}




///////////////////////////////////////////////////////////////////////////// 
//
// ����̽� ��ü �� ���� ��ġ ���� : ����� 1ȸ ȣ��.  
// �⺻ ������ ��ü�� ���Ž� ������ �������� ó���մϴ�.(����)
//
void DXRelease()
{ 	 
	//��ġ ���� ���� : ���� ���� �ʱ�ȭ�� �ؾ� �մϴ�. (�޸� ���� ����)
	if (g_pDXDC) g_pDXDC->ClearState();
	//if (g_pSwapChain) g_pSwapChain->SetFullscreenState(false, NULL);

	//���� ����.
	YenaRelease();
 
	//ynFontRelease();					//��Ʈ ����.--> Yena �� ���Ե�.

	ShaderRelease();					//���̴� ����.

	//���� ��ü ����.
	StateObjectRelease();


	SAFE_RELEASE(g_pDS);				//����/���ٽ� ���� ����.
	SAFE_RELEASE(g_pDSView);			
	SAFE_RELEASE(g_pRTView);			//����Ÿ�� ����.
	SAFE_RELEASE(g_pSwapChain);			//����ü�� ����.
	SAFE_RELEASE(g_pDXDC);
	SAFE_RELEASE(g_pDevice);			//����̽� ����. �� ���߿� �����մϴ�.
}
 




/////////////////////////////////////////////////////////////////////////////
//
// D3D ������ '��ġ(Device)' �� ����ü�� ����. 
//
HRESULT CreateDeviceSwapChain(HWND hwnd)
{
	HRESULT hr = S_OK;

	// ��ġ (Device) �� ����ü��(SwapChain) ���� ����.
	// ����ü���� ���߹��۸� �ý����� ���ϸ�
	// �������� '�ø���Flipping' ü�ΰ� ������ �ǹ��Դϴ�.  
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Windowed = g_bWindowMode;				//Ǯ��ũ�� �Ǵ� â��� ����.
	sd.OutputWindow = hwnd;						//����� ������ �ڵ�.
	sd.BufferCount = 1;							//����� ����.
	sd.BufferDesc.Width = g_Mode.Width;			//�ػ� ����.(����� ũ��)
	sd.BufferDesc.Height = g_Mode.Height;
	sd.BufferDesc.Format = g_Mode.Format;		//����� ����԰� (A8R8G8B8) â��忡���� ���� ���� 
	sd.BufferDesc.RefreshRate.Numerator = g_bVSync ? 60 : 0;   //���� ������.(��������ȭ VSync Ȱ��ȭ�� ǥ�ذ����� ���� : 60hz)
	//sd.BufferDesc.RefreshRate.Numerator = 0;	//���� ������.(��������ȭ VSync Off)
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//�뵵 ����: '����Ÿ��' 
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	//sd.Flags = 0;
	sd.SampleDesc.Count = 1;					//AA ����
	sd.SampleDesc.Quality = 0;
	
	
	
	// D3D ������ '��ġ(Device)' �� ����ü�� ����. 
	//
	hr = D3D11CreateDeviceAndSwapChain(
			NULL,						//���� ��� ������ (�⺻��ġ�� NULL)
			D3D_DRIVER_TYPE_HARDWARE,	//HW ���� 
			NULL,						//SW Rasterizer DLL �ڵ�.  HW ���ӽÿ��� NULL.
			0,							//����̽� ���� �÷���.(�⺻��)
			//D3D11_CREATE_DEVICE_DEBUG,//����̽� ���� �÷���.(�����)
			&g_FeatureLevels,			//(������) ����̽� ��� ����(Feature Level) �迭
			1,							//(������) ����̽� ��� ����(Feature Level) �迭 ũ��.
			D3D11_SDK_VERSION,			//DX SDK ����.
			&sd,						//����̽� ���� �� �ɼ�.
			&g_pSwapChain,				//[���] ����ü�� �������̽� ���.
			&g_pDevice,					//[���] ����̽� �������̽� ���.
			NULL,						//[���] (������) ����̽� ��� ����. �ʿ���ٸ� NULL ����. 
			&g_pDXDC					//[���] ����̽� ���ؽ�Ʈ ���.
	);
	if (FAILED(hr))
	{
		ynError(hr, L"����̽� / ����ü�� ���� ����.");
	}
	
	return hr;
}





/////////////////////////////////////////////////////////////////////////////
//
// ��ġ-����ü���� ����Ÿ��(�����) ȹ��
//
// ���ҽ���(Resource View)  
// DX11 �� ���ҽ��� "����" �������� �����ϰ� ����Ǿ� �ֽ��ϴ�. 
// (�޸� ������ ȿ���� ���� �� �ٿ뵵 Ȱ���� ����..)
// ���� ����� �� �뵵�� �´� ���ٹ��(�������̽�)�� ȹ��(����)�ؾ� �մϴ�.
// �� �۾��� "���ҽ� �� Resource View" �� ���� ó���մϴ�.
//                     
// ����ü�ο� ����(Bind)�� ������ ��¹��� - "����� BackBuffer" �� 2D ��� ���ҽ�(�ؽ�ó) �̸�
// "����Ÿ�� RenderTarget" ������ ���ҽ��並 ����, �ش� ���۸� ����/��� �ϰڽ��ϴ�.
//
HRESULT CreateRenderTarget()
{
	HRESULT hr = S_OK;

	// ����� ȹ��.
	ID3D11Texture2D *pBackBuffer;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);		// ����� ���, 2D �ؽ��� ����, ȭ����¿�
	if (FAILED(hr))	return hr;

	//ȹ���� ����ۿ� ����Ÿ�� �� ����.(����Ÿ��'��'���� ������)
	hr = g_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRTView);		// '����Ÿ�ٺ�'�� ����.	
	if (FAILED(hr)) 
	{
		ynError(hr, L"�����-����Ÿ�ٺ� ���� ����.");
		//return hr;
	}

	//���ҽ� �� ���� ��, ���ʿ��� DX �ڵ��� �����ؾ� �մϴ�.(�޸� ���� ����)
	SAFE_RELEASE(pBackBuffer);

	return hr;
}





/////////////////////////////////////////////////////////////////////////////
//
// ����-���ٽǹ��� ����. : DX11 ������ ����������-����Ÿ�ٺ�- �� ����������, 
//                       ����-���ٽǹ��ۿ��� ���� ������ �մϴ�.
//                       ����̽��� ��ϵ� ���� �����Դϴ�.
//
HRESULT CreateDepthStencil()
{
	HRESULT hr = S_OK;

	//����/���ٽ� ���ۿ� ���� ����.
	D3D11_TEXTURE2D_DESC   td;					
	ZeroMemory(&td, sizeof(td));
	td.Width = g_Mode.Width;
	td.Height = g_Mode.Height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_D32_FLOAT;			// 32BIT. ���� ����.
	td.SampleDesc.Count = 1;					// AA ���� - RT �� ���� �԰� �ؼ�.
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// ����-���ٽ� ���ۿ����� ����.
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;
	// ���� ���� ����.
	//ID3D11Texture2D* pDS = NULL;						 
	hr = g_pDevice->CreateTexture2D(&td, NULL, &g_pDS);	
	if (FAILED(hr))
	{
		ynError(hr, L"����/���ٽ� ���ۿ� CreateTexture ����.");		 
		return hr;
	}
	

	// ����-���ٽǹ��ۿ� ���ҽ� �� ���� ����. 
	D3D11_DEPTH_STENCIL_VIEW_DESC  dd;
	ZeroMemory(&dd, sizeof(dd));
	dd.Format = td.Format;
	dd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; //AA ����.
	dd.Texture2D.MipSlice = 0;

	//����-���ٽ� ���� �� ����.
	hr = g_pDevice->CreateDepthStencilView(g_pDS, &dd, &g_pDSView);
	if (FAILED(hr))
	{
		ynError(hr, L"����/���ٽǺ� ���� ����.");		 
		return hr;
	}

	//���ҽ� �� ���� ��, ���ʿ��� DX �ڵ��� �����ؾ� �մϴ�.(�޸� ���� ����)
	//SAFE_RELEASE(pDS);
		
	return hr;
}




/////////////////////////////////////////////////////////////////////////////
//
// ����Ʈ ���� :  DX11 ������ �⺻ó�� ���� ������ ����ڰ� ���� �����ؾ��մϴ� 
//			     (OpenGL ������ �������� �̷��� �� �Խ��ϴ�.)
//
void SetViewPort()
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (FLOAT)g_Mode.Width;
	vp.Height = (FLOAT)g_Mode.Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	g_pDXDC->RSSetViewports(1, &vp);

}




/////////////////////////////////////////////////////////////////////////////
//
// ��ġ-����ü��/����Ÿ��/����/���ٽ� ���� Ŭ���� 
//
int ClearBackBuffer(UINT flag, COLOR col, float depth, UINT stencil)
{
	g_pDXDC->ClearRenderTargetView(g_pRTView, (float*)&col);			//����Ÿ�� �����.
	g_pDXDC->ClearDepthStencilView(g_pDSView, flag, depth, stencil);	//����/���ٽ� �����.
	
	return YN_OK;
}



/////////////////////////////////////////////////////////////////////////////
//
// ��ġ-����ü�� ��ȯ : ��� �׸��� �Ϸ�� �� ȣ���ؾ� ��.
//
int Flip()
{
	g_pSwapChain->Present(g_bVSync, 0);			//ȭ����� : Flip! (+��������ȭ)

	return YN_OK;
}




////////////////////////////////////////////////////////////////////////////// 
//
// ��ġ ������ ���� ��ü ����.
//
//----------------------------------------------------------------------------
// ���� ��ü State Objects (DX10/11)
// DX10 ���� ���� TnL �� RenderState �� ���ŵǾ����ϴ�.
// �̸� ����ϴ� ���� ���°�ü State Objects �������̽���, ������ ���¸� �ϳ��� �׷����� 
// ���� �������� ����̽��� �����մϴ�.  �̸� ���� ��ġ�� ��� ���� ��ȭ�� �ѹ��� �����Ͽ� 
// ���ʿ��� �������(Overhead) �� ���̰� ���� ȿ������ �������� ������ �մϴ�.
//
// ���°�ü�� ���� �ʱ�� ������ ����ϱ⸦ �����ϸ� ����� ����(Release) �ؾ� �մϴ�.
// ���°�ü�� �����Ұ���(Immutable, �б�����) ��ü �Դϴ�.
// DX9 ������ State-Block �� �̿� ������ ����� ����߾����ϴ�.
//
// ���� ��ü �������̽� �� '�����Ͷ����� �������� Rasterize Stage' ���� ������ 
// ID3D11RasterizerState �������̽��� ���� ó���մϴ�.  
// �����ϰ� ������ ����/��� ��� ������ �����սô�.  �ڼ��� ���� ������ �����Ͻʽÿ�. 
// ��ũ1 : ���� ��ü https://msdn.microsoft.com/en-us/library/windows/desktop/bb205071(v=vs.85).aspx
// ��ũ2 : ���̹��� ���� �����ϱ� https://msdn.microsoft.com/ko-kr/library/windows/desktop/bb205074(v=vs.85).aspx#Create_Depth_Stencil_State 
//----------------------------------------------------------------------------
//
int StateObjectCreate()
{
	
	//----------------------------
	// ������ ���� ��ü ���� : "�����Ͷ����� �������� Rasterize Stage" ���� ����.
	//----------------------------
	//...


	//----------------------------
	// ����/���ٽ� ���� ��ü ����.: "��º��ձ� Output Merger" ���� ����. 
	//----------------------------
	//...	 
	D3D11_DEPTH_STENCIL_DESC  ds;
	ds.DepthEnable = TRUE;
	ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ds.DepthFunc = D3D11_COMPARISON_LESS;
	ds.StencilEnable = FALSE;
	//ds.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	//ds.StnecilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	//...���� �⺻��, ����... 
	//...
	//ù��° ���� ��ü : Z-Test ON! (�⺻��)
	g_pDevice->CreateDepthStencilState(&ds, &g_DSState[DS_DEPTH_ON]);

	//�ι�° ���� ��ü : Z-Test OFF ����.
	ds.DepthEnable = FALSE;
	g_pDevice->CreateDepthStencilState(&ds, &g_DSState[DS_DEPTH_OFF]);

	//����° ���� ��ü : Z-Test On + Z-Write OFF.
	// Z-Test (ZEnable, DepthEnable) �� ������, Z-Write ���� ��Ȱ��ȭ �˴ϴ�.
	ds.DepthEnable = TRUE;
	ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;		//���̰� ���� ��.
	g_pDevice->CreateDepthStencilState(&ds, &g_DSState[DS_DEPTH_WRITE_OFF]);



	//----------------------------
	// ���ĺ��� ���� ��ü ���� : "��º��ձ� Output Merger" ���� ����. 
	//----------------------------
	//...
	

	return YN_OK;
}




///////////////////////////////////////////////////////////////////////////////
//
//  ���°�ü �����ϱ� : ���� ����� 1ȸ ȣ��.
//
void StateObjectRelease()
{
	for (int i = 0; i < DS_MAX_; i++)
		SAFE_RELEASE(g_DSState[i]);
}






///////////////////////////////////////////////////////////////////////////////
//
// �ʴ� �����Ӽ� ���.
//
// ���� : int x, y : ����� ȭ�� ��ǥ.
//
void PutFPS(int x, int y)
{
	static int oldtime = GetTickCount();
	int nowtime = GetTickCount();

	static int frmcnt = 0;
	static float fps = 0.0f;

	++frmcnt;
	
	int time = nowtime - oldtime;
	if( time >= 999)
	{
		oldtime = nowtime;

		fps = (float)frmcnt*1000/(float)time;
		frmcnt = 0;
	}
	
	//ynTextDraw(x, y, RGB(0, 0, 255), L"FPS:%.1f/%d", fps, frmcnt );
	ynTextDraw(x, y, COLOR(0, 1, 0, 1), L"FPS:%.1f/%d", fps, frmcnt);
}




/////////////////////////////////////////////////////////////////////////////
//
// ���� ����ȭ�� ����ð� ȹ��.
//
float GetEngineTime()
{	 
	static int oldtime = GetTickCount();
	int nowtime = GetTickCount();
	float dTime = (nowtime - oldtime) * 0.001f;
	oldtime = nowtime;

	return dTime; 
}





////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
////////////////////////////////////////////////////////////////////////////// 
//
// ���� ó�� �Լ���.
//
////////////////////////////////////////////////////////////////////////////// 

////////////////////////////////////////////////////////////////////////////// 
//
// ���� �޼��� ó��
//
int ynErrorW(TCHAR* file, UINT line, TCHAR* func, BOOL bMBox, HRESULT hr, TCHAR* msg, ...)
{
	TCHAR msgva[1024] = L"";
	va_list vl;
	va_start(vl, msg);
	_vstprintf(msgva, msg, vl);
	va_end(vl);
	

	//HRESULT ���� ���� �޼��� ���.
	TCHAR herr[1024] = L"";
	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0,  hr, 
				   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				   herr, 1024, NULL);

	//���� �޼��� ���̱�.
	TCHAR errmsg[2048] = L"";
	_stprintf(errmsg, L"%s \n�����ڵ�(0x%X) : %s \nFile = %s (%d)\nFunc = %s\n\n",
		msgva, hr, herr,
		file, line, func );
	 

	//(����� ��) VS ���â���� ���..
	OutputDebugString(L"\n");
	OutputDebugString(errmsg);

	//�α����Ϸ� ���.
	//...

	//�޼��� â ���..
	if (bMBox)
	{
		MessageBox(NULL, errmsg, L"Yena::Error", MB_OK | MB_ICONERROR);
		//GetLastError(hr);
	}

	return YN_OK;
}



////////////////////////////////////////////////////////////////////////////// 
//
// ���� �޼��� ó�� : ���̴� ���� ó����.
// 
int ynErrorW(BOOL bMBox, TCHAR* msg, HRESULT hr, ID3DBlob* pBlob, 
			TCHAR* filename, char* EntryPoint, char* ShaderModel )
{

	/*//�����Ű����� ó��.
	TCHAR msgva[2048] = L"";
	va_list vl;
	va_start(vl, msg);
	_vstprintf(msgva, msg, vl);
	va_end(vl);
	*/
	//�Ķ����, �����ڵ�� ��ȯ.
	TCHAR func[80] = L"";
	::mbstowcs(func, EntryPoint, strlen(EntryPoint));
	TCHAR sm[20] = L"";
	::mbstowcs(sm, ShaderModel, strlen(ShaderModel));


	//���̴� ���� �޼��� �б�.
	TCHAR errw[4096] = L"";
 	::mbstowcs(errw, (char*)pBlob->GetBufferPointer(), pBlob->GetBufferSize());
 

	//HRESULT ���� ���� �޼��� ��� 
	//�ý������� ���� ��� ���̴� �����޼����� ����Ȯ�ϹǷ� ����.
	TCHAR herr[1024] = L"�Ʒ��� ������ Ȯ���Ͻʽÿ�.";
	/*FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		herr, 1024, NULL);
	*/
	

	//���� �޼��� ���̱�.
	TCHAR errmsg[1024];
	_stprintf(errmsg, L"%s \nFile=%s  Entry=%s  Target=%s  \n�����ڵ�(0x%08X) : %s \n\n%s", 
				msg, filename, func, sm,
				hr, herr, errw ); 
	

	//(����� ��) VS ���â���� ���..
	OutputDebugString(L"\n");
	OutputDebugString(errmsg);
	//OutputDebugString(errw);


	//�α����Ϸ� ���.
	//...


	//�޼��� â ���..
	if (bMBox)
	{
		MessageBox(NULL, errmsg, L"Yena::Error", MB_OK | MB_ICONERROR);
		//MessageBox(NULL, errw, L"Yena::Error", MB_OK | MB_ICONERROR);
	}
		
	return YN_OK;
}





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// ��ġ/GPU ���� ȹ�� �Լ���
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// ��ġ/GPU ���� ȹ��
//
void GetDeviceInfo()
{
 	//��ġ ��ɷ��� Ȯ��.
	GetFeatureLevel();

	//GPU ���� ���.
	GetAdapterInfo(&g_Adc);

	//����� ���� ���.
	//...
		 
}




/////////////////////////////////////////////////////////////////////////////
//
//  DX ��� ���� ���ϱ�.
//
static TCHAR* strFeature[4][4] =
{
	{ L"DX9",   L"DX9.1",  L"DX9.2", L"DX9.3" },
	{ L"DX10",  L"DX10.1", L"N/A",   L"N/A" },
	{ L"DX11",  L"DX11.1", L"N/A",   L"N/A" },
	{ L"DX12",  L"DX12.1"  L"N/A",   L"N/A" }
};

void GetFeatureLevel()
{
	UINT feat = g_FeatureLevels;
	UINT ver = 0;
	UINT sub = 0;

	#define OFFSET 0x9;

	ver = ((feat & 0xf000) >> 12) - OFFSET;	//���� ���� ����.   	
	sub = ((feat & 0x0f00) >> 8);			//���� ���� ����.
	
	g_strFeatureLevel = strFeature[ver][sub];
}




/////////////////////////////////////////////////////////////////////////////
//
// ��ġ/GPU ���� ȹ�� 
//
// �⺻ ��ġ�� ó����.���� GPU �����ÿ��� ������ ���� ó���� �ʿ�.
// �����׸�1 : DirectX Graphics Infrastructure (DXGI) /  https://msdn.microsoft.com/ko-kr/library/windows/desktop/ee417025(v=vs.85).aspx
// �����׸�2 : How To: Enumerate Adapters  /  https://msdn.microsoft.com/en-us/library/windows/desktop/ff476877(v=vs.85).aspx 
// �����׸�3 : DXGI_ADAPTER_DESC structure /  https://msdn.microsoft.com/ko-kr/library/windows/desktop/bb173058(v=vs.85).aspx
//

// ����̽�/GPU ���� ȹ�� <��� #1> 
// DXGI 1.1 / DXGI Factory ��� / DX11 �̻� ����
//
HRESULT GetAdapterInfo(DXGI_ADAPTER_DESC1* pAd)
{
	IDXGIAdapter1* pAdapter;
	IDXGIFactory1* pFactory = NULL;
	//DXGI_ADAPTER_DESC ad;

	//DXGIFactory ��ü ����. (DXGI.lib �ʿ�)
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory)))
	{
		return E_FAIL;
	}

	pFactory->EnumAdapters1(0, &pAdapter);		//��� ȹ��.
	pAdapter->GetDesc1(pAd);					//��� ���� ȹ��.
	//*pAd = ad;								//�ܺη� ����.

	//���� �����, ������ �������̽��� �����մϴ�. (�޸� ���� ����)
	SAFE_RELEASE(pAdapter);
	SAFE_RELEASE(pFactory);

	return S_OK;
}

/*
// ����̽�/GPU ���� ȹ�� <��� #2>  
// DXGI 1.1 / ���� ��ġ Interface ��� / DX11 �̻� ����
//
HRESULT GetAdapterInfo(DXGI_ADAPTER_DESC1* pAd)
{
	IDXGIDevice* pDXGIDev;
	IDXGIAdapter1* pAdapter;

	//DXGIFactory ��ü ����. (DXGI.lib �ʿ�)
	if (FAILED(g_pDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDXGIDev)))
	{
		return E_FAIL;
	}

	pDXGIDev->GetAdapter((IDXGIAdapter**)&pAdapter);		//��� ȹ��.
	pAdapter->GetDesc1(pAd);					//��� ���� ȹ��.

	//���� �����, ������ �������̽��� �����մϴ�. (�޸� ���� ����)
	SAFE_RELEASE(pAdapter);
	SAFE_RELEASE(pDXGIDev);

	return S_OK;
}
*/




/////////////////////////////////////////////////////////////////////////////
//
// ��ġ/GPU ���� ���
//
// �����׸�1 : DXGI_ADAPTER_DESC structure
// ������ũ1 : https://msdn.microsoft.com/ko-kr/library/windows/desktop/bb173058(v=vs.85).aspx
//
void AdapterInfo(int x, int y, COLOR col)
{
	#define ToMB(a) (a/1024/1024)

	DXGI_ADAPTER_DESC1& dc = g_Adc;
	ynTextDraw(x, y += 14, COLOR(0, 1, 0, 1), L"[GPU #%d]", 0);
	//GPU ����.
	ynTextDraw(x, y += 14, col, L"Adapter: %u", 0 );		//�⺻ ��ġ�� ó����. ���� GPU �����ÿ���  ����ó���� �ʿ�.
	ynTextDraw(x, y += 14, col, L"Description: %s", dc.Description);
	ynTextDraw(x, y += 14, col, L"VendorID: %u", dc.VendorId);
	ynTextDraw(x, y += 14, col, L"DeviceID: %u", dc.DeviceId);
	ynTextDraw(x, y += 14, col, L"SubSysID: %u", dc.SubSysId);
	ynTextDraw(x, y += 14, col, L"Revision: %u", dc.Revision);
	ynTextDraw(x, y += 14, col, L"VideoMem: %lu MB", ToMB(dc.DedicatedVideoMemory));
	ynTextDraw(x, y += 14, col, L"SystemMem: %lu MB", ToMB(dc.DedicatedSystemMemory));
	ynTextDraw(x, y += 14, col, L"SharedSysMem: %lu MB", ToMB(dc.SharedSystemMemory));
	ynTextDraw(x, y += 14, col, L"AdpaterLuid: %u.%d", dc.AdapterLuid.HighPart, dc.AdapterLuid.LowPart);
		 
}

 


/////////////////////////////////////////////////////////////////////////////
//
// ��ġ/GPU �� �ý��� ���� ���
//
void SystemInfo(int x, int y, COLOR col)
{
	COLOR col2 = col * 0.7f;

	ynTextDraw(x, y += 14, col, L"[SYSTEM]");
	ynTextDraw(x, y += 14, col, L"VGA: %s", g_Adc.Description);
	ynTextDraw(x, y += 14, col, L"Feat: %s", g_strFeatureLevel);
	ynTextDraw(x, y += 14, col, L"�ػ�: %dx%d", g_Mode.Width, g_Mode.Height);
	
	//GPU ���� ���.
	//AdapterInfo(x, y += 14, col);
	
	//Yena ���� ���.
	YenaSysInfo(x, y, col);
}




/*  Yena Math �� ���Ե�.
/////////////////////////////////////////////////////////////////////////////
//
// ���� ������ �����ε�.
//
COLOR operator * (COLOR& lhs, float rhs)
{
	COLOR col;
	col.x = lhs.x * rhs;
	col.y = lhs.y * rhs;
	col.z = lhs.z * rhs;
	col.w = lhs.w * rhs;

	return col;
}
*/






//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
//////////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////////
//
// ��Ʈ ���� --> Yena �� ���Ե� 
//
// DirectXTK : SpriteFont ��� ��Ʈ ��� Ŭ����
// 2D Texture / Sprite ����� ��Ʈ ��� ���̺귯��.
// ������ ����������, CJK �迭�� ��Ʈ�� ��� �ϼ����� �����˴ϴ�.
// ���� : https://directxtk.codeplex.com/wikipage?title=SpriteFont
// #include "SpriteFont.h"	
//
// �ڼ��� ���� DirectXTK.h �Ǵ�  Yena/DXTK/Readme.txt �� ����. 
//
///////////////////////////////////////////////////////////////////////////////
/*
SpriteBatch* g_pFontBatch = nullptr;
SpriteFont*  g_pFont = nullptr;


///////////////////////////////////////////////////////////////////////////////
// 
int ynFontCreate(LPDEVICE pDev)
{
	
	//��ġ ��� ȹ��.
	LPDXDC pDXDC = nullptr;	
	pDev->GetImmediateContext(&pDXDC);


	//Sprite Batch ��ü ����.
	g_pFontBatch = new SpriteBatch(pDXDC);

	//DirectX Toolkit : Sprite Font ��ü ����.
	TCHAR* filename = L"../Yena/Font/����9k.sfont";	//ASCII 0 ~ 255 + Ư������'��' + Unicode �ѱ� �ϼ��� �� 11,440 ����, ũ��:9		
	try {
		g_pFont = new SpriteFont(pDev, filename );	
		g_pFont->SetLineSpacing(14.0f);				//��Ʈ9 ����, �ٰ��� ����. '���߶��� ��½� ����� ������'
		g_pFont->SetDefaultCharacter('_');			//��� ���ڰ� �̰˻��� ��� ����� Ű��.
	}
	catch (std::exception& e)
	{
		//ynError(0, L"��Ʈ ���� ���� : File=%s", filename);
		TCHAR msg[1024] = L"";	
		::mbstowcs(msg, e.what(), strlen(e.what()));
		ynError(0, L"��Ʈ ���� ���� : %s \n File=%s", msg, filename);
	}

	//�����, ��ġ��� ����.
	SAFE_RELEASE(pDXDC);

	return YN_OK;
}



///////////////////////////////////////////////////////////////////////////////
//
void ynFontRelease()
{
	SAFE_DELETE(g_pFontBatch);
	SAFE_DELETE(g_pFont);
}




/////////////////////////////////////////////////////////////////////////////// 
//
// ���ڿ� ��� : DX11 (�����ڵ�) ����.
//
void ynTextDraw(int x, int y, COLOR col, TCHAR* msg, ...)
{
	const DWORD _size = 2048;

	TCHAR buff[_size] = L"";
	va_list vl;
	va_start(vl, msg);
	_vstprintf(buff, msg, vl);
	va_end(vl);

	g_pFontBatch->Begin();
	//g_pFontBatch->Begin(SpriteSortMode_Deferred, nullptr, nullptr, ypStateObj[YNSO_DEPTH_ON_]);		//���̿��� �߰�.
	g_pFont->DrawString(g_pFontBatch, buff, Vector2((float)x, (float)y), Vector4(col));					//���� ũ��.
	//g_pFont->DrawString(ypFontBatch, buff, Vector2((float)x, (float)y), Vector4(col), 0, Vector4::Zero, 0.8f);	//80% ��� ���.
	g_pFontBatch->End();

}
*/

 
/***************** End of "Device.cpp" *******************************/


 
