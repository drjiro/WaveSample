//
// �g�T���v��
// Ver. 20090703
//
// Copyright (C) WADA Takao. All rights reserved.
//
#include <windows.h>
#include <d3dx9.h>
#include "Main.h"

// �g���K�[�t���L�[���̓}�N��
#define GetTrg(n) (((~oldKeyBuffer[n] & keyBuffer[n]) & 0x80) > 0)

#define FLOOR_WIDTH		(20)
#define FLOOR_HEIGHT	(20)

//-----------------------------------------------------------------------------
// �F�����_�\����
//-----------------------------------------------------------------------------
typedef struct color_vertex_t 
{
	D3DXVECTOR3 v;				// ���_
	unsigned long color;		// �F
	float tu;					// �e�N�X�`��U���W
	float tv;					// �e�N�X�`��V���W
} COLOR_VERTEX;
// ���_�t�H�[�}�b�g
#define FVF_COLORVERTEX	(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

//-----------------------------------------------------------------------------
// ���C�e�B���O���钸�_�\����
//-----------------------------------------------------------------------------
typedef struct light_vertex_t 
{
	D3DXVECTOR3 v;				// ���_
	D3DXVECTOR3 n;				// �@��
} LIGHT_VERTEX;
// ���_�t�H�[�}�b�g
#define FVF_LIGHTVERTEX	(D3DFVF_XYZ|D3DFVF_NORMAL)

//-----------------------------------------------------------------------------
// ���b�V���\����
//-----------------------------------------------------------------------------
typedef struct usermesh_t 
{
    LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer;		// ���_�o�b�t�@
    LPDIRECT3DINDEXBUFFER9  m_IndexBuffer;		// �C���f�b�N�X�o�b�t�@
    LPDIRECT3DTEXTURE9 m_pTexture;				// �e�N�X�`��
	unsigned long m_Color;						// �F
	unsigned long m_NumVertices;				// ���_��
	unsigned long m_NumIndices;					// �C���f�b�N�X��
	int m_NumGrid;								// �O���b�h��
	int m_Tile;									// �^�C����
	float m_Width;								// ��
	float m_Height;								// ����
} USERMESH;

//-----------------------------------------------------------------------------
// �����b�V���\����
//-----------------------------------------------------------------------------
typedef struct spheremesh_t 
{
    LPDIRECT3DVERTEXBUFFER9 m_VertexBuffer;		// ���_�o�b�t�@
    LPDIRECT3DINDEXBUFFER9  m_IndexBuffer;		// �C���f�b�N�X�o�b�t�@
	unsigned long m_NumVertices;				// ���_��
	unsigned long m_NumIndices;					// �C���f�b�N�X��
	float m_Radius;								// ���a
	int m_NumVertical;							// ����������
	int m_NumHorizontal;						// ����������
	D3DMATERIAL9 m_Material;					// �}�e���A��
} SPHEREMESH;

// �J�����\����
typedef struct camera_t 
{
	D3DXVECTOR3 m_Eye;		// ���_
	D3DXVECTOR3 m_Target;	// �Ώۓ_
	D3DXVECTOR3 m_Up;		// ������x�N�g��
	float m_Fov;			// FOV
} CAMERA;

// �Q�[���I�u�W�F�N�g�\����
typedef struct object_t 
{
	D3DXVECTOR3 m_Pos;		// �ʒu
	SPHEREMESH m_Sphere;	// �����b�V��
} OBJECT;

// �O���[�o���ϐ��錾
static USERMESH g_Ground;					// ���[�U���b�V���ɂ��n�ʗp���b�V��
static OBJECT g_Sphere;						// �����b�V���I�u�W�F�N�g

