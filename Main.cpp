//
// �T���v���̋��ʏ���
// Ver. 20090703
//
// Copyright (C) WADA Takao. All rights reserved.
//
#include <windows.h>
#include <d3dx9.h>
#include <stdio.h>
// ���������[�N�`�F�b�N
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include "Main.h"

//�K�v�ȃ��C�u�����t�@�C���̃��[�h
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

char* g_clsName = "Game";	// �N���X��
char* g_winName = "�g�̃T���v�� Ver.20090703";	// �E�B���h�E��

// �O���[�o���ϐ��錾
LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pD3DDevice = NULL;

#define TEXT_DATA_MAX	(64)
#define TEXT_LENGTH		(128)

// �f�o�b�O�f�[�^�\����
typedef struct textdata_t 
{
	char str[TEXT_LENGTH];			// ������
	int x;							// X���W
	int y;							// Y���W
} TEXTDATA;

// ���[�J���ϐ�
static LPD3DXFONT g_Font;						// �t�H���g�I�u�W�F�N�g
static TEXTDATA g_Array[TEXT_DATA_MAX];			// �e�L�X�g�f�[�^
static int g_ArrayCount;						// �e�L�X�g�f�[�^�J�E���g
static unsigned long g_Color;					// �e�L�X�g�J���[

// �֐��v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT InitD3D(HWND hWnd);
void Initialize();
void Update(float elapsedTime);
void Draw(float elapsedTime);
void Finalize();
void Cleanup();
void InitText();

//-------------------------------------------------------------
// �A�v���P�[�V�����̃G���g���|�C���g
// ����
//		hInstance     ���݂̃C���X�^���X�̃n���h��
//		hPrevInstance �ȑO�̃C���X�^���X�̃n���h��
//		lpCmdLine	  �R�}���h���C���p�����[�^
//		nCmdShow	  �E�B���h�E�̕\�����
// �߂�l
//		����������0�ȊO�̒l
//-------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	// �S�Ẵ��|�[�g�o�͂��E�B���h�E�ɑ���ݒ�
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_WNDW);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW);
	// ���������[�N���o
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif
	HWND		hWnd;

	// �E�B���h�E�N���X�̏�����
	WNDCLASSEX	wcex = 
	{
		sizeof(WNDCLASSEX),				// ���̍\���̂̃T�C�Y
		NULL,							// �E�C���h�E�̃X�^�C��(default)
		WindowProc,						// ���b�Z�[�W�����֐��̓o�^
		0,								// �ʏ�͎g��Ȃ��̂ŏ��0
		0,								// �ʏ�͎g��Ȃ��̂ŏ��0
		hInstance,						// �C���X�^���X�ւ̃n���h��
		NULL,							// �A�C�R���i�Ȃ��j
		LoadCursor(NULL, IDC_ARROW),	// �J�[�\���̌`
		NULL, NULL,						// �w�i�Ȃ��A���j���[�Ȃ�
		g_clsName,						// �N���X���̎w��
		NULL							// ���A�C�R���i�Ȃ��j
	};

	// �E�B���h�E�N���X�̓o�^
	if(RegisterClassEx(&wcex) == 0)
	{
		return 0;	// �o�^���s
	}

	// �`��̈悪�w��T�C�Y�ɂȂ�悤�ɃE�B���h�E�T�C�Y��ݒ�
	RECT rect;		// ��`
	// ��`�̒l��ݒ�
	SetRect(&rect, 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT);
	// ��`���ɂ��E�B���h�E��`���v�Z
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	// �E�B���h�E�̕��ƍ����̌v�Z
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	hWnd = CreateWindowEx(
		NULL,						// �g���Ȃ�
		g_clsName,
		g_winName,
		WS_OVERLAPPEDWINDOW,		// �E�B���h�E�X�^�C�� (�W���̃E�B���h�E���쐬)
		CW_USEDEFAULT,				// �E�B���h�E�̉������̈ʒu
		CW_USEDEFAULT,				// �E�B���h�E�̏c�����̈ʒu
		width,						// ��
		height,						// ����
		NULL,
		NULL,
		hInstance,
		NULL
	);

	// �E�B���h�E�̕\��
    ShowWindow(hWnd, nCmdShow);

	// WM_PAINT���Ă΂�Ȃ��悤�ɂ���
	ValidateRect(hWnd, 0);
	
	// Direct3D�̏�����
	if(FAILED(InitD3D(hWnd)))
	{
		return 0;	// ���������s
	}

	// �e�L�X�g�����̏����ݒ������B
	InitText();

	// �Q�[���̏������������Ăяo���B�O���̃t�@�C���Œ�`���邱�ƁB
	Initialize();

	// ���b�Z�[�W��������ѕ`�惋�[�v
	// �t���[�����[�g��ݒ肷��
	LONGLONG frequency, counter, nextFrame, frameCount;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&counter);
	frameCount = frequency / FRAME_RATE;
	nextFrame = counter;

	// ���b�Z�[�W��������ѕ`�惋�[�v
	MSG msg;
	while (1) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			// PostQuitMessage() ���Ă΂ꂽ�烋�[�v���I������B
			if (msg.message == WM_QUIT)
			{
				break;
			}
			// ���b�Z�[�W��|�󂵁C�f�B�X�p�b�`����B
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else 
		{	// �������郁�b�Z�[�W�������Ƃ��ɕ`����s���B
			QueryPerformanceCounter((LARGE_INTEGER*)&counter);
			if (counter > nextFrame) 
			{
				float elapsedTime = (float)(counter - nextFrame + frameCount) / frequency;
				// �Q�[���̂P�t���[�����Ƃ̍X�V�������Ăяo���B�O���̃t�@�C���Œ�`���邱�ƁB
				Update(elapsedTime);

				// �Q�[���̂P�t���[�����Ƃ̕`�揈�����Ăяo���B�O���̃t�@�C���Œ�`���邱�ƁB
				Draw(elapsedTime);

				nextFrame = counter + frameCount;
			}
		}
	}
	// �Q�[���̌�n���������Ăяo���B
	Finalize();

	// �Ō�̌�n��������B
	Cleanup();

	return (int) msg.wParam;
}

