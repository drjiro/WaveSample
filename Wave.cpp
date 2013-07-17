//
// 波サンプル
// Ver. 20090703
//
// Copyright (C) WADA Takao. All rights reserved.
//
#include <windows.h>
#include <d3dx9.h>
#include "Main.h"

// トリガー付きキー入力マクロ
#define GetTrg(n) (((~oldKeyBuffer[n] & keyBuffer[n]) & 0x80) > 0)

#define FLOOR_WIDTH		(20)
#define FLOOR_HEIGHT	(20)

//-----------------------------------------------------------------------------
// 色つき頂点構造体
//-----------------------------------------------------------------------------
typedef struct color_vertex_t 
{
	D3DXVECTOR3 v;				// 頂点
	unsigned long color;		// 色
	float tu;					// テクスチャU座標
	float tv;					// テクスチャV座標
} COLOR_VERTEX;
// 頂点フォーマット
#define FVF_COLORVERTEX	(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

//-----------------------------------------------------------------------------
// ライティングする頂点構造体
//-----------------------------------------------------------------------------
typedef struct light_vertex_t 
{
	D3DXVECTOR3 v;				// 頂点
	D3DXVECTOR3 n;				// 法線
} LIGHT_VERTEX;
// 頂点フォーマット
#define FVF_LIGHTVERTEX	(D3DFVF_XYZ|D3DFVF_NORMAL)

//-----------------------------------------------------------------------------
// メッシュ構造体
//-----------------------------------------------------------------------------
typedef struct usermesh_t 
{
    LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer;		// 頂点バッファ
    LPDIRECT3DINDEXBUFFER9  m_IndexBuffer;		// インデックスバッファ
    LPDIRECT3DTEXTURE9 m_pTexture;				// テクスチャ
	unsigned long m_Color;						// 色
	unsigned long m_NumVertices;				// 頂点数
	unsigned long m_NumIndices;					// インデックス数
	int m_NumGrid;								// グリッド数
	int m_Tile;									// タイル数
	float m_Width;								// 幅
	float m_Height;								// 高さ
} USERMESH;

//-----------------------------------------------------------------------------
// 球メッシュ構造体
//-----------------------------------------------------------------------------
typedef struct spheremesh_t 
{
    LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer;		// 頂点バッファ
    LPDIRECT3DINDEXBUFFER9  m_IndexBuffer;		// インデックスバッファ
	unsigned long m_NumVertices;				// 頂点数
	unsigned long m_NumIndices;					// インデックス数
	float m_Radius;								// 半径
	int m_NumVertical;							// 垂直分割数
	int m_NumHorizontal;						// 水平分割数
	D3DMATERIAL9 m_Material;					// マテリアル
} SPHEREMESH;

// カメラ構造体
typedef struct camera_t 
{
	D3DXVECTOR3 m_Eye;		// 視点
	D3DXVECTOR3 m_Target;	// 対象点
	D3DXVECTOR3 m_Up;		// 上向きベクトル
	float m_Fov;			// FOV
} CAMERA;

// ゲームオブジェクト構造体
typedef struct object_t 
{
	D3DXVECTOR3 m_Pos;		// 位置
	SPHEREMESH m_Sphere;	// 球メッシュ
} OBJECT;

// グローバル変数宣言
static USERMESH g_Ground;					// ユーザメッシュによる地面用メッシュ
static OBJECT g_Sphere;						// 球メッシュオブジェクト

static LPDIRECT3DTEXTURE9* texture;			// 地面用テクスチャ
static CAMERA g_Camera;						//	カメラ
// g_Type タイプ(0:定常波 1:進行波)
// g_Omega: 角速度
// g_Amplitude: 振幅
// g_WaveNumber: 波数
// g_Phase:位相のずれ
static int g_Type = 0;
static float g_Omega = 2 * D3DX_PI / 1;
static float g_Amplitude = 1;
static float g_WaveNumber = 1;
static float g_Phase = 0;
static float g_TotalElapsedTime = 0;		// 合計経過時間

static D3DLIGHT9 g_Light;					// ライト

// プロトタイプ宣言
// ユーザメッシュ用
static float HeightField(float x, float z);
static int CreateUserMesh(USERMESH *mesh, LPDIRECT3DTEXTURE9 texture,
				 float width, float height, unsigned short numGrid, unsigned long color);
