/**
 * @file	kdispwnd.cpp
 * @brief	�L�[�{�[�h �N���X�̓���̒�`���s���܂�
 */

#include "compiler.h"
#include "resource.h"
#include "kdispwnd.h"
#include "np2.h"
#include "ini.h"
#include "menu.h"
#include "sysmng.h"
#include "dialog/np2class.h"
#include "generic/keydisp.h"

extern WINLOCEX np2_winlocexallwin(HWND base);

#if defined(SUPPORT_KEYDISP)

//! �B��̃C���X�^���X�ł�
CKeyDisplayWnd CKeyDisplayWnd::sm_instance;

/**
 * ���[�h
 */
enum
{
	KDISPCFG_FM		= 0x00,
	KDISPCFG_MIDI	= 0x80
};

/**
 * @brief �R���t�B�O
 */
struct KeyDisplayConfig
{
	int		posx;		//!< X
	int		posy;		//!< Y
	UINT8	mode;		//!< ���[�h
	UINT8	type;		//!< �E�B���h�E �^�C�v
};

//! �R���t�B�O
static KeyDisplayConfig s_kdispcfg;

//! �^�C�g��
static const TCHAR s_kdispapp[] = TEXT("Key Display");

/**
 * �ݒ�
 */
static const PFTBL s_kdispini[] =
{
	PFVAL("WindposX", PFTYPE_SINT32,	&s_kdispcfg.posx),
	PFVAL("WindposY", PFTYPE_SINT32,	&s_kdispcfg.posy),
	PFVAL("keydmode", PFTYPE_UINT8,		&s_kdispcfg.mode),
	PFVAL("windtype", PFTYPE_BOOL,		&s_kdispcfg.type)
};

//! �p���b�g
static const UINT32 s_kdisppal[KEYDISP_PALS] = {0x00000000, 0xffffffff, 0xf9ff0000};

/**
 * ������
 */
void CKeyDisplayWnd::Initialize()
{
	keydisp_initialize();
}

/**
 * ���
 */
void CKeyDisplayWnd::Deinitialize()
{
}

/**
 * �R���X�g���N�^
 */
CKeyDisplayWnd::CKeyDisplayWnd()
{
}

/**
 * �f�X�g���N�^
 */
CKeyDisplayWnd::~CKeyDisplayWnd()
{
}

/**
 * 8bpp�F��Ԃ�
 * @param[in] self �C���X�^���X
 * @param[in] num �p���b�g�ԍ�
 * @return �F
 */
static UINT8 kdgetpal8(CMNPALFN* self, UINT num)
{
	if (num < KEYDISP_PALS)
	{
		return s_kdisppal[num] >> 24;
	}
	return 0;
}

/**
 * 16bpp�F��Ԃ�
 * @param[in] self �C���X�^���X
 * @param[in] pal32 �p���b�g
 * @return �F
 */
static UINT16 kdcnvpal16(CMNPALFN* self, RGB32 pal32)
{
	return (reinterpret_cast<DD2Surface*>(self->userdata))->GetPalette16(pal32);
}

/**
 * 32bpp�F��Ԃ�
 * @param[in] self �C���X�^���X
 * @param[in] num �p���b�g�ԍ�
 * @return �F
 */
static UINT32 kdgetpal32(CMNPALFN* self, UINT num)
{
	if (num < KEYDISP_PALS)
	{
		return s_kdisppal[num] & 0xffffff;
	}
	return 0;
}

/**
 * �쐬
 */
