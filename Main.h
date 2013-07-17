#ifndef _MAIN_H_
#define _MAIN_H_

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = NULL; } }
#define SAFE_DELETE(p) { if(p) { delete (p); (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p) = NULL; } }

#define FRAME_RATE		60
#define CLIENT_WIDTH	640	// クライアント領域のデフォルトの幅, フルスクリーンの場合は水平方向解像度
#define	CLIENT_HEIGHT	480	// クライアント領域のデフォルトの高さ, フルスクリーンの場合は垂直方向解像度

// グローバル変数宣言
extern LPDIRECT3DDEVICE9 g_pD3DDevice;

// デバッグ文字列表示
void SetTextColor(int r, int g, int b);			// 文字の色の変更
void DrawText(void);							// テキストの描画

void TextPrintf(int x, int y, char*format, ...);	// 編集した文字列の描画
void DebugPrintf(char* format, ...);				// コンソールに編集した文字列を描画

#endif