static int Transform(USERMESH *mesh, float elapsedTime);
static void LoadTexture(LPDIRECT3DTEXTURE9* texture, char* fileName);
static void DrawUserMesh(USERMESH *mesh);
static void FinalizeUserMesh(USERMESH* mesh);

static void UpdateCamera(unsigned char* keyBuffer);
static void SetupCamera();

static int CreateSphereMesh(SPHEREMESH *mesh, float radius, int numVertical, int numHorizontal);
static void DrawSphereMesh(SPHEREMESH *mesh);
static void FinalizeSphereMesh(SPHEREMESH *mesh);
static void CreateLight();

//-------------------------------------------------------------
// ゲームループ前に初期化する。
// 引数
//		なし
// 戻り値
//		なし
//-------------------------------------------------------------
void Initialize()
{
	// 自前でメッシュを作成する。
	CreateUserMesh(&g_Ground, 0, FLOOR_WIDTH, FLOOR_HEIGHT, 20, 0xFFFFFFFF);
	// テクスチャをロードする。
	LoadTexture(&g_Ground.m_pTexture, "star_water_090121.png");

	// 自前で球メッシュを作成する。
	CreateSphereMesh(&g_Sphere.m_Sphere, 1, 8, 8);

	// カメラ
	g_Camera.m_Eye = D3DXVECTOR3(0.0f, 50.0f, -100.0f);		// 視点
	g_Camera.m_Target = D3DXVECTOR3(0.0f, 0.0f, 0.0f);		// 対象点
	g_Camera.m_Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);			// 上向きベクトル
	g_Camera.m_Fov = D3DX_PI / 4;

	// ライトを作成する。
	CreateLight();
}

//-------------------------------------------------------------
// フレームごとに更新する。
// 引数
//		elapsedTime			経過時間(秒)
// 戻り値
//		なし
//-------------------------------------------------------------
void Update(float elapsedTime)
{
	unsigned char keyBuffer[256];
	static unsigned char oldKeyBuffer[256];

	// キーボード情報を読み込む。
	GetKeyboardState(keyBuffer);

	if (GetTrg(VK_F1)) 
	{
		// 定常波にする。
		g_Type = 0;
	}
	else if (GetTrg(VK_F2)) 
	{
		// 進行波にする。
		g_Type = 1;
	}
	if (GetTrg(VK_F3)) 
	{
		// 波数を増やす。
		g_WaveNumber++;
	}
	else if (GetTrg(VK_F4)) 
	{
		// 波数を減らす。
		if (g_WaveNumber > 1) 
		{
			g_WaveNumber--;
		}
	}
	if (GetTrg(VK_F5)) {
		// 振幅を増やす。
		g_Amplitude++;
	}
	else if (GetTrg(VK_F6)) 
	{
		// 振幅を減らす。
		g_Amplitude--;
		if (g_Amplitude < 0)
		{
			g_Amplitude = 0;
		}
	}
	if (GetTrg(VK_F7)) 
	{
		// 位相を増やす。180°まで
		g_Phase += D3DX_PI / 6;
		if (g_Phase > D3DX_PI) 
		{
			g_Phase = D3DX_PI;
		}
	}
	else if (GetTrg(VK_F8)) 
	{
		// 位相を減らす。-180°まで
		g_Phase -= D3DX_PI / 6;
		if (g_Phase < -D3DX_PI) 
		{
			g_Phase = -D3DX_PI;
		}
	}
	if (GetTrg('Z')) 
	{
		// 角速度を増やす。
		g_Omega += D3DX_PI / 6;
	}
	else if (GetTrg('X')) 
	{
		// 角速度を減らす。
		g_Omega -= D3DX_PI / 6;
		if (g_Omega < 0) 
		{
			g_Omega = 0;
		}
	}
	if ((keyBuffer['A'] & 0x80) != 0) 
	{
		// 左へ
		g_Sphere.m_Pos.x -= 0.1f;
	}
	else if ((keyBuffer['D'] & 0x80) != 0) 
	{
		// 右へ
		g_Sphere.m_Pos.x += 0.1f;
	}
	if ((keyBuffer['W'] & 0x80) != 0) 
	{
		// 奥へ
		g_Sphere.m_Pos.z += 0.1f;
	}
	else if ((keyBuffer['S'] & 0x80) != 0) 
	{
		// 手前へ
		g_Sphere.m_Pos.z -= 0.1f;
	}

	TextPrintf(0, 0, "タイプ=%s", g_Type == 0 ? "定常波(F1)" : "進行波(F2)");
	TextPrintf(0, 20, "波数=%.1f (F3:+ F4:-)", g_WaveNumber);
	TextPrintf(0, 40, "振幅=%.1f (F5:+ F6:-)", g_Amplitude);
	TextPrintf(0, 60, "位相=%.1f (F7:+ F8:-)", g_Phase);
	TextPrintf(0, 80, "角速度=%.1f (Z:+ X:-)", g_Omega);
	TextPrintf(0, 100, "時間=%.3f ボール X=%.1f Y=%.1f Z=%.1f",
		g_TotalElapsedTime, g_Sphere.m_Pos.x, g_Sphere.m_Pos.y, g_Sphere.m_Pos.z);
	TextPrintf(0, 120, "オブジェクトの移動:ADWS");
	TextPrintf(0, 140, "カメラ回転:←→");
	TextPrintf(0, 160, "カメラズーム:↑↓");


	// 変形する。
	Transform(&g_Ground, elapsedTime);

	// カメラを更新する。
	UpdateCamera(keyBuffer);

	// 前のキー状態を覚えておく。
	memcpy(oldKeyBuffer, keyBuffer, 256);
}