static LPDIRECT3DTEXTURE9* texture;			// �n�ʗp�e�N�X�`��
static CAMERA g_Camera;						//	�J����
// g_Type �^�C�v(0:���g 1:�i�s�g)
// g_Omega: �p���x
// g_Amplitude: �U��
// g_WaveNumber: �g��
// g_Phase:�ʑ��̂���
static int g_Type = 0;
static float g_Omega = 2 * D3DX_PI / 1;
static float g_Amplitude = 1;
static float g_WaveNumber = 1;
static float g_Phase = 0;
static float g_TotalElapsedTime = 0;		// ���v�o�ߎ���

static D3DLIGHT9 g_Light;					// ���C�g

// �v���g�^�C�v�錾
// ���[�U���b�V���p
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
// �Q�[�����[�v�O�ɏ���������B
// ����
//		�Ȃ�
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
void Initialize()
{
	// ���O�Ń��b�V�����쐬����B
	CreateUserMesh(&g_Ground, 0, FLOOR_WIDTH, FLOOR_HEIGHT, 20, 0xFFFFFFFF);
	// �e�N�X�`�������[�h����B
	LoadTexture(&g_Ground.m_pTexture, "star_water_090121.png");

	// ���O�ŋ����b�V�����쐬����B
	CreateSphereMesh(&g_Sphere.m_Sphere, 1, 8, 8);

	// �J����
	g_Camera.m_Eye = D3DXVECTOR3(0.0f, 50.0f, -100.0f);		// ���_
	g_Camera.m_Target = D3DXVECTOR3(0.0f, 0.0f, 0.0f);		// �Ώۓ_
	g_Camera.m_Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);			// ������x�N�g��
	g_Camera.m_Fov = D3DX_PI / 4;

	// ���C�g���쐬����B
	CreateLight();
}

//-------------------------------------------------------------
// �t���[�����ƂɍX�V����B
// ����
//		elapsedTime			�o�ߎ���(�b)
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
void Update(float elapsedTime)
{
	unsigned char keyBuffer[256];
	static unsigned char oldKeyBuffer[256];

	// �L�[�{�[�h����ǂݍ��ށB
	GetKeyboardState(keyBuffer);

	if (GetTrg(VK_F1)) 
	{
		// ���g�ɂ���B
		g_Type = 0;
	}
	else if (GetTrg(VK_F2)) 
	{
		// �i�s�g�ɂ���B
		g_Type = 1;
	}
	if (GetTrg(VK_F3)) 
	{
		// �g���𑝂₷�B
		g_WaveNumber++;
	}
	else if (GetTrg(VK_F4)) 
	{
		// �g�������炷�B
		if (g_WaveNumber > 1) 
		{
			g_WaveNumber--;
		}
	}
	if (GetTrg(VK_F5)) {
		// �U���𑝂₷�B
		g_Amplitude++;
	}
	else if (GetTrg(VK_F6)) 
	{
		// �U�������炷�B
		g_Amplitude--;
		if (g_Amplitude < 0)
		{
			g_Amplitude = 0;
		}
	}
	if (GetTrg(VK_F7)) 
	{
		// �ʑ��𑝂₷�B180���܂�
		g_Phase += D3DX_PI / 6;
		if (g_Phase > D3DX_PI) 
		{
			g_Phase = D3DX_PI;
		}
	}
	else if (GetTrg(VK_F8)) 
	{
		// �ʑ������炷�B-180���܂�
		g_Phase -= D3DX_PI / 6;
		if (g_Phase < -D3DX_PI) 
		{
			g_Phase = -D3DX_PI;
		}
	}
	if (GetTrg('Z')) 
	{
		// �p���x�𑝂₷�B
		g_Omega += D3DX_PI / 6;
	}
	else if (GetTrg('X')) 
	{
		// �p���x�����炷�B
		g_Omega -= D3DX_PI / 6;
		if (g_Omega < 0) 
		{
			g_Omega = 0;
		}
	}
	if ((keyBuffer['A'] & 0x80) != 0) 
	{
		// ����
		g_Sphere.m_Pos.x -= 0.1f;
	}
	else if ((keyBuffer['D'] & 0x80) != 0) 
	{
		// �E��
		g_Sphere.m_Pos.x += 0.1f;
	}
	if ((keyBuffer['W'] & 0x80) != 0) 
	{
		// ����
		g_Sphere.m_Pos.z += 0.1f;
	}
	else if ((keyBuffer['S'] & 0x80) != 0) 
	{
		// ��O��
		g_Sphere.m_Pos.z -= 0.1f;
	}

	TextPrintf(0, 0, "�^�C�v=%s", g_Type == 0 ? "���g(F1)" : "�i�s�g(F2)");
	TextPrintf(0, 20, "�g��=%.1f (F3:+ F4:-)", g_WaveNumber);
	TextPrintf(0, 40, "�U��=%.1f (F5:+ F6:-)", g_Amplitude);
	TextPrintf(0, 60, "�ʑ�=%.1f (F7:+ F8:-)", g_Phase);
	TextPrintf(0, 80, "�p���x=%.1f (Z:+ X:-)", g_Omega);
	TextPrintf(0, 100, "����=%.3f �{�[�� X=%.1f Y=%.1f Z=%.1f",
		g_TotalElapsedTime, g_Sphere.m_Pos.x, g_Sphere.m_Pos.y, g_Sphere.m_Pos.z);
	TextPrintf(0, 120, "�I�u�W�F�N�g�̈ړ�:ADWS");
	TextPrintf(0, 140, "�J������]:����");
	TextPrintf(0, 160, "�J�����Y�[��:����");


	// �ό`����B
	Transform(&g_Ground, elapsedTime);

	// �J�������X�V����B
	UpdateCamera(keyBuffer);

	// �O�̃L�[��Ԃ��o���Ă����B
	memcpy(oldKeyBuffer, keyBuffer, 256);
}

