/**
 * @file	statemgr.cpp
 * @brief	State Manager Window implementation
 */

#include "compiler.h"
#include "resource.h"
#include "statemgr.h"
#include "np2.h"
#include "pccore.h"
#include "../dosio.h"
#include <commctrl.h>

// Static instance
CStateManagerWnd* CStateManagerWnd::s_pInstance = NULL;

/**
 * Constructor
 */
CStateManagerWnd::CStateManagerWnd()
	: m_hListView(NULL)
	, m_hImageList(NULL)
	, m_hStatusBar(NULL)
	, m_hToolBar(NULL)
	, m_nSelectedSlot(-1)
	, m_bInitialized(FALSE)
{
	ZeroMemory(&m_SlotMaster, sizeof(m_SlotMaster));
}

/**
 * Destructor
 */
CStateManagerWnd::~CStateManagerWnd()
{
	if (m_hImageList) {
		::ImageList_Destroy(m_hImageList);
		m_hImageList = NULL;
	}
}

/**
 * Get singleton instance
 */
CStateManagerWnd* CStateManagerWnd::GetInstance()
{
	return s_pInstance;
}

/**
 * Create singleton instance
 */
void CStateManagerWnd::CreateInstance()
{
	TRACEOUT(("StateManager: Creating singleton instance\n"));
	if (!s_pInstance) {
		s_pInstance = new CStateManagerWnd();
		TRACEOUT(("StateManager: Singleton instance created successfully\n"));
	} else {
		TRACEOUT(("StateManager: Singleton instance already exists\n"));
	}
}

/**
 * Destroy singleton instance
 */
void CStateManagerWnd::DestroyInstance()
{
	if (s_pInstance) {
		if (s_pInstance->m_hWnd) {
			s_pInstance->DestroyWindow();
		}
		delete s_pInstance;
		s_pInstance = NULL;
	}
}

/**
 * Create window
 */