//-------------------------------------------------------------
// カメラを更新する。
// 引数
//		keyBuffer		キーボードバッファ
// 戻り値
//		なし
//-------------------------------------------------------------
static void UpdateCamera(unsigned char* keyBuffer)
{
	float rad = 0;
	// ←キー
	if ((keyBuffer[VK_LEFT] & 0x80) > 0)
	{
		// カメラを回転する。
		rad = D3DXToRadian(-1);
	}
	else if ((keyBuffer[VK_RIGHT] & 0x80) > 0)
	{
		// カメラを回転する。
		rad = D3DXToRadian(1);
	}	
	//else if ((keyBuffer[VK_UP] & 0x80) > 0)
	//{
	//	// ズームインする。
	//	if (g_Camera.m_Fov > 0) {
	//		g_Camera.m_Fov -= D3DXToRadian(1);
	//	}
	//}
	//else if ((keyBuffer[VK_DOWN] & 0x80) > 0) 
	//{
	//	// ズームアウトする。
	//	if (g_Camera.m_Fov < D3DX_PI) {
	//		g_Camera.m_Fov += D3DXToRadian(1);
	//	}
	//}	
	else if ((keyBuffer[VK_UP] & 0x80) > 0) 
	{
		// ズームインする。
		g_Camera.m_Eye = (g_Camera.m_Eye - g_Camera.m_Target) * 0.9f;
	}
	else if ((keyBuffer[VK_DOWN] & 0x80) > 0) 
	{
		// ズームアウトする。
		g_Camera.m_Eye = (g_Camera.m_Eye - g_Camera.m_Target) * 1.1f;
	}	
	// カメラの向きベクトルの逆を計算する。
	D3DXVECTOR3 dir = g_Camera.m_Eye - g_Camera.m_Target;
	// 回転行列を作成する。
	D3DXMATRIX matRotY;
	D3DXMatrixRotationY(&matRotY, rad);
	// カメラの向きベクトルを回転行列で回転する。
	D3DXVec3TransformCoord(&dir, &dir, &matRotY);
	// カメラの視点を設定する。
	g_Camera.m_Eye = g_Camera.m_Target + dir;
}