//-------------------------------------------------------------
// �J�������X�V����B
// ����
//		keyBuffer		�L�[�{�[�h�o�b�t�@
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
static void UpdateCamera(unsigned char* keyBuffer)
{
	float rad = 0;
	// ���L�[
	if ((keyBuffer[VK_LEFT] & 0x80) > 0)
	{
		// �J��������]����B
		rad = D3DXToRadian(-1);
	}
	else if ((keyBuffer[VK_RIGHT] & 0x80) > 0)
	{
		// �J��������]����B
		rad = D3DXToRadian(1);
	}	
	//else if ((keyBuffer[VK_UP] & 0x80) > 0)
	//{
	//	// �Y�[���C������B
	//	if (g_Camera.m_Fov > 0) {
	//		g_Camera.m_Fov -= D3DXToRadian(1);
	//	}
	//}
	//else if ((keyBuffer[VK_DOWN] & 0x80) > 0) 
	//{
	//	// �Y�[���A�E�g����B
	//	if (g_Camera.m_Fov < D3DX_PI) {
	//		g_Camera.m_Fov += D3DXToRadian(1);
	//	}
	//}	
	else if ((keyBuffer[VK_UP] & 0x80) > 0) 
	{
		// �Y�[���C������B
		g_Camera.m_Eye = (g_Camera.m_Eye - g_Camera.m_Target) * 0.9f;
	}
	else if ((keyBuffer[VK_DOWN] & 0x80) > 0) 
	{
		// �Y�[���A�E�g����B
		g_Camera.m_Eye = (g_Camera.m_Eye - g_Camera.m_Target) * 1.1f;
	}	
	// �J�����̌����x�N�g���̋t���v�Z����B
	D3DXVECTOR3 dir = g_Camera.m_Eye - g_Camera.m_Target;
	// ��]�s����쐬����B
	D3DXMATRIX matRotY;
	D3DXMatrixRotationY(&matRotY, rad);
	// �J�����̌����x�N�g������]�s��ŉ�]����B
	D3DXVec3TransformCoord(&dir, &dir, &matRotY);
	// �J�����̎��_��ݒ肷��B
	g_Camera.m_Eye = g_Camera.m_Target + dir;
}