//-------------------------------------------------------------
// ���b�Z�[�W�����p�R�[���o�b�N�֐�
// ����
//		hWnd	�E�B���h�E�n���h��
//		msg		���b�Z�[�W
//		wParam	���b�Z�[�W�̍ŏ��̃p�����[�^
//		lParam	���b�Z�[�W��2�Ԗڂ̃p�����[�^
// �߂�l
//		���b�Z�[�W��������
//-------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
	{
	case WM_KEYDOWN:				// �L�[�������ꂽ
		// ESC�L�[�ŏI��
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);		// �v���O�������I������
		}
		break;
	case WM_CLOSE:					// �E�C���h�E������ꂽ
		PostQuitMessage(0);			// �A�v���P�[�V�������I������
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

//-------------------------------------------------------------
// Direct3D������������B
// ����
//		hWnd	�E�B���h�E�n���h��
// �߂�l
//		S_OK: �����CE_FAIL:���s
//-------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
    D3DDISPLAYMODE d3ddm;
    
	// Direct3D9�I�u�W�F�N�g�̍쐬
	if((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == 0)
	{
        return E_FAIL;	// �擾���s
    }
	
	// ���݂̃f�B�X�v���C���[�h���擾
    if(FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
	{
		return E_FAIL;
	}

	// �f�o�C�X�̃v���[���e�[�V�����p�����[�^��������
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.Windowed = TRUE;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferFormat= d3ddm.Format;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	
	// �f�B�X�v���C�A�_�v�^��\�����߂̃f�o�C�X���쐬
	// �`��ƒ��_�������n�[�h�E�F�A�ōs�Ȃ�
	if(FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, 
									D3DDEVTYPE_HAL, 
									hWnd, 
									D3DCREATE_HARDWARE_VERTEXPROCESSING, 
									&d3dpp, &g_pD3DDevice))) 
	{
		// ��L�̐ݒ肪���s������
		// �`����n�[�h�E�F�A�ōs���A���_������CPU�ōs�Ȃ�
		if(FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, 
										D3DDEVTYPE_HAL, 
										hWnd, 
										D3DCREATE_SOFTWARE_VERTEXPROCESSING, 
										&d3dpp, &g_pD3DDevice))) 
		{
			// ��L�̐ݒ肪���s������
			// �`��ƒ��_������CPU�ōs�Ȃ�
			if(FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, 
											D3DDEVTYPE_REF, hWnd, 
											D3DCREATE_SOFTWARE_VERTEXPROCESSING, 
											&d3dpp, &g_pD3DDevice))) 
			{
				// ���������s
				return E_FAIL;
			}
		}
	}

	return S_OK;
}