BOOL CStateManagerWnd::Create()
{
	TRACEOUT(("StateManager: Create() called\n"));
	if (m_hWnd) {
		TRACEOUT(("StateManager: Window already exists\n"));
		return TRUE;
	}

	TRACEOUT(("StateManager: Creating main window\n"));

	// Calculate position to avoid overlapping with main window
	int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
	if (g_hWndMain && ::IsWindow(g_hWndMain)) {
		RECT rcMain, rcScreen;
		::GetWindowRect(g_hWndMain, &rcMain);

		// Get screen dimensions
		rcScreen.left = 0;
		rcScreen.top = 0;
		rcScreen.right = GetSystemMetrics(SM_CXSCREEN);
		rcScreen.bottom = GetSystemMetrics(SM_CYSCREEN);

		// Try to place to the right of main window
		x = rcMain.right + 10;
		y = rcMain.top;

		// If it would go off-screen, place to the left
		if (x + 800 > rcScreen.right) {
			x = rcMain.left - 800 - 10;
			// If still off-screen, place at screen edge
			if (x < 0) {
				x = rcScreen.right - 800 - 20;
			}
		}

		// Ensure the window stays within screen bounds
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		if (x + 800 > screenWidth) {
			x = rcMain.left - 810; // Place to the left if not enough space on right
		}
		if (x < 0) {
			x = 50; // Fallback position
		}
	}

	if (!CSubWndBase::Create(_T("State Manager"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		x, y, 800, 600,
		g_hWndMain, NULL)) {
		TRACEOUT(("StateManager: Failed to create main window\n"));
		return FALSE;
	}
	TRACEOUT(("StateManager: Main window created successfully\n"));

	TRACEOUT(("StateManager: Initializing controls\n"));
	if (!InitializeControls()) {
		TRACEOUT(("StateManager: Failed to initialize controls\n"));
		DestroyWindow();
		return FALSE;
	}
	TRACEOUT(("StateManager: Controls initialized successfully\n"));

	SetWndType(WNDTYPE_STATEMGR);
	m_bInitialized = TRUE;

	// Load initial data
	TRACEOUT(("StateManager: Loading initial data\n"));
	Refresh();
	TRACEOUT(("StateManager: Create() completed successfully\n"));

	return TRUE;
}

/**
 * Show window
 */
void CStateManagerWnd::Show(BOOL bShow)
{
	if (!m_hWnd && bShow) {
		Create();
	}

	if (m_hWnd) {
		ShowWindow(bShow ? SW_SHOW : SW_HIDE);
		if (bShow) {
			SetForegroundWindow(m_hWnd);
		}
	}
}

/**
 * Initialize controls
 */
BOOL CStateManagerWnd::InitializeControls()
{
	RECT rc;
	GetClientRect(&rc);

	// Create toolbar with improved style
	m_hToolBar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_LIST | CCS_ADJUSTABLE,
		0, 0, 0, 0, m_hWnd, (HMENU)IDC_STATEMGR_TOOLBAR, CWndProc::GetInstanceHandle(), NULL);

	if (!m_hToolBar) {
		return FALSE;
	}

	// Set extended toolbar styles for better appearance
	::SendMessage(m_hToolBar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);

	// Setup toolbar buttons with text
	TBBUTTON tbButtons[] = {
		{I_IMAGENONE, IDM_STATEMGR_SAVE,    TBSTATE_ENABLED, BTNS_BUTTON | BTNS_SHOWTEXT, {0}, 0, (INT_PTR)_T("Save")},
		{I_IMAGENONE, IDM_STATEMGR_LOAD,    TBSTATE_ENABLED, BTNS_BUTTON | BTNS_SHOWTEXT, {0}, 0, (INT_PTR)_T("Load")},
		{I_IMAGENONE, IDM_STATEMGR_DELETE,  TBSTATE_ENABLED, BTNS_BUTTON | BTNS_SHOWTEXT, {0}, 0, (INT_PTR)_T("Delete")},
		{0, 0,                              TBSTATE_ENABLED, BTNS_SEP,    {0}, 0, 0},
		{I_IMAGENONE, IDM_STATEMGR_REFRESH, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_SHOWTEXT, {0}, 0, (INT_PTR)_T("Refresh")},
	};

	::SendMessage(m_hToolBar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	::SendMessage(m_hToolBar, TB_ADDBUTTONS, _countof(tbButtons), (LPARAM)tbButtons);

	// Set button size for better text visibility
	::SendMessage(m_hToolBar, TB_SETBUTTONSIZE, 0, MAKELONG(60, 24));
	::SendMessage(m_hToolBar, TB_AUTOSIZE, 0, 0);

	// Create list view
	RECT rcToolbar;
	::GetWindowRect(m_hToolBar, &rcToolbar);
	int toolbarHeight = rcToolbar.bottom - rcToolbar.top;

	m_hListView = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
		WS_CHILD | WS_VISIBLE | LVS_ICON | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
		0, toolbarHeight, rc.right, rc.bottom - toolbarHeight - 20,
		m_hWnd, (HMENU)IDC_STATEMGR_LISTVIEW, CWndProc::GetInstanceHandle(), NULL);

	if (!m_hListView) {
		return FALSE;
	}

	// Create status bar
	m_hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0, 0, 0, 0, m_hWnd, (HMENU)IDC_STATEMGR_STATUSBAR, CWndProc::GetInstanceHandle(), NULL);

	if (!m_hStatusBar) {
		return FALSE;
	}

	SetupListView();

	return TRUE;
}

/**
 * Setup list view
 */
void CStateManagerWnd::SetupListView()
{
	// Create image list for thumbnails
	m_hImageList = ::ImageList_Create(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT,
		ILC_COLOR24 | ILC_MASK, 200, 50);

	if (m_hImageList) {
		ListView_SetImageList(m_hListView, m_hImageList, LVSIL_NORMAL);
	}

	// Set icon spacing for 5x40 grid
	ListView_SetIconSpacing(m_hListView, THUMBNAIL_WIDTH + 20, THUMBNAIL_HEIGHT + 40);

	// Set extended styles
	ListView_SetExtendedListViewStyle(m_hListView,
		0x00010000 /*LVS_EX_DOUBLEBUFFER*/ | LVS_EX_INFOTIP);
}