//-------------------------------------------------------------
// �t���[�����Ƃ̕`��
// ����
//		elapsedTime			�o�ߎ���(�b)
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
void Draw(float elapsedTime)
{
	// �J������ݒ肷��B
	SetupCamera();

	// ��ʂ��N���A����B
	g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0, 255), 1.0f, 0);
	// �`����J�n����B
	if(SUCCEEDED(g_pD3DDevice->BeginScene())) 
	{
		// Z�o�b�t�@��L���ɂ���B
		g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
		// �����v���ɃJ�����O����B
		g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		// ���C�g���I�t�ɂ���B
		g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

		// �n�ʂ������_�����O����B
		D3DXMATRIX matWorld;		// ���[���h�ϊ��s��
		// ���[���h���W�ϊ�
		D3DXMatrixIdentity(&matWorld);
		g_pD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);
		// �n�ʂ�`�悷��B
		DrawUserMesh(&g_Ground);

		// ���C�g���I���ɂ���B
		g_pD3DDevice->SetLight(0, &g_Light);
		g_pD3DDevice->LightEnable(0, TRUE);	
		g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
		// �X�y�L������L���ɂ���B
		g_pD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
		// �A���r�G���g���C�g��ݒ肷��B
		g_pD3DDevice->SetRenderState(D3DRS_AMBIENT, 0x55555555);

		// �e�N�X�`���X�e�[�W�̐ݒ� 
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		// �A���t�@�u�����f�B���O��ݒ肷��B
		g_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		// ����������B
		g_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		g_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// ����`�悷��B
		D3DXMATRIX matTrans;
		// Y���W��Height-Field����v�Z����B
		g_Sphere.m_Pos.y = HeightField(g_Sphere.m_Pos.x, g_Sphere.m_Pos.z) + g_Sphere.m_Sphere.m_Radius;
		D3DXMatrixTranslation(&matTrans, g_Sphere.m_Pos.x, g_Sphere.m_Pos.y, g_Sphere.m_Pos.z);
		g_pD3DDevice->SetTransform(D3DTS_WORLD, &matTrans);
		DrawSphereMesh(&g_Sphere.m_Sphere);

		// �f�o�b�O����������ۂɕ`�悷��B
		DrawText();

		// �`����I������B
		g_pD3DDevice->EndScene();
	}
	// ����ʂɔ��f����B
	g_pD3DDevice->Present(NULL, NULL, NULL, NULL);
}

