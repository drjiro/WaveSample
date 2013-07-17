//
// サンプルの共通処理
// Ver. 20090703
//
// Copyright (C) WADA Takao. All rights reserved.
//
#include <windows.h>
#include <d3dx9.h>
#include <stdio.h>
// メモリリークチェック
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include "Main.h"

//必要なライブラリファイルのロード
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

char* g_clsName = "Game";	// クラス名
char* g_winName = "波のサンプル Ver.20090703";	// ウィンドウ名

// グローバル変数宣言
LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pD3DDevice = NULL;

#define TEXT_DATA_MAX	(64)
#define TEXT_LENGTH		(128)

// デバッグデータ構造体
typedef struct textdata_t 
{
	char str[TEXT_LENGTH];			// 文字列
	int x;							// X座標
	int y;							// Y座標
} TEXTDATA;

// ローカル変数
static LPD3DXFONT g_Font;						// フォントオブジェクト
static TEXTDATA g_Array[TEXT_DATA_MAX];			// テキストデータ
static int g_ArrayCount;						// テキストデータカウント
static unsigned long g_Color;					// テキストカラー

// 関数プロトタイプ宣言
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT InitD3D(HWND hWnd);
void Initialize();
void Update(float elapsedTime);
void Draw(float elapsedTime);
void Finalize();
void Cleanup();
void InitText();

//-------------------------------------------------------------
// アプリケーションのエントリポイント
// 引数
//		hInstance     現在のインスタンスのハンドル
//		hPrevInstance 以前のインスタンスのハンドル
//		lpCmdLine	  コマンドラインパラメータ
//		nCmdShow	  ウィンドウの表示状態
// 戻り値
//		成功したら0以外の値
//-------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	// 全てのレポート出力をウィンドウに送る設定
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_WNDW);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW);
	// メモリリーク検出
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif
	HWND		hWnd;

	// ウィンドウクラスの初期化
	WNDCLASSEX	wcex = 
	{
		sizeof(WNDCLASSEX),				// この構造体のサイズ
		NULL,							// ウインドウのスタイル(default)
		WindowProc,						// メッセージ処理関数の登録
		0,								// 通常は使わないので常に0
		0,								// 通常は使わないので常に0
		hInstance,						// インスタンスへのハンドル
		NULL,							// アイコン（なし）
		LoadCursor(NULL, IDC_ARROW),	// カーソルの形
		NULL, NULL,						// 背景なし、メニューなし
		g_clsName,						// クラス名の指定
		NULL							// 小アイコン（なし）
	};

	// ウィンドウクラスの登録
	if(RegisterClassEx(&wcex) == 0)
	{
		return 0;	// 登録失敗
	}

	// 描画領域が指定サイズになるようにウィンドウサイズを設定
	RECT rect;		// 矩形
	// 矩形の値を設定
	SetRect(&rect, 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT);
	// 矩形情報によりウィンドウ矩形を計算
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	// ウィンドウの幅と高さの計算
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	hWnd = CreateWindowEx(
		NULL,						// 拡張なし
		g_clsName,
		g_winName,
		WS_OVERLAPPEDWINDOW,		// ウィンドウスタイル (標準のウィンドウを作成)
		CW_USEDEFAULT,				// ウィンドウの横方向の位置
		CW_USEDEFAULT,				// ウィンドウの縦方向の位置
		width,						// 幅
		height,						// 高さ
		NULL,
		NULL,
		hInstance,
		NULL
	);

	// ウィンドウの表示
    ShowWindow(hWnd, nCmdShow);

	// WM_PAINTが呼ばれないようにする
	ValidateRect(hWnd, 0);
	
	// Direct3Dの初期化
	if(FAILED(InitD3D(hWnd)))
	{
		return 0;	// 初期化失敗
	}

	// テキスト処理の初期設定をする。
	InitText();

	// ゲームの初期化処理を呼び出す。外部のファイルで定義すること。
	Initialize();

	// メッセージ処理および描画ループ
	// フレームレートを設定する
	LONGLONG frequency, counter, nextFrame, frameCount;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&counter);
	frameCount = frequency / FRAME_RATE;
	nextFrame = counter;

	// メッセージ処理および描画ループ
	MSG msg;
	while (1) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			// PostQuitMessage() が呼ばれたらループを終了する。
			if (msg.message == WM_QUIT)
			{
				break;
			}
			// メッセージを翻訳し，ディスパッチする。
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else 
		{	// 処理するメッセージが無いときに描画を行う。
			QueryPerformanceCounter((LARGE_INTEGER*)&counter);
			if (counter > nextFrame) 
			{
				float elapsedTime = (float)(counter - nextFrame + frameCount) / frequency;
				// ゲームの１フレームごとの更新処理を呼び出す。外部のファイルで定義すること。
				Update(elapsedTime);

				// ゲームの１フレームごとの描画処理を呼び出す。外部のファイルで定義すること。
				Draw(elapsedTime);

				nextFrame = counter + frameCount;
			}
		}
	}
	// ゲームの後始末処理を呼び出す。
	Finalize();

	// 最後の後始末をする。
	Cleanup();

	return (int) msg.wParam;
}