/**
 * Refresh data and UI
 */
void CStateManagerWnd::Refresh()
{
	if (!m_bInitialized) {
		return;
	}

	// Load slot master data
	if (statsave_get_slot_master(&m_SlotMaster) != SUCCESS) {
		// Initialize empty master if load fails
		ZeroMemory(&m_SlotMaster, sizeof(m_SlotMaster));
		m_SlotMaster.signature = 0x544F4C53; // 'SLOT'
		m_SlotMaster.version = 1;
		m_SlotMaster.slot_count = 200;
		m_SlotMaster.used_count = 0;
	}

	UpdateSlotList();
	UpdateStatusBar();
}

/**
 * Update slot list
 */
void CStateManagerWnd::UpdateSlotList()
{
	if (!m_hListView) {
		return;
	}

	// Clear existing items
	ListView_DeleteAllItems(m_hListView);

	if (m_hImageList) {
		::ImageList_RemoveAll(m_hImageList);
	}

	// Add all slots (including empty ones for grid layout)
	for (int slot = 0; slot < 200; slot++) {
		AddSlotToList(slot);
	}
}

/**
 * Add slot to list
 */
void CStateManagerWnd::AddSlotToList(int slot)
{
	LVITEM lvi;
	TCHAR title[64];
	HBITMAP hThumbnail;
	int imageIndex = -1;

	// Get thumbnail - try to load actual thumbnail from save file
	if (IsSlotUsed(slot)) {
		hThumbnail = statsave_load_thumbnail(slot);

		// If thumbnail loading failed, create a default placeholder
		if (!hThumbnail) {
			HDC hDC = GetDC(m_hWnd);
			HDC hMemDC = CreateCompatibleDC(hDC);
			hThumbnail = CreateCompatibleBitmap(hDC, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
			HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hThumbnail);

			// Fill with gray background
			HBRUSH hBrush = CreateSolidBrush(RGB(192, 192, 192));
			RECT rc = {0, 0, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT};
			FillRect(hMemDC, &rc, hBrush);
			DeleteObject(hBrush);

			// Draw "No Image" text instead of slot number
			SetBkMode(hMemDC, TRANSPARENT);
			SetTextColor(hMemDC, RGB(64, 64, 64));
			DrawText(hMemDC, _T("No Image"), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			SelectObject(hMemDC, hOldBmp);
			DeleteDC(hMemDC);
			ReleaseDC(m_hWnd, hDC);
		}

		if (hThumbnail) {
			imageIndex = ::ImageList_Add(m_hImageList, hThumbnail, NULL);
			DeleteObject(hThumbnail);
		}
	}

	// Prepare list item
	if (IsSlotUsed(slot)) {
		_stprintf_s(title, _countof(title), _T("Slot %03d\\nUsed"), slot);
	} else {
		_stprintf_s(title, _countof(title), _T("Slot %03d\\n[Empty]"), slot);
	}

	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = slot;
	lvi.iSubItem = 0;
	lvi.pszText = title;
	lvi.iImage = imageIndex;
	lvi.lParam = slot;

	ListView_InsertItem(m_hListView, &lvi);
}

/**
 * Update status bar
 */
void CStateManagerWnd::UpdateStatusBar()
{
	if (!m_hStatusBar) {
		return;
	}

	TCHAR status[256];
	_stprintf_s(status, _countof(status),
		_T("Total Slots: 200 | Used: %d | Selected: %s"),
		m_SlotMaster.used_count,
		m_nSelectedSlot >= 0 ? _T("Yes") : _T("None"));

	::SendMessage(m_hStatusBar, SB_SETTEXT, 0, (LPARAM)status);
}

/**
 * Check if slot is used
 */
BOOL CStateManagerWnd::IsSlotUsed(int slot)
{
	return statsave_check_slot_exists(slot) ? TRUE : FALSE;
}

/**
 * Window procedure
 */
LRESULT CStateManagerWnd::WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg) {
		case WM_SIZE:
			{
				RECT rc;
				GetClientRect(&rc);

				// Resize toolbar
				if (m_hToolBar) {
					::SendMessage(m_hToolBar, TB_AUTOSIZE, 0, 0);
				}

				// Resize status bar
				if (m_hStatusBar) {
					::SendMessage(m_hStatusBar, WM_SIZE, 0, 0);
				}

				// Resize list view
				if (m_hListView && m_hToolBar && m_hStatusBar) {
					RECT rcToolbar, rcStatus;
					::GetWindowRect(m_hToolBar, &rcToolbar);
					::GetWindowRect(m_hStatusBar, &rcStatus);

					int toolbarHeight = rcToolbar.bottom - rcToolbar.top;
					int statusHeight = rcStatus.bottom - rcStatus.top;

					SetWindowPos(m_hListView, NULL, 0, toolbarHeight,
						rc.right, rc.bottom - toolbarHeight - statusHeight,
						SWP_NOZORDER);
				}
			}
			break;

		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR)lParam;
				if (pnmh->hwndFrom == m_hListView) {
					switch (pnmh->code) {
						case LVN_ITEMCHANGED:
							{
								LPNMLISTVIEW pnmlv = (LPNMLISTVIEW)lParam;
								if (pnmlv->uNewState & LVIS_SELECTED) {
									m_nSelectedSlot = (int)pnmlv->lParam;
									UpdateStatusBar();
								}
							}
							break;

						case NM_DBLCLK:
							{
								LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)lParam;
								if (pnmia->iItem >= 0) {
									OnSlotDoubleClick((int)pnmia->lParam);
								}
							}
							break;

						case NM_RCLICK:
							{
								LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)lParam;
								if (pnmia->iItem >= 0) {
									OnContextMenu((int)pnmia->lParam, pnmia->ptAction);
								}
							}
							break;
					}
				}
			}
			break;

		case WM_COMMAND:
			{
				WORD cmd = LOWORD(wParam);
				switch (cmd) {
					case IDM_STATEMGR_SAVE:
						OnSlotSave();
						break;
					case IDM_STATEMGR_LOAD:
						OnSlotLoad();
						break;
					case IDM_STATEMGR_DELETE:
						OnSlotDelete();
						break;
					case IDM_STATEMGR_REFRESH:
						OnRefresh();
						break;
				}
			}
			break;

		default:
			return CSubWndBase::WindowProc(nMsg, wParam, lParam);
	}

	return 0;
}