//-------------------------------------------------------------
// �J�����̐ݒ�
// ����
//		�Ȃ�
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
static void SetupCamera()
{
	D3DXMATRIX matView;
	D3DXMATRIX matProj;

	// �r���[���W�ϊ�
	D3DXMatrixLookAtLH(&matView,
		&g_Camera.m_Eye,
		&g_Camera.m_Target,
		&g_Camera.m_Up);
	g_pD3DDevice->SetTransform(D3DTS_VIEW, &matView);

	// �ˉe�ϊ�
	D3DXMatrixPerspectiveFovLH(&matProj,
		g_Camera.m_Fov,
		(float)CLIENT_WIDTH / (float)CLIENT_HEIGHT, 1.0f, 300.0f);
	g_pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

//-------------------------------------------------------------
// ��n������B
// ����
//		�Ȃ�
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
void Finalize()
{
	FinalizeSphereMesh(&g_Sphere.m_Sphere);
	FinalizeUserMesh(&g_Ground);
}


//-------------------------------------------------------------
// �����`�̃��b�V�����쐬����B
// ����:
//		mesh		���b�V��
//		texture		�e�N�X�`��
//		width		��
//		height		����
//		m_NumGrid		�O���b�h��
//		color		�F
// �߂�l:
//		0�E�E�E�߂�l, -1�E�E�E�G���[
//-------------------------------------------------------------
static int CreateUserMesh(USERMESH *mesh, LPDIRECT3DTEXTURE9 texture,
				 float width, float height, unsigned short numGrid, unsigned long color)
{
	// �O���b�h�̐����璸�_�����v�Z����B
	int numVertices = (numGrid + 1) * (numGrid + 1);
	// �O���b�h�̐�����C���f�b�N�X�����v�Z����B
	int numIndices = (numGrid * numGrid) * 6;
	// �����Ƃ̌J��Ԃ������v�Z����B
	int tile = (unsigned short)width / numGrid;

    // ���_�o�b�t�@���쐬����B
	LPDIRECT3DVERTEXBUFFER9 vb;
	HRESULT hr = g_pD3DDevice->CreateVertexBuffer(numVertices * sizeof(COLOR_VERTEX),	// �o�b�t�@�T�C�Y
                                    D3DUSAGE_WRITEONLY,	// �������݉\
									FVF_COLORVERTEX,	// ���_�t�H�[�}�b�g
                                    D3DPOOL_MANAGED,	// DirectX�ɔC����
									&vb,				// ���_�o�b�t�@
									NULL);
	if (FAILED(hr)) 
	{
        return -1;
	}

    // ���_�o�b�t�@�𖄂߂�B
    COLOR_VERTEX *vertices;
	// ���_�o�b�t�@�����b�N����B
	if (FAILED(vb->Lock(0, 0, (void **)&vertices, NULL))) 
	{
        return -1;
	}

    // ���_���Z�b�g����B
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
	// ���_�o�b�t�@���A�����b�N����B
	vb->Unlock();

    // �C���f�b�N�X�o�b�t�@���쐬����B
    LPDIRECT3DINDEXBUFFER9 ib;		// �C���f�b�N�X�o�b�t�@
    hr = g_pD3DDevice->CreateIndexBuffer(numIndices * sizeof(unsigned short),	// �o�b�t�@�T�C�Y
                                          D3DUSAGE_WRITEONLY,	// �������݉\
                                          D3DFMT_INDEX16,		// 16�r�b�g�̃C���f�b�N�X
										  D3DPOOL_MANAGED,		// DirectX�ɔC����
                                          &ib,					// �C���f�b�N�X�o�b�t�@
										  NULL);
	if (FAILED(hr))
	{
        return -1;
	}

    // �C���f�b�N�X�o�b�t�@�𖄂߂�B
    unsigned short *indices;
	// �C���f�b�N�X�o�b�t�@�����b�N����B
	ib->Lock(0, 0, (void **)&indices, 0);
	if (FAILED(hr)) 
	{
        return -1;
	}

    // �C���f�b�N�X���Z�b�g����B
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
	// �C���f�b�N�X�o�b�t�@���A�����b�N����B
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
// ���_�����[�t�B���O����B
// ����:
//		mesh		���[�U���b�V��
//		elapsedTime	�o�ߎ���
// �߂�l:
//		����������0�C�G���[�Ȃ�-1
//-------------------------------------------------------------
static int Transform(USERMESH *mesh, float elapsedTime)
{
	g_TotalElapsedTime += elapsedTime * 0.5f;
    COLOR_VERTEX *vertices;
	// ���_�����b�N����B
	if (FAILED(mesh->m_VertexBuffer->Lock(0, 0, (void **)&vertices, NULL)))
	{
        return -1;
	}

	// ���_���Z�b�g����B
    for (int z = 0; z <= mesh->m_NumGrid; z++) 
	{
        for (int x = 0; x <= mesh->m_NumGrid; x++)
		{
			// ���_��Y���W��Hight-Field�@�Ōv�Z����B
			vertices->v.y = HeightField(vertices->v.x, vertices->v.z);
			vertices++;
        }
    }
	// ���_���A�����b�N����B
	mesh->m_VertexBuffer->Unlock();

	return 0;
}

//-------------------------------------------------------------
// �e�N�X�`�������[�h����B
// ����
//		texture		�e�N�X�`���C���^�[�t�F�C�X
//		fileName	�e�N�X�`���t�@�C����
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
static void LoadTexture(LPDIRECT3DTEXTURE9* texture, char* fileName)
{
	if (D3DXCreateTextureFromFileEx 
		(
			g_pD3DDevice,			// Direct3D�f�o�C�X
			fileName,				// �e�N�X�`���t�@�C����
			0,						// ���i�s�N�Z���j, 0�̓t�@�C������擾
			0,						// �����i�s�N�Z���j, 0�̓t�@�C������擾
			0,						// �~�b�v���x���̐��B0�͊��S�ȃ~�b�v�}�b�v
			0,						// �g�����B
			D3DFMT_UNKNOWN,			// �s�N�Z���t�H�[�}�b�g�BD3DFMT_UNKNOWN�̓t�@�C������擾
			D3DPOOL_MANAGED,		// �쐬��B
			D3DX_DEFAULT,			// �t�B���^�����O�̕��@
			D3DX_DEFAULT,			// �~�b�v�t�B���^�����O�̕��@
			0xFF000000,				// �J���[�E�L�[�i�����F�j
			NULL,					// �摜�t�@�C�����̋L�q�B
			NULL,					// �p���b�g
			texture					// �e�N�X�`���C���^�[�t�F�C�X
		) != 0) 
	{
		MessageBox(NULL, "�e�N�X�`����������܂���B", "�G���[", MB_OK);
	}
}

//-----------------------------------------------------------------------------
// Height Field�@�ɂ��g�𐶐�����B
// ����:
//		x, z		X,Z���W
// �߂�l:
//		Y���W
//-----------------------------------------------------------------------------
float HeightField(float x, float z)
{
	float y = 0;
	x /= FLOOR_WIDTH;
	z /= FLOOR_HEIGHT;
	if (g_Type == 0) 
	{
		// ���g
		y =  g_Amplitude * sinf(g_Omega * g_TotalElapsedTime - g_WaveNumber * x - g_Phase)
			+ g_Amplitude * sinf(g_Omega * g_TotalElapsedTime + g_WaveNumber * x + g_Phase)
			+ g_Amplitude * sinf(g_Omega * g_TotalElapsedTime - g_WaveNumber * z - g_Phase)
			+ g_Amplitude * sinf(g_Omega * g_TotalElapsedTime + g_WaveNumber * z + g_Phase);
	}
	else 
	{
		// �i�s�g
		y = g_Amplitude * sinf(g_Omega * g_TotalElapsedTime - g_WaveNumber * x - g_Phase);
	}
	return y;
}


//-----------------------------------------------------------------------------
// ���[�U���b�V����`�悷��B
// ����:
//		mesh		���b�V��
// �߂�l:
//		�Ȃ�
//-----------------------------------------------------------------------------
static void DrawUserMesh(USERMESH *mesh)
{
	// �����_�����O����B
	g_pD3DDevice->SetTexture(0, mesh->m_pTexture);
    g_pD3DDevice->SetFVF(FVF_COLORVERTEX);
    g_pD3DDevice->SetStreamSource(0, mesh->m_VertexBuffer, 0, sizeof(COLOR_VERTEX));
    g_pD3DDevice->SetIndices(mesh->m_IndexBuffer);
    g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
							0, 0, mesh->m_NumVertices, 0, mesh->m_NumIndices / 3);
	g_pD3DDevice->SetTexture(0, NULL);
}

//-----------------------------------------------------------------------------
// ���[�U���b�V�����������B
// ����:
//		mesh		���b�V��
// �߂�l:
//		�Ȃ�
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
// ���O�ŋ��̃��b�V�����쐬����B
// X,Y�����������@Z�͐�������
// ����:
//		mesh				�����b�V��
//		radius				���a
//		numVertical			����������
//		numHorizontal		����������
// �߂�l:
//		0�E�E�E�߂�l, -1�E�E�E�G���[
//-----------------------------------------------------------------------------
static int CreateSphereMesh(SPHEREMESH *mesh, float radius, int numVertical, int numHorizontal)
{
	float theta;			// ��
	float rcosphi;			// r * cos��
	float x,y,z;
	// ���������璸�_�����v�Z����B
	int numVertices = (numVertical - 1) * numHorizontal + 2;
	// ����������ʐ����v�Z����B
	int numFaces = (numVertical - 2) * (numHorizontal * 2) + numHorizontal * 2;
	// �ʐ�����C���f�b�N�X�����v�Z����B
	int numIndices = numFaces * 3;

    // ���_�o�b�t�@���쐬����B
	LPDIRECT3DVERTEXBUFFER9 vb;
	HRESULT hr = g_pD3DDevice->CreateVertexBuffer(numVertices * sizeof(LIGHT_VERTEX),	// �o�b�t�@�T�C�Y
                                    D3DUSAGE_WRITEONLY,	// �������݉\
									FVF_COLORVERTEX,	// ���_�t�H�[�}�b�g
                                    D3DPOOL_MANAGED,	// DirectX�ɔC����
									&vb,				// ���_�o�b�t�@
									NULL);
	if (FAILED(hr))
	{
        return -1;
	}

    // ���_�o�b�t�@�𖄂߂�B
    LIGHT_VERTEX *vertices;
	// ���_�o�b�t�@�����b�N����B
	if (FAILED(vb->Lock(0, 0, (void **)&vertices, NULL))) 
	{
        return -1;
	}

	// ��̒[�̒��_�i�ŏ��̒��_�j���쐬����
	vertices->v = D3DXVECTOR3(0.0f, radius, 0.0f);
	vertices->n = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	vertices++;

	// ���Ԃ̃����O�̒��_���쐬����
	float dphi = D3DX_PI / numVertical;		// ���ꂼ��̃����O�̐��������̊p�x
	float dtheta = 2 * D3DX_PI / numHorizontal;	// ���ꂼ��̋��̐��������̊p�x
	float phi = D3DX_PI / 2 - dphi;			// �ŏ��̐��������̊p�x

	// 2�ڂ̒��_����Ō��1�O�܂Őݒ肷��
	int n = 1;
	// ���������̕����̃��[�v
	for (int i=0; i < numVertical - 1; i++) 
	{
		// Y���W���v�Z����
		y = sinf(phi);
		// cos�ӂ����[�v���Ŗ���v�Z���Ȃ��ł����悤�ɐ�Ɍv�Z���Ă���
		rcosphi = cosf(phi);

		theta = 0.0f;
		// ���������̕������[�v			// �ŏ��̐��������̊p�x
		for	(int j = 0; j < numHorizontal; j++) 
		{
			// X���W���v�Z����
			x = cosf(theta) * rcosphi;
			// Z���W���v�Z����
			z = sinf(theta) * rcosphi;
			// ���W��ݒ肷��
			vertices->v = D3DXVECTOR3(radius * x, radius * y, radius * z);
			// �@����ݒ肷��
			vertices->n = D3DXVECTOR3(x, y, z);
			// ���K������B
			D3DXVec3Normalize(&vertices->n, &vertices->n);
			vertices++;

			theta += dtheta;		// ���̐����ʒu��
			++n;
		}
		phi -=dphi;					// ���̐����ʒu��
	}

	// ���̒[�̒��_�i�Ō�̒��_�j���쐬����
	vertices->v = D3DXVECTOR3(0.0f, -radius, 0.0f);
	vertices->n = D3DXVECTOR3(0.0f, -1.0f, 0.0f);


	// ���_�o�b�t�@���A�����b�N����B
	vb->Unlock();


    // �C���f�b�N�X�o�b�t�@���쐬����B
    LPDIRECT3DINDEXBUFFER9 ib;		// �C���f�b�N�X�o�b�t�@
    hr = g_pD3DDevice->CreateIndexBuffer(numIndices * sizeof(unsigned short),	// �o�b�t�@�T�C�Y
                                          D3DUSAGE_WRITEONLY,	// �������݉\
                                          D3DFMT_INDEX16,		// 16�r�b�g�̃C���f�b�N�X
										  D3DPOOL_MANAGED,		// DirectX�ɔC����
                                          &ib,					// �C���f�b�N�X�o�b�t�@
										  NULL);
	if (FAILED(hr)) 
	{
        return -1;
	}

    // �C���f�b�N�X�o�b�t�@�𖄂߂�B
    unsigned short *indices;
	// �C���f�b�N�X�o�b�t�@�����b�N����B
	ib->Lock(0, 0, (void **)&indices, 0);
	if (FAILED(hr))
	{
        return -1;
	}

	// ��Ɖ��̎O�p�`�𐶐�����
	for (int i = 0; i < numHorizontal; i++)
	{
		// ��
		*indices++ = (unsigned short)0;
		*indices++ = (unsigned short)(1 + ((1 + i) % numHorizontal));
		*indices++ = (unsigned short)(i + 1);
		// ��
		*indices++ = (unsigned short)(numVertices - 1);
		*indices++ = (unsigned short)(numVertices - 1 - ((1 + i) % numHorizontal));
		*indices++ = (unsigned short)(numVertices - 2 - i);
	}

	// ���Ԃ̃����O�̎O�p�`�𐶐�����B���̏ꍇ�͎l�p�ɂȂ�̂łQ�����B
	int m = 1;				// ��ԏ�̒��_���玟�̓_���g��
	for (int i = 0; i < numVertical - 2; i++) 
	{
		for (int j = 0; j < numHorizontal; j++) 
		{
			// 1�ڂ̎O�p�`
			*indices++ = (unsigned short)(m + j);
			*indices++ = (unsigned short)(m + ((j + 1) % numHorizontal));
			*indices++ = (unsigned short)(m + numHorizontal + ((j + 1) % numHorizontal));
			// 2�ڂ̎O�p�`
			*indices++ = (unsigned short)(m + j);
			*indices++ = (unsigned short)(m + numHorizontal + ((j + 1) % numHorizontal));
			*indices++ = (unsigned short)(m + numHorizontal + j);
		}
		m += numHorizontal;
	}

	// �C���f�b�N�X�o�b�t�@���A�����b�N����B
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
// �����b�V����`�悷��B
// ����:
//		mesh		�����b�V��
// �߂�l:
//		�Ȃ�
//-----------------------------------------------------------------------------
static void DrawSphereMesh(SPHEREMESH *mesh)
{
	// �����_�����O����B
	// �e�N�X�`���Ȃ�
	g_pD3DDevice->SetTexture(0, NULL);
	g_pD3DDevice->SetMaterial(&mesh->m_Material);
    g_pD3DDevice->SetFVF(FVF_LIGHTVERTEX);
    g_pD3DDevice->SetStreamSource(0, mesh->m_VertexBuffer, 0, sizeof(LIGHT_VERTEX));
    g_pD3DDevice->SetIndices(mesh->m_IndexBuffer);
    g_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
							0, 0, mesh->m_NumVertices, 0, mesh->m_NumIndices / 3);
}


//-----------------------------------------------------------------------------
// �����b�V�����������B
// ����:
//		mesh		�����b�V��
// �߂�l:
//		�Ȃ�
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
// ���C�g���쐬����B
// ����
//		�Ȃ�
// �߂�l
//		�Ȃ�
//-------------------------------------------------------------
static void CreateLight()
{
	// ���C�g��ݒ肷��B
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