//
// Device.h : D3D ����̽� �¾� �ڵ�.
//
// DirectX ���� ���� : DX11
//
// 2003.11.4. Kihong Kim / mad_dog@hanmail.net
// 2010.12.01. Update. (DX11.Jun.2010)
// 2016.12.27. Update. (DX11. Windows SDK 8.1)
//
#pragma once
#pragma warning(disable:4996)

// Platform Header �� Library
//
#include "Windows.h"
#include "stdio.h"

///////////////////////////////////////////////////////////////////////////////
//
// DirectX ǥ�� ���̺귯�� 
//
///////////////////////////////////////////////////////////////////////////////
// < DirectX ������ ���� ���̺귯�� >
// DX9         : DirectX SDK 2002 Nov. 
// DX10.x/DX11 : DirectX SDK 2010 June (DX ���� ���� ������ ����)
// DX11.X/DX12 : Windows SDK 8.x  (DX �� ���ԵǾ� ������) 

// DX ǥ�� ��� 
#include "D3D11.h"					//DX ǥ�� ���. (DX 11.0)
//#include "D3D11_1.h"				//DX ǥ�� ���. (DX 11.1)
#pragma comment(lib, "D3D11")		//DX ���̺귯�� �ε�.  D3D11.dll �ʿ�.

//DirectX Math : Dx ���� ���̺귯�� 
//#include "XNAMath.h"				//XNA Math ���̺귯��.(ver.2.0) DirectX SDK 2010 June. ����.
#include "DirectXMath.h"			//XNA Math ���̺귯��.(ver.3.0) VS2012 (Windows SDK 8) �̻�.
using namespace DirectX;

//DXGI : DirectX Graphics Infrastructure 
//DX �� ������ �������� �ϵ���� ������ ���� ����� �����մϴ�.
//#include "DXGI.h"					//DXGI ���. (d3d11.h �� ���Ե�)
#pragma comment(lib, "dxgi")		//DXGI ���̺귯��. DXGI.dll �ʿ�.


///////////////////////////////////////////////////////////////////////////////
//
// DirectX Toolkit : DX Ȯ�� ���̺귯��  --> Yena �� ���Ե�.
//
///////////////////////////////////////////////////////////////////////////////
// DX Ȯ�� ���̺귯�� �� ��ƿ��Ƽ ������.
// DX ���� �ʿ��� ���� ��� Ŭ���� �� �ΰ� ���񽺸� �����մϴ�.
// DirectXTK �� ���� ���ϰ� ����ϱ� ���ؼ� Project �Ӽ� �߰��� �ʿ��մϴ�.
// DirectXTK.h �Ǵ�  Yena/DXTK/Readme.txt �� �����ϼ���.  
//#include "DirectXTK.h"



////////////////////////////////////////////////////////////////////////////
//
// Yena : ���� ���.
//
////////////////////////////////////////////////////////////////////////////
// ���� ���߿� �ʿ��� �⺻ ��ġ �� Ȯ�� ����� ����.
// �����ӿ�ũ,��Ʈ,����,ī�޶�,����,���̴�,���� ��ü ���..
// �ڼ��� ���� Yena ����.txt ����.
//
#include "Yena.h"			// Yena ���� ���.




///////////////////////////////////////////////////////////////////////////////
//
// Ÿ�� �� ��� ������ : ������ ȣȯ�� ��� ���밡 ����.
//
///////////////////////////////////////////////////////////////////////////////
//DX11 ��ġ���� ������.											
typedef ID3D11Device*			LPDEVICE;
typedef ID3D11DeviceContext*	LPDXDC;
typedef IDXGISwapChain*			LPSWAPCHAIN;
typedef ID3D11RenderTargetView*	LPRTVIEW;
typedef ID3D11DepthStencilView*	LPDSVIEW;

/*
// DirectX Math Ÿ�� ������ : ȣȯ�� ���
// DX9/DX10 : DirectX SDK June.2010 ����
typedef D3DXMATRIXA16	MATRIXA;	//��� : 16����Ʈ ���� ����.
typedef D3DXMATRIX		MATRIX;		//��� : �Ϲ� ����.
typedef D3DXVECTOR4		VECTOR4;
typedef D3DXVECTOR3		VECTOR3;
typedef D3DXVECTOR2		VECTOR2;
*/
// DirectX Math Ÿ�� ������ : ���� �ҽ����� ȣȯ�� ����� ����.
// DX10/11 "XNAMath ver.2": DirectX SDK june.2010
// DX11/12 "XNAMath ver.3": DirectXMath, Windows SDK 8.x �� ����  
// ��ũ : https://msdn.microsoft.com/ko-kr/library/windows/desktop/ee418728(v=vs.85).aspx
//
typedef XMMATRIX		MATRIXA;	//��� : 16����Ʈ ����, SIMD ����. ����/���� ������. "Register Type"
typedef XMFLOAT4X4		MATRIX;		//��� : �Ϲ� ����. SIMD ������, Class ������ �����. "Storage Type"
typedef XMVECTOR		VECTOR;		//4���� ���� : 16����Ʈ ����, SIMD ����. ����/���� ������. "Register Type"
typedef XMFLOAT4		VECTOR4;	//4���� ���� : �Ϲ� ����, SIMD ������, Class ������ �����. "Storage Type"
typedef XMFLOAT3		VECTOR3;
typedef XMFLOAT2		VECTOR2;