//-------------------------------------------------------------
// フレームごとの描画
// 引数
//		elapsedTime			経過時間(秒)
// 戻り値
//		なし
//-------------------------------------------------------------
void Draw(float elapsedTime)
{
	// カメラを設定する。
	SetupCamera();

	// 画面をクリアする。
	g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0, 255), 1.0f, 0);
	// 描画を開始する。
	if(SUCCEEDED(g_pD3DDevice->BeginScene())) 
	{
		// Zバッファを有効にする。
		g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		// 反時計回りにカリングする。
		g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		// ライトをオフにする。
		g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

		// 地面をレンダリングする。
		D3DXMATRIX matWorld;		// ワールド変換行列
		// ワールド座標変換
		D3DXMatrixIdentity(&matWorld);
		g_pD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);
		// 地面を描画する。
		DrawUserMesh(&g_Ground);

		// ライトをオンにする。
		g_pD3DDevice->SetLight(0, &g_Light);
		g_pD3DDevice->LightEnable(0, TRUE);	
		g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
		// スペキュラを有効にする。
		g_pD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
		// アンビエントライトを設定する。
		g_pD3DDevice->SetRenderState(D3DRS_AMBIENT, 0x55555555);

		// テクスチャステージの設定 
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		// アルファブレンディングを設定する。
		g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		// α合成する。
		g_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		g_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// 球を描画する。
		D3DXMATRIX matTrans;
		// Y座標をHeight-Fieldから計算する。
		g_Sphere.m_Pos.y = HeightField(g_Sphere.m_Pos.x, g_Sphere.m_Pos.z) + g_Sphere.m_Sphere.m_Radius;
		D3DXMatrixTranslation(&matTrans, g_Sphere.m_Pos.x, g_Sphere.m_Pos.y, g_Sphere.m_Pos.z);
		g_pD3DDevice->SetTransform(D3DTS_WORLD, &matTrans);
		DrawSphereMesh(&g_Sphere.m_Sphere);

		// デバッグ文字列を実際に描画する。
		DrawText();

		// 描画を終了する。
		g_pD3DDevice->EndScene();
	}
	// 実画面に反映する。
	g_pD3DDevice->Present(NULL, NULL, NULL, NULL);
}

