/**
 * @file	statemgr.h
 * @brief	State Manager Window
 */

#pragma once

#include "subwnd.h"
#include "../../statsave.h"
#include <commctrl.h>

/**
 * @brief State Manager Window Class
 */
class CStateManagerWnd : public CSubWndBase
{
private:
	// UI Components
	HWND			m_hListView;		// Main list view
	HIMAGELIST		m_hImageList;		// Thumbnail image list
	HWND			m_hStatusBar;		// Status bar
	HWND			m_hToolBar;			// Tool bar

	// Data Management
	NP2SLOT_MASTER	m_SlotMaster;		// Slot information
	int				m_nSelectedSlot;	// Selected slot
	BOOL			m_bInitialized;		// Initialization flag
	HWND			m_hMainWindow;		// Main emulator window handle

	// Constants
	static const int THUMBNAIL_WIDTH = 80;
	static const int THUMBNAIL_HEIGHT = 64;
	static const int GRID_COLS = 5;
	static const int GRID_ROWS = 40;

public:
	CStateManagerWnd();
	virtual ~CStateManagerWnd();

	// Static instance management
	static CStateManagerWnd* GetInstance();
	static void CreateInstance();
	static void DestroyInstance();

	// Window operations
	BOOL Create();
	void Show(BOOL bShow = TRUE);
	void Update();
	void Refresh();

	// Slot operations
	void OnSlotSave();
	void OnSlotLoad();
	void OnSlotDelete();
	void OnSlotComment();
	void OnSlotProtect();

	// UI events
	void OnSlotSelect(int slot);
	void OnSlotDoubleClick(int slot);
	void OnContextMenu(int slot, POINT pt);
	void OnRefresh();

protected:
	// Window procedure
	virtual LRESULT WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);

private:
	// Internal operations
	BOOL InitializeControls();
	void UpdateSlotList();
	void UpdateStatusBar();
	void LoadSlotThumbnails();
	HBITMAP GetSlotThumbnail(int slot);
	void SetSlotThumbnail(int slot, HBITMAP hBitmap);

	// List view operations
	void SetupListView();
	void AddSlotToList(int slot);
	void UpdateSlotInList(int slot);
	void RemoveSlotFromList(int slot);

	// Helper functions
	void GetSlotDisplayInfo(int slot, LPTSTR title, LPTSTR saveTime, LPTSTR fileSize);
	BOOL IsSlotUsed(int slot);
	void ShowSlotContextMenu(int slot, POINT pt);

	// Static instance
	static CStateManagerWnd* s_pInstance;
};

// Menu IDs for state manager
#define IDM_STATEMGR_SAVE		40001
#define IDM_STATEMGR_LOAD		40002
#define IDM_STATEMGR_DELETE		40003
#define IDM_STATEMGR_COMMENT	40004
#define IDM_STATEMGR_PROTECT	40005
#define IDM_STATEMGR_REFRESH	40006

// Control IDs
#define IDC_STATEMGR_LISTVIEW	5001
#define IDC_STATEMGR_STATUSBAR	5002
#define IDC_STATEMGR_TOOLBAR	5003

// Window Type for np2class
#define WNDTYPE_STATEMGR		3