//���� Ÿ��: 2����.
//typedef XMCOLOR		COLOR;		// r, g, b, a.  [������ 0~255]
typedef XMFLOAT4		COLOR;		// r, g, b, a.  [�Ǽ��� 0~1.0]


///////////////////////////////////////////////////////////////////////////////
//
// DX �� ��ġ ���� ���� �����͵�  
//
///////////////////////////////////////////////////////////////////////////////

int		DXSetup(HWND hwnd);
void	DXRelease(); 

int		ClearBackBuffer(UINT flag, COLOR col, float depth = 1.0f, UINT stencil = 0);
int     Flip();

float	GetEngineTime();
void	PutFPS(int x, int y);
//void  ynTextDraw( int x, int y, COLOR col, char* msg, ...);

void	GetDeviceInfo();
HRESULT GetAdapterInfo(DXGI_ADAPTER_DESC1* pAd);
void	SystemUpdate(float dTime);
void	SystemInfo(int x, int y, COLOR col);

// ���� DX ��� ���� ���ϱ�. 
void	GetFeatureLevel();


extern ID3D11Device*           g_pDevice;
extern ID3D11DeviceContext*	   g_pDXDC;
extern IDXGISwapChain*         g_pSwapChain;
extern ID3D11RenderTargetView* g_pRTView;
extern ID3D11DepthStencilView* g_pDSView;



// ��ġ ���� ���� ����ü. (DX9/11 ���� ȣȯ�� ������)
typedef DXGI_MODE_DESC	  DISPLAYMODE;	//DX11 ����
//typedef DXGI_MODE_DESC1 DISPLAYMODE;	//DX11.1 ����
//typedef D3DDISPLAYMODE DISPLAYMODE;   //DX9 ��.

extern DISPLAYMODE g_Mode;
extern HWND g_hWnd;
extern BOOL g_bShowFrame;

//��üȭ�� ��뿩��.
extern BOOL g_bWindowMode;
//��������ȭ ��뿩��.
extern BOOL g_bVSync;

//���� ����̽� DX ��� ����.
extern TCHAR* g_strFeatureLevel;




///////////////////////////////////////////////////////////////////////////////
//
// ������ ���� ��ü ����  
//
///////////////////////////////////////////////////////////////////////////////

//����/���ٽ� ���� ����.
enum {
	DS_DEPTH_ON,		//���̹��� ON! (�⺻��)
	DS_DEPTH_OFF,		//���̹��� OFF!
	DS_DEPTH_WRITE_OFF,	//���̹��� ���� ����.

	DS_MAX_,
};

//����/���ٽ� ���� ��ü�� : ���� ��ü ������.
extern ID3D11DepthStencilState* g_DSState[DS_MAX_];





///////////////////////////////////////////////////////////////////////////////
//
// ��Ÿ �ý��� ���� �Լ� �� ������.
//
///////////////////////////////////////////////////////////////////////////////

//�񵿱� Ű���� �Է� ó��.
#ifndef IsKeyDown
#define IsKeyDown(key)	((GetAsyncKeyState(key)&0x8000) == 0x8000)
#define IsKeyUp(key)	((GetAsyncKeyState(key)&0x8001) == 0x8001)
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(pBuff) if((pBuff)){ (pBuff)->Release(); (pBuff) = NULL; }
#define SAFE_DELETE(pBuff)	if((pBuff)){ delete (pBuff); (pBuff) = NULL; }
#define SAFE_DELARRY(pBuff) if((pBuff)){ delete [] (pBuff); (pBuff) = NULL; }
#endif

#ifndef YES_
#define YES_ TRUE
#define NO_  FALSE
#endif

/* Yena �� ���Ե�
#ifndef YN_OK
#define YN_OK	   0					//���� ����. Ư���� ���� ����.
#define YN_FAIL   -1					//���� �߻�.
#define YN_FAILED(v)   ((v) != YN_OK_)	//���� üũ ��ũ��.
#endif

// ���� * ��Į�� ��  (ex)  col = col * 0.5f
COLOR operator * (COLOR& lhs, float s);
*/



///////////////////////////////////////////////////////////////////////////////
//
// ���� ��� �Լ�
//
///////////////////////////////////////////////////////////////////////////////

//int ynError(BOOL bBox, TCHAR* msg, ...);
int ynErrorW(BOOL bMBox, TCHAR* msg, HRESULT hr, ID3DBlob* pBlob, TCHAR* filename, char* EntryPoint, char* ShaderModel);
int ynErrorW(BOOL bMBox, TCHAR* msg, HRESULT hr, ID3DBlob* pBlob);
int ynErrorW(TCHAR* file, UINT line, TCHAR* func, BOOL bMBox, HRESULT hr, TCHAR* msg, ...);
 
#define ynError(hr, msg, ...)  \
ynErrorW( __FILEW__, __LINE__, __FUNCTIONW__, TRUE, hr, msg, __VA_ARGS__ )

//#define ynError(hr, msg, pBlob)  ynErrorW( TRUE, hr, msg, pBlob)



///////////////////////////////////////////////////////////////////////////////
//
// ��Ʈ ���� : �Ϲ� �Լ� ����. --> Yena �� ���Ե�
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
int  ynFontCreate(LPDEVICE pDev);
void ynFontRelease();
void ynTextDraw(int x, int y, COLOR col, TCHAR* msg, ...);
*/


/**************** end of "Device.h" ***********************************/