//-------------------------------------------------------------
// メッセージ処理用コールバック関数
// 引数
//		hWnd	ウィンドウハンドル
//		msg		メッセージ
//		wParam	メッセージの最初のパラメータ
//		lParam	メッセージの2番目のパラメータ
// 戻り値
//		メッセージ処理結果
//-------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
	{
	case WM_KEYDOWN:				// キーが押された
		// ESCキーで終了
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);		// プログラムを終了する
		}
		break;
	case WM_CLOSE:					// ウインドウが閉じられた
		PostQuitMessage(0);			// アプリケーションを終了する
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

//-------------------------------------------------------------
// Direct3Dを初期化する。
// 引数
//		hWnd	ウィンドウハンドル
// 戻り値
//		S_OK: 成功，E_FAIL:失敗
//-------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
    D3DDISPLAYMODE d3ddm;
    
	// Direct3D9オブジェクトの作成
	if((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == 0)
	{
        return E_FAIL;	// 取得失敗
    }
	
	// 現在のディスプレイモードを取得
    if(FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
	{
		return E_FAIL;
	}

	// デバイスのプレゼンテーションパラメータを初期化
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.Windowed = TRUE;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferFormat= d3ddm.Format;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	
	// ディスプレイアダプタを表すためのデバイスを作成
	// 描画と頂点処理をハードウェアで行なう
	if(FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, 
									D3DDEVTYPE_HAL, 
									hWnd, 
									D3DCREATE_HARDWARE_VERTEXPROCESSING, 
									&d3dpp, &g_pD3DDevice))) 
	{
		// 上記の設定が失敗したら
		// 描画をハードウェアで行い、頂点処理はCPUで行なう
		if(FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, 
										D3DDEVTYPE_HAL, 
										hWnd, 
										D3DCREATE_SOFTWARE_VERTEXPROCESSING, 
										&d3dpp, &g_pD3DDevice))) 
		{
			// 上記の設定が失敗したら
			// 描画と頂点処理をCPUで行なう
			if(FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, 
											D3DDEVTYPE_REF, hWnd, 
											D3DCREATE_SOFTWARE_VERTEXPROCESSING, 
											&d3dpp, &g_pD3DDevice))) 
			{
				// 初期化失敗
				return E_FAIL;
			}
		}
	}

	return S_OK;
}

//-------------------------------------------------------------
// Direct3Dオブジェクトを解放する。
// 引数
//		なし
// 戻り値
//		なし
//-------------------------------------------------------------
void Cleanup()
{
	SAFE_RELEASE(g_Font);
	SAFE_RELEASE(g_pD3DDevice);
	SAFE_RELEASE(g_pD3D);
}

//-------------------------------------------------------------
// フォントオブジェクトを作成する。
// 引数
//		なし
// 戻り値
//		なし
//-------------------------------------------------------------
void InitText()
{
	// 高さ，幅，スタイル
	D3DXCreateFont(g_pD3DDevice, 14, FW_DONTCARE, FW_BOLD, NULL, FALSE,
		SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "MS ゴシック", &g_Font);
	// 色
	g_Color = 0xFF000000;
	// データ初期化
	memset(g_Array, 0, sizeof(TEXTDATA)*TEXT_DATA_MAX);
	g_ArrayCount = 0;
}

//-------------------------------------------------------------
// 文字の色を設定する。
// 引数
//		r, g, b			色
// 戻り値
//		なし
//-------------------------------------------------------------
void SetTextColor(int r, int g, int b)
{
	g_Color = D3DCOLOR_XRGB(r, g, b);
}

//-------------------------------------------------------------
// 文字列を描画する。描画ループ内で最後に一度呼び出すこと。
// 引数
//		なし
// 戻り値
//		なし
//-------------------------------------------------------------
void DrawText(void)
{
	RECT rect;

	for (int i = 0; i < g_ArrayCount; i++) 
	{
		// 描画位置を指定
		rect.top  = g_Array[i].y;
		rect.left = g_Array[i].x;
		rect.right = 0;
		rect.bottom = 0;
		// 文字列描画領域を計算
		g_Font->DrawText(NULL, g_Array[i].str, -1, &rect, DT_CALCRECT, NULL);
		// 文字列を描画
		g_Font->DrawText(NULL, g_Array[i].str, -1, &rect, DT_LEFT | DT_TOP, g_Color);
	}

	g_ArrayCount = 0;
}

//-------------------------------------------------------------
// 編集した文字列を描画する。
// 引数
//		x, y		描画するスクリーン座標
//		format		書式
//		...			引数
// 戻り値
//		なし
//-------------------------------------------------------------
void TextPrintf(int x, int y, char *format, ...)
{
	// 配列オーバーのチェック
	if (g_ArrayCount >= TEXT_DATA_MAX - 1)
	{
		return;
	}

	va_list args;

	// 書式文字列を処理
	va_start(args, format);
	vsprintf(g_Array[g_ArrayCount].str, format, args);
	va_end(args);

	// データを保存
	g_Array[g_ArrayCount].x = x;
	g_Array[g_ArrayCount].y = y;

	g_ArrayCount++;
}

//-------------------------------------------------------------
// コンソールに文字列を編集した文字列を描画する。
// 引数
//		format		書式
//		...			引数
// 戻り値
//		なし
//-------------------------------------------------------------
void DebugPrintf(char* format, ...)
{
	char str[256];
	va_list args;

	// 書式文字列を処理
	va_start(args, format);
	vsprintf(str, format, args);
	va_end(args);

	// デバッグ出力
	OutputDebugString(str);
}