/**
 * Slot operations
 */
void CStateManagerWnd::OnSlotSave()
{
	extern HWND g_hWndMain;

	if (m_nSelectedSlot < 0) {
		MessageBox(m_hWnd, _T("Please select a slot first."), _T("State Manager"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	int result = statsave_save_ext_with_hwnd(m_nSelectedSlot, NULL, g_hWndMain);
	if (result == STATFLAG_SUCCESS) {
		Refresh(); // Update display
		TCHAR debugMsg[256];
		_stprintf_s(debugMsg, _countof(debugMsg), _T("State saved successfully to slot %d\n\nFiles saved to: SaveStates\\"), m_nSelectedSlot);
		MessageBox(m_hWnd, debugMsg, _T("State Manager"), MB_OK | MB_ICONINFORMATION);
	} else if (result & STATFLAG_WARNING) {
		Refresh(); // Update display since save was successful with warnings
		TCHAR warningMsg[256];
		_stprintf_s(warningMsg, _countof(warningMsg),
			_T("State saved with warnings to slot %d.\nWarning code: %d\n\nFiles saved to: SaveStates\\\n\nThis is usually normal."),
			m_nSelectedSlot, result);
		MessageBox(m_hWnd, warningMsg, _T("State Manager - Save Warning"), MB_OK | MB_ICONWARNING);
	} else {
		TCHAR errorMsg[256];
		_stprintf_s(errorMsg, _countof(errorMsg),
			_T("Failed to save state to slot %d.\nError code: %d\n\nPossible causes:\n- Disk full\n- Permission denied\n- File system error"),
			m_nSelectedSlot, result);
		MessageBox(m_hWnd, errorMsg, _T("State Manager - Save Error"), MB_OK | MB_ICONERROR);
	}
}

void CStateManagerWnd::OnSlotLoad()
{
	if (m_nSelectedSlot < 0) {
		MessageBox(m_hWnd, _T("Please select a slot first."), _T("State Manager"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	if (!IsSlotUsed(m_nSelectedSlot)) {
		MessageBox(m_hWnd, _T("Selected slot is empty."), _T("State Manager"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	int result = statsave_load_ext(m_nSelectedSlot);
	if (result == STATFLAG_SUCCESS || (result & STATFLAG_WARNING)) {
		MessageBox(m_hWnd, _T("State loaded successfully."), _T("State Manager"), MB_OK | MB_ICONINFORMATION);
	} else {
		TCHAR errorMsg[256];
		_stprintf_s(errorMsg, _countof(errorMsg),
			_T("Failed to load state from slot %d.\nError code: %d\n\nPossible causes:\n- Save file corrupted\n- Version mismatch\n- File access error"),
			m_nSelectedSlot, result);
		MessageBox(m_hWnd, errorMsg, _T("State Manager - Load Error"), MB_OK | MB_ICONERROR);
	}
}

void CStateManagerWnd::OnSlotDelete()
{
	if (m_nSelectedSlot < 0) {
		MessageBox(m_hWnd, _T("Please select a slot first."), _T("State Manager"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	if (!IsSlotUsed(m_nSelectedSlot)) {
		MessageBox(m_hWnd, _T("Selected slot is already empty."), _T("State Manager"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	if (MessageBox(m_hWnd, _T("Are you sure you want to delete this state?"),
		_T("State Manager"), MB_YESNO | MB_ICONQUESTION) == IDYES) {

		if (statsave_delete_ext(m_nSelectedSlot) == STATFLAG_SUCCESS) {
			Refresh(); // Update display
			MessageBox(m_hWnd, _T("State deleted successfully."), _T("State Manager"), MB_OK | MB_ICONINFORMATION);
		} else {
			MessageBox(m_hWnd, _T("Failed to delete state."), _T("State Manager"), MB_OK | MB_ICONERROR);
		}
	}
}

void CStateManagerWnd::OnSlotDoubleClick(int slot)
{
	m_nSelectedSlot = slot;
	if (IsSlotUsed(slot)) {
		OnSlotLoad();
	} else {
		OnSlotSave();
	}
}

void CStateManagerWnd::OnContextMenu(int slot, POINT pt)
{
	// TODO: Implement context menu
}

void CStateManagerWnd::OnRefresh()
{
	Refresh();
}

// Additional placeholder methods
void CStateManagerWnd::OnSlotComment() {}
void CStateManagerWnd::OnSlotProtect() {}
void CStateManagerWnd::Update() { Refresh(); }
void CStateManagerWnd::OnSlotSelect(int slot) { m_nSelectedSlot = slot; }
HBITMAP CStateManagerWnd::GetSlotThumbnail(int slot) { return NULL; }
void CStateManagerWnd::SetSlotThumbnail(int slot, HBITMAP hBitmap) {}
void CStateManagerWnd::UpdateSlotInList(int slot) {}
void CStateManagerWnd::RemoveSlotFromList(int slot) {}
void CStateManagerWnd::LoadSlotThumbnails() {}
void CStateManagerWnd::GetSlotDisplayInfo(int slot, LPTSTR title, LPTSTR saveTime, LPTSTR fileSize) {}
void CStateManagerWnd::ShowSlotContextMenu(int slot, POINT pt) {}