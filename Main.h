#ifndef _MAIN_H_
#define _MAIN_H_

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = NULL; } }
#define SAFE_DELETE(p) { if(p) { delete (p); (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p) = NULL; } }

#define FRAME_RATE		60
#define CLIENT_WIDTH	640	// �N���C�A���g�̈�̃f�t�H���g�̕�, �t���X�N���[���̏ꍇ�͐��������𑜓x
#define	CLIENT_HEIGHT	480	// �N���C�A���g�̈�̃f�t�H���g�̍���, �t���X�N���[���̏ꍇ�͐��������𑜓x

// �O���[�o���ϐ��錾
extern LPDIRECT3DDEVICE9 g_pD3DDevice;

// �f�o�b�O������\��
void SetTextColor(int r, int g, int b);			// �����̐F�̕ύX
void DrawText(void);							// �e�L�X�g�̕`��

void TextPrintf(int x, int y, char*format, ...);	// �ҏW����������̕`��
void DebugPrintf(char* format, ...);				// �R���\�[���ɕҏW�����������`��

#endif