void CKeyDisplayWnd::Create()
{
	if (m_hWnd != NULL)
	{
		return;
	}

	HINSTANCE hInstance = FindResourceHandle(MAKEINTRESOURCE(IDR_KEYDISP), RT_MENU);
	HMENU hMenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_KEYDISP));
	if (!CSubWndBase::Create(IDS_CAPTION_KEYDISP, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, s_kdispcfg.posx, s_kdispcfg.posy, KEYDISP_WIDTH, KEYDISP_HEIGHT, NULL, hMenu))
	{
		np2oscfg.keydisp = 0;
		sysmng_update(SYS_UPDATEOSCFG);
		return;
	}

	UINT8 mode;
	switch (s_kdispcfg.mode)
	{
		case KDISPCFG_FM:
		default:
			mode = KEYDISP_MODEFM;
			break;

		case KDISPCFG_MIDI:
			mode = KEYDISP_MODEMIDI;
			break;
	}
	SetDispMode(mode);
	ShowWindow(SW_SHOWNOACTIVATE);
	UpdateWindow();

	if (!m_dd2.Create(m_hWnd, KEYDISP_WIDTH, KEYDISP_HEIGHT))
	{
		DestroyWindow();
		return;
	}

	CMNPALFN palfn;
	palfn.get8 = kdgetpal8;
	palfn.get32 = kdgetpal32;
	palfn.cnv16 = kdcnvpal16;
	palfn.userdata = reinterpret_cast<INTPTR>(&m_dd2);
	keydisp_setpal(&palfn);
	kdispwin_draw(0);
	::SetForegroundWindow(g_hWndMain);
}

/**
 * �`�悷��
 * @param[in] cnt �i�񂾃t���[��
 */
void CKeyDisplayWnd::Draw(UINT8 cnt)
{
	if (m_hWnd)
	{
		if (!cnt)
		{
			cnt = 1;
		}
		UINT8 flag = keydisp_process(cnt);
		if (flag & KEYDISP_FLAGSIZING)
		{
			OnResize();
		}
		OnDraw(FALSE);
	}
}

/**
 * CWndProc �I�u�W�F�N�g�� Windows �v���V�[�W�� (WindowProc) ���p�ӂ���Ă��܂�
 * @param[in] nMsg ��������� Windows ���b�Z�[�W���w�肵�܂�
 * @param[in] wParam ���b�Z�[�W�̏����Ŏg���t������񋟂��܂��B���̃p�����[�^�̒l�̓��b�Z�[�W�Ɉˑ����܂�
 * @param[in] lParam ���b�Z�[�W�̏����Ŏg���t������񋟂��܂��B���̃p�����[�^�̒l�̓��b�Z�[�W�Ɉˑ����܂�
 * @return ���b�Z�[�W�Ɉˑ�����l��Ԃ��܂�
 */
LRESULT CKeyDisplayWnd::WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
		case WM_CREATE:
			np2class_wmcreate(m_hWnd);
			np2class_windowtype(m_hWnd, (s_kdispcfg.type & 1) << 1);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDM_KDISPFM:
					s_kdispcfg.mode = KDISPCFG_FM;
					sysmng_update(SYS_UPDATEOSCFG);
					SetDispMode(KEYDISP_MODEFM);
					break;

				case IDM_KDISPMIDI:
					s_kdispcfg.mode = KDISPCFG_MIDI;
					sysmng_update(SYS_UPDATEOSCFG);
					SetDispMode(KEYDISP_MODEMIDI);
					break;

				case IDM_CLOSE:
					return SendMessage(WM_CLOSE, 0, 0);
			}
			break;

		case WM_PAINT:
			OnPaint();
			break;

		case WM_LBUTTONDOWN:
			if (s_kdispcfg.type & 1)
			{
				return SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, 0L);
			}
			break;

		case WM_RBUTTONDOWN:
			{
				POINT pt;
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);
				OnRButtonDown(static_cast<UINT>(wParam), pt);
			}
			break;

		case WM_LBUTTONDBLCLK:
			s_kdispcfg.type ^= 1;
			SetWndType((s_kdispcfg.type & 1) << 1);
			sysmng_update(SYS_UPDATEOSCFG);
			break;

		case WM_MOVE:
			if (!(GetWindowLong(m_hWnd, GWL_STYLE) & (WS_MAXIMIZE | WS_MINIMIZE)))
			{
				RECT rc;
				GetWindowRect(&rc);
				s_kdispcfg.posx = rc.left;
				s_kdispcfg.posy = rc.top;
				sysmng_update(SYS_UPDATEOSCFG);
			}
			break;

		case WM_CLOSE:
			np2oscfg.keydisp = 0;
			sysmng_update(SYS_UPDATEOSCFG);
			DestroyWindow();
			break;

		case WM_DESTROY:
			OnDestroy();
			break;

		default:
			return CSubWndBase::WindowProc(nMsg, wParam, lParam);
	}
	return 0L;
}