//-------------------------------------------------------------
// カメラの設定
// 引数
//		なし
// 戻り値
//		なし
//-------------------------------------------------------------
static void SetupCamera()
{
	D3DXMATRIX matView;
	D3DXMATRIX matProj;

	// ビュー座標変換
	D3DXMatrixLookAtLH(&matView,
		&g_Camera.m_Eye,
		&g_Camera.m_Target,
		&g_Camera.m_Up);
	g_pD3DDevice->SetTransform(D3DTS_VIEW, &matView);

	// 射影変換
	D3DXMatrixPerspectiveFovLH(&matProj,
		g_Camera.m_Fov,
		(float)CLIENT_WIDTH / (float)CLIENT_HEIGHT, 1.0f, 300.0f);
	g_pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

//-------------------------------------------------------------
// 後始末する。
// 引数
//		なし
// 戻り値
//		なし
//-------------------------------------------------------------
void Finalize()
{
	FinalizeSphereMesh(&g_Sphere.m_Sphere);
	FinalizeUserMesh(&g_Ground);
}


//-------------------------------------------------------------
// 正方形のメッシュを作成する。
// 引数:
//		mesh		メッシュ
//		texture		テクスチャ
//		width		幅
//		height		高さ
//		m_NumGrid		グリッド数
//		color		色
// 戻り値:
//		0・・・戻り値, -1・・・エラー
//-------------------------------------------------------------
static int CreateUserMesh(USERMESH *mesh, LPDIRECT3DTEXTURE9 texture,
				 float width, float height, unsigned short numGrid, unsigned long color)
{
	// グリッドの数から頂点数を計算する。
	int numVertices = (numGrid + 1) * (numGrid + 1);
	// グリッドの数からインデックス数を計算する。
	int numIndices = (numGrid * numGrid) * 6;
	// 幅ごとの繰り返し数を計算する。
	int tile = (unsigned short)width / numGrid;

    // 頂点バッファを作成する。
	LPDIRECT3DVERTEXBUFFER9 vb;
	HRESULT hr = g_pD3DDevice->CreateVertexBuffer(numVertices * sizeof(COLOR_VERTEX),	// バッファサイズ
                                    D3DUSAGE_WRITEONLY,	// 書き込み可能
									FVF_COLORVERTEX,	// 頂点フォーマット
                                    D3DPOOL_MANAGED,	// DirectXに任せる
									&vb,				// 頂点バッファ
									NULL);
	if (FAILED(hr)) 
	{
        return -1;
	}

    // 頂点バッファを埋める。
    COLOR_VERTEX *vertices;
	// 頂点バッファをロックする。
	if (FAILED(vb->Lock(0, 0, (void **)&vertices, NULL))) 
	{
        return -1;
	}

    // 頂点をセットする。
    for (int z = 0; z <= numGrid; z++) 
	{
        for (int x = 0; x <= numGrid; x++) 
		{
            //vertices->v.x = (width * x / (float)numGrid - width / 2) * D3DX_PI / (mesh->m_NumGrid + 1);
            //vertices->v.z = (height * z / (float)numGrid - height / 2) * D3DX_PI / (mesh->m_NumGrid + 1);
            vertices->v.x = width * (x * D3DX_PI / (numGrid + 1) - D3DX_PI / 2);
            vertices->v.z = height * (z * D3DX_PI / (numGrid + 1) - D3DX_PI / 2);
			vertices->v.y = HeightField(vertices->v.x, vertices->v.z);
            vertices->color = color;
			vertices->tu = x * tile / (float)numGrid;
            vertices->tv = z * tile / (float)numGrid;
            vertices++;
        }
    }
	// 頂点バッファをアンロックする。
	vb->Unlock();

    // インデックスバッファを作成する。
    LPDIRECT3DINDEXBUFFER9 ib;		// インデックスバッファ
    hr = g_pD3DDevice->CreateIndexBuffer(numIndices * sizeof(unsigned short),	// バッファサイズ
                                          D3DUSAGE_WRITEONLY,	// 書き込み可能
                                          D3DFMT_INDEX16,		// 16ビットのインデックス
										  D3DPOOL_MANAGED,		// DirectXに任せる
                                          &ib,					// インデックスバッファ
										  NULL);
	if (FAILED(hr))
	{
        return -1;
	}

    // インデックスバッファを埋める。
    unsigned short *indices;
	// インデックスバッファをロックする。
	ib->Lock(0, 0, (void **)&indices, 0);
	if (FAILED(hr)) 
	{
        return -1;
	}

    // インデックスをセットする。
    for (int z = 0; z < numGrid; z++) 
	{
        for (int x = 0; x < numGrid; x++) 
		{
            unsigned short idx = x + z * (numGrid + 1);
            *indices++ = (unsigned short)(idx + 1);
            *indices++ = (unsigned short)idx;
            *indices++ = (unsigned short)(idx + numGrid + 1);
            *indices++ = (unsigned short)(idx + 1);
            *indices++ = (unsigned short)(idx + numGrid + 1);
            *indices++ = (unsigned short)(idx + 1 + numGrid + 1);
        }
    }
	// インデックスバッファをアンロックする。
    ib->Unlock();

	mesh->m_Width = width;
	mesh->m_Height = height;
	mesh->m_NumGrid = numGrid;
	mesh->m_NumVertices = numVertices;
	mesh->m_NumIndices = numIndices;
	mesh->m_VertexBuffer = vb;
	mesh->m_IndexBuffer = ib;
	mesh->m_pTexture = texture;
	mesh->m_Color = color;

	return 0;
}

//-------------------------------------------------------------
// 頂点をモーフィングする。
// 引数:
//		mesh		ユーザメッシュ
//		elapsedTime	経過時間
// 戻り値:
//		成功したら0，エラーなら-1
//-------------------------------------------------------------
static int Transform(USERMESH *mesh, float elapsedTime)
{
	g_TotalElapsedTime += elapsedTime * 0.5f;
    COLOR_VERTEX *vertices;
	// 頂点をロックする。
	if (FAILED(mesh->m_VertexBuffer->Lock(0, 0, (void **)&vertices, NULL)))
	{
        return -1;
	}

	// 頂点をセットする。
    for (int z = 0; z <= mesh->m_NumGrid; z++) 
	{
        for (int x = 0; x <= mesh->m_NumGrid; x++)
		{
			// 頂点のY座標をHight-Field法で計算する。
			vertices->v.y = HeightField(vertices->v.x, vertices->v.z);
			vertices++;
        }
    }
	// 頂点をアンロックする。
	mesh->m_VertexBuffer->Unlock();

	return 0;
}

//-------------------------------------------------------------
// テクスチャをロードする。
// 引数
//		texture		テクスチャインターフェイス
//		fileName	テクスチャファイル名
// 戻り値
//		なし
//-------------------------------------------------------------
static void LoadTexture(LPDIRECT3DTEXTURE9* texture, char* fileName)
{
	if (D3DXCreateTextureFromFileEx 
		(
			g_pD3DDevice,			// Direct3Dデバイス
			fileName,				// テクスチャファイル名
			0,						// 幅（ピクセル）, 0はファイルから取得
			0,						// 高さ（ピクセル）, 0はファイルから取得
			0,						// ミップレベルの数。0は完全なミップマップ
			0,						// 使い方。
			D3DFMT_UNKNOWN,			// ピクセルフォーマット。D3DFMT_UNKNOWNはファイルから取得
			D3DPOOL_MANAGED,		// 作成先。
			D3DX_DEFAULT,			// フィルタリングの方法
			D3DX_DEFAULT,			// ミップフィルタリングの方法
			0xFF000000,				// カラー・キー（抜き色）
			NULL,					// 画像ファイル内の記述。
			NULL,					// パレット
			texture					// テクスチャインターフェイス
		) != 0) 
	{
		MessageBox(NULL, "テクスチャが見つかりません。", "エラー", MB_OK);
	}
}

//-----------------------------------------------------------------------------
// Height Field法による波を生成する。
// 引数:
//		x, z		X,Z座標
// 戻り値:
//		Y座標
//-----------------------------------------------------------------------------
float HeightField(float x, float z)
{
	float y = 0;
	x /= FLOOR_WIDTH;
	z /= FLOOR_HEIGHT;
	if (g_Type == 0) 
	{
		// 定常波
		y =  g_Amplitude * sinf(g_Omega * g_TotalElapsedTime - g_WaveNumber * x - g_Phase)
			+ g_Amplitude * sinf(g_Omega * g_TotalElapsedTime + g_WaveNumber * x + g_Phase)
			+ g_Amplitude * sinf(g_Omega * g_TotalElapsedTime - g_WaveNumber * z - g_Phase)
			+ g_Amplitude * sinf(g_Omega * g_TotalElapsedTime + g_WaveNumber * z + g_Phase);
	}
	else 
	{
		// 進行波
		y = g_Amplitude * sinf(g_Omega * g_TotalElapsedTime - g_WaveNumber * x - g_Phase);
	}
	return y;
}


//-----------------------------------------------------------------------------
// ユーザメッシュを描画する。
// 引数:
//		mesh		メッシュ
// 戻り値:
//		なし
//-----------------------------------------------------------------------------
static void DrawUserMesh(USERMESH *mesh)
{
	// レンダリングする。
	g_pD3DDevice->SetTexture(0, mesh->m_pTexture);
    g_pD3DDevice->SetFVF(FVF_COLORVERTEX);
    g_pD3DDevice->SetStreamSource(0, mesh->m_VertexBuffer, 0, sizeof(COLOR_VERTEX));
    g_pD3DDevice->SetIndices(mesh->m_IndexBuffer);
    g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
							0, 0, mesh->m_NumVertices, 0, mesh->m_NumIndices / 3);
	g_pD3DDevice->SetTexture(0, NULL);
}