//-------------------------------------------------------------
// Direct3D�I�u�W�F�N�g���������B
// ����
//		�Ȃ�
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
void Cleanup()
{
	SAFE_RELEASE(g_Font);
	SAFE_RELEASE(g_pD3DDevice);
	SAFE_RELEASE(g_pD3D);
}

//-------------------------------------------------------------
// �t�H���g�I�u�W�F�N�g���쐬����B
// ����
//		�Ȃ�
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
void InitText()
{
	// �����C���C�X�^�C��
	D3DXCreateFont(g_pD3DDevice, 14, FW_DONTCARE, FW_BOLD, NULL, FALSE,
		SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "MS �S�V�b�N", &g_Font);
	// �F
	g_Color = 0xFF000000;
	// �f�[�^������
	memset(g_Array, 0, sizeof(TEXTDATA)*TEXT_DATA_MAX);
	g_ArrayCount = 0;
}

//-------------------------------------------------------------
// �����̐F��ݒ肷��B
// ����
//		r, g, b			�F
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
void SetTextColor(int r, int g, int b)
{
	g_Color = D3DCOLOR_XRGB(r, g, b);
}

//-------------------------------------------------------------
// �������`�悷��B�`�惋�[�v���ōŌ�Ɉ�x�Ăяo�����ƁB
// ����
//		�Ȃ�
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
void DrawText(void)
{
	RECT rect;

	for (int i = 0; i < g_ArrayCount; i++) 
	{
		// �`��ʒu���w��
		rect.top  = g_Array[i].y;
		rect.left = g_Array[i].x;
		rect.right = 0;
		rect.bottom = 0;
		// ������`��̈���v�Z
		g_Font->DrawText(NULL, g_Array[i].str, -1, &rect, DT_CALCRECT, NULL);
		// �������`��
		g_Font->DrawText(NULL, g_Array[i].str, -1, &rect, DT_LEFT | DT_TOP, g_Color);
	}

	g_ArrayCount = 0;
}

//-------------------------------------------------------------
// �ҏW�����������`�悷��B
// ����
//		x, y		�`�悷��X�N���[�����W
//		format		����
//		...			����
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
void TextPrintf(int x, int y, char *format, ...)
{
	// �z��I�[�o�[�̃`�F�b�N
	if (g_ArrayCount >= TEXT_DATA_MAX - 1)
	{
		return;
	}

	va_list args;

	// ���������������
	va_start(args, format);
	vsprintf(g_Array[g_ArrayCount].str, format, args);
	va_end(args);

	// �f�[�^��ۑ�
	g_Array[g_ArrayCount].x = x;
	g_Array[g_ArrayCount].y = y;

	g_ArrayCount++;
}

//-------------------------------------------------------------
// �R���\�[���ɕ������ҏW�����������`�悷��B
// ����
//		format		����
//		...			����
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
void DebugPrintf(char* format, ...)
{
	char str[256];
	va_list args;

	// ���������������
	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);

	// �f�o�b�O�o��
	OutputDebugString(str);
}