/**
 * �E�B���h�E�j���̎��ɌĂ΂��
 */
void CKeyDisplayWnd::OnDestroy()
{
	::np2class_wmdestroy(m_hWnd);
	m_dd2.Release();
	SetDispMode(KEYDISP_MODENONE);
}

/**
 * �t���[�����[�N�́A���[�U�[���}�E�X�̉E�{�^���������ƁA���̃����o�[�֐����Ăяo���܂�
 * @param[in] nFlags ���z�L�[��������Ă��邩�ǂ����������܂�
 * @param[in] point �J�[�\���� x ���W�� y ���W���w�肵�܂�
 */
void CKeyDisplayWnd::OnRButtonDown(UINT nFlags, POINT point)
{
	HMENU hMenu = CreatePopupMenu();
	menu_addmenu(hMenu, 0, np2class_gethmenu(m_hWnd), FALSE);
	menu_addmenures(hMenu, -1, IDR_CLOSE, TRUE);

	ClientToScreen(&point);
	::TrackPopupMenu(hMenu, TPM_LEFTALIGN, point.x, point.y, 0, m_hWnd, NULL);
	::DestroyMenu(hMenu);
}

/**
 * �`��̎��ɌĂ΂��
 */
void CKeyDisplayWnd::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(&ps);
	OnDraw(TRUE);
	EndPaint(&ps);
}

/**
 * �`��
 * @param[in] redraw �ĕ`��
 */
void CKeyDisplayWnd::OnDraw(BOOL redraw)
{
	RECT rect;
	GetClientRect(&rect);

	RECT draw;
	draw.left = 0;
	draw.top = 0;
	draw.right = min(KEYDISP_WIDTH, rect.right - rect.left);
	draw.bottom = min(KEYDISP_HEIGHT, rect.bottom - rect.top);
	if ((draw.right <= 0) || (draw.bottom <= 0))
	{
		return;
	}
	CMNVRAM* vram = m_dd2.Lock();
	if (vram)
	{
		keydisp_paint(vram, redraw);
		m_dd2.Unlock();
		m_dd2.Blt(NULL, &draw);
	}
}

/**
 * ���T�C�Y
 */
void CKeyDisplayWnd::OnResize()
{
	WINLOCEX wlex = np2_winlocexallwin(g_hWndMain);
	winlocex_setholdwnd(wlex, m_hWnd);

	int width;
	int height;
	keydisp_getsize(&width, &height);
	winloc_setclientsize(m_hWnd, width, height);
	winlocex_move(wlex);
	winlocex_destroy(wlex);
}

/**
 * ���[�h �`�F���W
 * @param[in] mode ���[�h
 */
void CKeyDisplayWnd::SetDispMode(UINT8 mode)
{
	keydisp_setmode(mode);

	HMENU hMenu = np2class_gethmenu(m_hWnd);
	CheckMenuItem(hMenu, IDM_KDISPFM, ((mode == KEYDISP_MODEFM) ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(hMenu, IDM_KDISPMIDI, ((mode == KEYDISP_MODEMIDI) ? MF_CHECKED : MF_UNCHECKED));
}

void kdispwin_create(){
	keydisp_setresizeflag();
	CKeyDisplayWnd::GetInstance()->Create();
}

/**
 * �ݒ�ǂݍ���
 */
void kdispwin_readini()
{
	ZeroMemory(&s_kdispcfg, sizeof(s_kdispcfg));
	s_kdispcfg.posx = CW_USEDEFAULT;
	s_kdispcfg.posy = CW_USEDEFAULT;

#ifdef _WIN64
	// x64では安全な設定でデフォルト値を使用
	return;
#else
	TCHAR szPath[MAX_PATH];
	initgetfile(szPath, _countof(szPath));
	ini_read(szPath, s_kdispapp, s_kdispini, _countof(s_kdispini));
#endif
}

/**
 * �ݒ菑������
 */
void kdispwin_writeini()
{
	if(!np2oscfg.readonly){
		TCHAR szPath[MAX_PATH];

		initgetfile(szPath, _countof(szPath));
		ini_write(szPath, s_kdispapp, s_kdispini, _countof(s_kdispini));
	}
}
#endif