//-----------------------------------------------------------------------------
// ユーザメッシュを解放する。
// 引数:
//		mesh		メッシュ
// 戻り値:
//		なし
//-----------------------------------------------------------------------------
static void FinalizeUserMesh(USERMESH *mesh)
{
	if (mesh->m_pTexture != NULL)
	{
		mesh->m_pTexture->Release();
		mesh->m_pTexture = NULL;
	}

	if (mesh->m_VertexBuffer != NULL)
	{
		mesh->m_VertexBuffer->Release();
		mesh->m_VertexBuffer = NULL;
	}

	if (mesh->m_IndexBuffer != NULL) 
	{
		mesh->m_IndexBuffer->Release();
		mesh->m_IndexBuffer = NULL;
	}
}

//-----------------------------------------------------------------------------
// 自前で球のメッシュを作成する。
// X,Yが水平方向　Zは垂直方向
// 引数:
//		mesh				球メッシュ
//		radius				半径
//		numVertical			垂直分割数
//		numHorizontal		水平分割数
// 戻り値:
//		0・・・戻り値, -1・・・エラー
//-----------------------------------------------------------------------------
static int CreateSphereMesh(SPHEREMESH *mesh, float radius, int numVertical, int numHorizontal)
{
	float theta;			// θ
	float rcosphi;			// r * cosφ
	float x,y,z;
	// 分割数から頂点数を計算する。
	int numVertices = (numVertical - 1) * numHorizontal + 2;
	// 分割数から面数を計算する。
	int numFaces = (numVertical - 2) * (numHorizontal * 2) + numHorizontal * 2;
	// 面数からインデックス数を計算する。
	int numIndices = numFaces * 3;

    // 頂点バッファを作成する。
	LPDIRECT3DVERTEXBUFFER9 vb;
	HRESULT hr = g_pD3DDevice->CreateVertexBuffer(numVertices * sizeof(LIGHT_VERTEX),	// バッファサイズ
                                    D3DUSAGE_WRITEONLY,	// 書き込み可能
									FVF_COLORVERTEX,	// 頂点フォーマット
                                    D3DPOOL_MANAGED,	// DirectXに任せる
									&vb,				// 頂点バッファ
									NULL);
	if (FAILED(hr))
	{
        return -1;
	}

    // 頂点バッファを埋める。
    LIGHT_VERTEX *vertices;
	// 頂点バッファをロックする。
	if (FAILED(vb->Lock(0, 0, (void **)&vertices, NULL))) 
	{
        return -1;
	}

	// 上の端の頂点（最初の頂点）を作成する
	vertices->v = D3DXVECTOR3(0.0f, radius, 0.0f);
	vertices->n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	vertices++;

	// 中間のリングの頂点を作成する
	float dphi = D3DX_PI / numVertical;		// それぞれのリングの垂直方向の角度
	float dtheta = 2 * D3DX_PI / numHorizontal;	// それぞれの区画の水平方向の角度
	float phi = D3DX_PI / 2 - dphi;			// 最初の垂直方向の角度

	// 2つ目の頂点から最後の1つ前まで設定する
	int n = 1;
	// 垂直方向の分割のループ
	for (int i=0; i < numVertical - 1; i++) 
	{
		// Y座標を計算する
		y = sinf(phi);
		// cosφをループ内で毎回計算しないでいいように先に計算しておく
		rcosphi = cosf(phi);

		theta = 0.0f;
		// 水平方向の分割ループ			// 最初の水平方向の角度
		for	(int j = 0; j < numHorizontal; j++) 
		{
			// X座標を計算する
			x = cosf(theta) * rcosphi;
			// Z座標を計算する
			z = sinf(theta) * rcosphi;
			// 座標を設定する
			vertices->v = D3DXVECTOR3(radius * x, radius * y, radius * z);
			// 法線を設定する
			vertices->n = D3DXVECTOR3(x, y, z);
			// 正規化する。
			D3DXVec3Normalize(&vertices->n, &vertices->n);
			vertices++;

			theta += dtheta;		// 次の水平位置へ
			++n;
		}
		phi -=dphi;					// 次の垂直位置へ
	}

	// 下の端の頂点（最後の頂点）を作成する
	vertices->v = D3DXVECTOR3(0.0f, -radius, 0.0f);
	vertices->n = D3DXVECTOR3(0.0f, -1.0f, 0.0f);


	// 頂点バッファをアンロックする。
	vb->Unlock();


    // インデックスバッファを作成する。
    LPDIRECT3DINDEXBUFFER9 ib;		// インデックスバッファ
    hr = g_pD3DDevice->CreateIndexBuffer(numIndices * sizeof(unsigned short),	// バッファサイズ
                                          D3DUSAGE_WRITEONLY,	// 書き込み可能
                                          D3DFMT_INDEX16,		// 16ビットのインデックス
										  D3DPOOL_MANAGED,		// DirectXに任せる
                                          &ib,					// インデックスバッファ
										  NULL);
	if (FAILED(hr)) 
	{
        return -1;
	}

    // インデックスバッファを埋める。
    unsigned short *indices;
	// インデックスバッファをロックする。
	ib->Lock(0, 0, (void **)&indices, 0);
	if (FAILED(hr))
	{
        return -1;
	}

	// 上と下の三角形を生成する
	for (int i = 0; i < numHorizontal; i++)
	{
		// 上
		*indices++ = (unsigned short)0;
		*indices++ = (unsigned short)(1 + ((1 + i) % numHorizontal));
		*indices++ = (unsigned short)(i + 1);
		// 下
		*indices++ = (unsigned short)(numVertices - 1);
		*indices++ = (unsigned short)(numVertices - 1 - ((1 + i) % numHorizontal));
		*indices++ = (unsigned short)(numVertices - 2 - i);
	}

	// 中間のリングの三角形を生成する。この場合は四角になるので２つずつ作る。
	int m = 1;				// 一番上の頂点から次の点を使う
	for (int i = 0; i < numVertical - 2; i++) 
	{
		for (int j = 0; j < numHorizontal; j++) 
		{
			// 1つ目の三角形
			*indices++ = (unsigned short)(m + j);
			*indices++ = (unsigned short)(m + ((j + 1) % numHorizontal));
			*indices++ = (unsigned short)(m + numHorizontal + ((j + 1) % numHorizontal));
			// 2つ目の三角形
			*indices++ = (unsigned short)(m + j);
			*indices++ = (unsigned short)(m + numHorizontal + ((j + 1) % numHorizontal));
			*indices++ = (unsigned short)(m + numHorizontal + j);
		}
		m += numHorizontal;
	}

	// インデックスバッファをアンロックする。
    ib->Unlock();

	mesh->m_Material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mesh->m_Material.Ambient = mesh->m_Material.Diffuse;
	mesh->m_Material.Specular = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);
	mesh->m_Material.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	mesh->m_Material.Power = 20.0f;

	mesh->m_NumVertices = numVertices;
	mesh->m_NumIndices = numIndices;
	mesh->m_VertexBuffer = vb;
	mesh->m_IndexBuffer = ib;
	mesh->m_NumVertical = numVertical;
	mesh->m_NumHorizontal = numHorizontal;
	mesh->m_Radius = radius;

	return 0;
}

