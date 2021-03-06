#include "stdafx.h"
#include "nkeyrochecker.h"

HWND hDlg = NULL;
UINT idTimer = 0;
SYSTEMTIME stStart;
DWORD dwTickStart = 0;
DWORD dwTickLast = 0;
char keyInput[256];

struct VKCODE_T {
	char vkCode;
	LPCTSTR pszDisplay;
};
VKCODE_T vkCodes[] =
{	VK_RETURN, _T("[ENT]"), VK_BACK, _T("[BS]"), VK_TAB, _T("[HT]"), VK_KANA, _T("[KANA]"), VK_KANJI, _T("[KNJ]"),
	VK_ESCAPE, _T("[ESC]"), VK_CONVERT, _T("[HEN]"), VK_NONCONVERT, _T("[NHN]"), VK_MODECHANGE, _T("[IME]"),
	VK_SPACE, _T("[SP]"), VK_PRIOR, _T("[PUP]"), VK_NEXT, _T("[PDN]"), VK_END, _T("[END]"), VK_HOME, _T("[HOM]"),
	VK_LEFT, _T("[LFT]"), VK_UP, _T("[UP]"), VK_RIGHT, _T("[RGT]"), VK_DOWN, _T("[DWN]"), VK_SNAPSHOT, _T("[SCR]"),
	VK_INSERT, _T("[INS]"), VK_DELETE, _T("[DEL]"), VK_HELP, _T("[HLP]")
};

LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	// http://www.kumei.ne.jp/c_lang/sdk/sdk_30.htm
	
	hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)DlgProc);
	ShowWindow(hDlg, SW_SHOW);
	UpdateWindow(hDlg);

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
		switch (msg.message) {
		case WM_KEYDOWN:
			if (idTimer == 0) {
				ZeroMemory(keyInput, sizeof(keyInput));
				GetLocalTime(&stStart);
				idTimer = SetTimer(hDlg, 100, 300, NULL);
				dwTickStart = GetTickCount();
			}
			{
				char vkCode = (char)(msg.wParam & 0xFF);
				for (int ii = 0; ii < sizeof(keyInput) - 1; ii++) {
					if (keyInput[ii] == vkCode) {
						break;
					}
					if (keyInput[ii] == 0) {
						keyInput[ii] = vkCode;
						dwTickLast = GetTickCount();
						break;
					}
				}
			}
			continue;
		case WM_TIMER:
			if (idTimer == msg.wParam) {
				KillTimer(hDlg, idTimer);
				idTimer = 0;

				DWORD dwTickDuration = dwTickLast - dwTickStart;

				HWND hWndListCtrl = GetDlgItem(hDlg, IDC_LIST_MAIN);

				int curRows = ListView_GetItemCount(hWndListCtrl);
				if (10 <= curRows) {
					ListView_DeleteItem(hWndListCtrl, 9);
				}

				LV_ITEM lvi;
				ZeroMemory(&lvi, sizeof(lvi));

				lvi.mask = LVIF_TEXT;

				TCHAR pszText[256];
				lvi.pszText = pszText;

				ZeroMemory(pszText, sizeof(pszText));
				for (int ii = 0; ii < sizeof(keyInput); ii++) {
					if (keyInput[ii] == 0)
						break;
					if ((0x30 <= keyInput[ii]) && (keyInput[ii] <= 0x5A)) {
						pszText[lstrlen(pszText)] = (TCHAR)keyInput[ii];
						continue;
					}
					bool vkFound = false;
					for (int jj = 0; jj < sizeof(vkCodes) / sizeof(vkCodes[0]); jj++) {
						if (vkCodes[jj].vkCode == keyInput[ii]) {
							lstrcat(pszText, vkCodes[jj].pszDisplay);
							vkFound = true;
							break;
						}
					}
					if (!vkFound) {
						TCHAR codeText[16];
						wsprintf(codeText, TEXT("[0x%2X]"), (unsigned char)keyInput[ii]);
						lstrcat(pszText, codeText);
					}
				}

				int idx = ListView_InsertItem(hWndListCtrl, &lvi);
				lvi.iItem = idx;

				lvi.iSubItem = 1;
				wsprintf(pszText, TEXT("%d"), strlen(keyInput)); // char
				ListView_SetItem(hWndListCtrl, &lvi);

				lvi.iSubItem = 2;
				wsprintf(pszText, TEXT("%02d : %02d : %02d . %03d"), stStart.wHour, stStart.wMinute, stStart.wSecond, stStart.wMilliseconds);
				ListView_SetItem(hWndListCtrl, &lvi);

				lvi.iSubItem = 3;
				wsprintf(pszText, TEXT("%d ms"), dwTickDuration);
				ListView_SetItem(hWndListCtrl, &lvi);
			}

			break;
		}

		if (hDlg == 0 || !IsDialogMessage(hDlg, &msg))
		{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

WCHAR szColumn0[] = TEXT("認識");
WCHAR szColumn1[] = TEXT("数");
WCHAR szColumn2[] = TEXT("時刻");
WCHAR szColumn3[] = TEXT("時間");

LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		HWND hWndListCtrl = GetDlgItem(hWnd, IDC_LIST_MAIN);

		LV_COLUMN lvc;
		ZeroMemory(&lvc, sizeof(lvc));
		lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
		lvc.fmt = LVCFMT_LEFT;
		lvc.pszText = szColumn0;
		lvc.cx = 220;
		ListView_InsertColumn(hWndListCtrl, 0, &lvc);
		lvc.pszText = szColumn1;
		lvc.cx = 50;
		ListView_InsertColumn(hWndListCtrl, 1, &lvc);
		lvc.pszText = szColumn2;
		lvc.cx = 180;
		ListView_InsertColumn(hWndListCtrl, 2, &lvc);
		lvc.pszText = szColumn3;
		lvc.cx = 90;
		ListView_InsertColumn(hWndListCtrl, 3, &lvc);

		ListView_SetExtendedListViewStyle(hWndListCtrl, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

		return TRUE;
	}
	case WM_SYSCOMMAND:
		switch (wp) {
		case SC_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0L);
			return TRUE;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
		case IDCANCEL:
			SendMessage(hWnd, WM_CLOSE, 0, 0L);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return TRUE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return true;
	}
	return (DefWindowProc(hWnd, msg, wp, lp));
}