//-----------------------------------------------------------------------------
// 球メッシュを描画する。
// 引数:
//		mesh		球メッシュ
// 戻り値:
//		なし
//-----------------------------------------------------------------------------
static void DrawSphereMesh(SPHEREMESH *mesh)
{
	// レンダリングする。
	// テクスチャなし
	g_pD3DDevice->SetTexture(0, NULL);
	g_pD3DDevice->SetMaterial(&mesh->m_Material);
    g_pD3DDevice->SetFVF(FVF_LIGHTVERTEX);
    g_pD3DDevice->SetStreamSource(0, mesh->m_VertexBuffer, 0, sizeof(LIGHT_VERTEX));
    g_pD3DDevice->SetIndices(mesh->m_IndexBuffer);
    g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
							0, 0, mesh->m_NumVertices, 0, mesh->m_NumIndices / 3);
}


//-----------------------------------------------------------------------------
// 球メッシュを解放する。
// 引数:
//		mesh		球メッシュ
// 戻り値:
//		なし
//-----------------------------------------------------------------------------
static void FinalizeSphereMesh(SPHEREMESH *mesh)
{
	if (mesh->m_VertexBuffer != NULL)
	{
		mesh->m_VertexBuffer->Release();
		mesh->m_VertexBuffer = NULL;
	}

	if (mesh->m_IndexBuffer != NULL)
	{
		mesh->m_IndexBuffer->Release();
		mesh->m_IndexBuffer = NULL;
	}
}

//-------------------------------------------------------------
// ライトを作成する。
// 引数
//		なし
// 戻り値
//		なし
//-------------------------------------------------------------
static void CreateLight()
{
	// ライトを設定する。
	D3DXVECTOR3 direction(-1, -1, 1);
    ZeroMemory(&g_Light, sizeof(D3DLIGHT9));
    g_Light.Type = D3DLIGHT_DIRECTIONAL;
    g_Light.Diffuse.r = 1.0f;
    g_Light.Diffuse.g = 1.0f;
    g_Light.Diffuse.b = 1.0f;
	//g_Light.Ambient = g_Light.Diffuse;
	g_Light.Specular.r = 1.0f;
	g_Light.Specular.g = 1.0f;
	g_Light.Specular.b = 1.0f;
    D3DXVec3Normalize((D3DXVECTOR3*)&g_Light.Direction, &direction);
    g_Light.Range = 200.0f;
}