#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <Psapi.h>
#include "resource.h"
#include "../sgshared/sgshared.h"

#include <thread>
#include <regex>

#pragma comment(lib,"ComCtl32.lib")

//MFC资源初始化补充代码

//https://docs.microsoft.com/zh-cn/cpp/mfc/tn024-mfc-defined-messages-and-resources （部分有误）
#define RT_DLGINIT MAKEINTRESOURCE(240)

typedef struct
{
	WORD iControlID;
	WORD iMessage;
	DWORD dwSizeOfData;
}DLGINITHEADER;

BOOL CALLBACK MFCInitDlgCallback(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
{
	HRSRC hRsrc = FindResource(hModule, lpName, lpType);
	if (hRsrc == NULL)
		return FALSE;
	HGLOBAL hRes = LoadResource(hModule, hRsrc);
	if (hRes == NULL)
		return FALSE;
	LPVOID ptr = LockResource(hRes);
	size_t sz = SizeofResource(hModule, hRsrc);
	size_t pos = 0;
	while (sz - pos >= sizeof(DLGINITHEADER))
	{
		DLGINITHEADER* hdr = (DLGINITHEADER*)((LPBYTE)ptr + pos);
		SendDlgItemMessageA((HWND)lParam, hdr->iControlID, CB_ADDSTRING, 0, (LPARAM)((LPBYTE)hdr + sizeof(DLGINITHEADER)));
		pos = pos + sizeof(DLGINITHEADER) + hdr->dwSizeOfData;
	}
	return TRUE;
}

BOOL OnMFCInitDialog(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EnumResourceNames(GetModuleHandle(NULL), RT_DLGINIT, MFCInitDlgCallback, (LPARAM)hWnd);
	return FALSE;
}

//MFC资源初始化补充代码结束


//补充控件功能

HWND CreateToolTipForRectA(HWND hwndParent,LPSTR msg)
{
	// Create a tooltip.
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndParent, NULL, GetModuleHandle(NULL), NULL);

	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Set up "tool" information. In this case, the "tool" is the entire parent window.

	TOOLINFOA ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hwndParent;
	ti.hinst = GetModuleHandle(NULL);
	ti.lpszText = msg;

	GetClientRect(hwndParent, &ti.rect);

	// Associate the tooltip with the "tool" window.
	SendMessage(hwndTT, TTM_ADDTOOLA, 0, (LPARAM)(LPTOOLINFO)&ti);
	SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, 0);
	return hwndTT;
}

void SetToolTipA(HWND hwndParent,HWND hwndTool, LPSTR msg,LPSTR title)
{
	TOOLINFOA ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hwndParent;
	ti.hinst = GetModuleHandle(NULL);
	ti.lpszText = msg;

	SendMessage(hwndTool, TTM_UPDATETIPTEXTA, 0, (LPARAM)&ti);
	SendMessage(hwndTool, TTM_SETTITLEA, 0, (LPARAM)title);
}



#define ID_MENU_LAUNCH_ANOTHER_ARCH 1
#define ID_MENU_ABOUT 2

//对话框回调主函数

#define ONCBK(x) if(x(hWnd,msg,wParam,lParam)==TRUE)return TRUE
#define DECLCBK(x) BOOL x(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)

DECLCBK(OnInitDialog);
DECLCBK(OnMinimize);
DECLCBK(OnRestore);
DECLCBK(OnEndDialog);
DECLCBK(OnCheckTurnOnOff);
DECLCBK(OnSliderSpeed);
DECLCBK(OnButtonModeHelp);
DECLCBK(OnButtonStatusHelp);
DECLCBK(OnButtonStatusFont);
DECLCBK(OnButtonSpeedText);
DECLCBK(OnCheckUseSystemDPI);
DECLCBK(OnComboMode);
DECLCBK(OnComboStatusPosition);
DECLCBK(OnComboStatusBackground);
DECLCBK(OnComboStatusFormat);
DECLCBK(OnPaint);
DECLCBK(OnHotkeyOnOff);
DECLCBK(OnHotkeyResetSpeed);
DECLCBK(OnHotkeySpeedUp);
DECLCBK(OnHotkeySlowDown);
DECLCBK(OnHotkeySlightFaster);
DECLCBK(OnHotkeySlightSlower);
DECLCBK(OnAbout);
DECLCBK(OnLaunchAnotherArch);
DECLCBK(OnSGMessageUpdateList);
INT_PTR OnCtlColorStaticStatusPreview(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK DlgCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:ONCBK(OnInitDialog);break;
	case WM_COMMAND:
		//HIWORD(wParam):0=菜单 1=加速器 其他=控件通知码
		//lParam:控件窗口句柄
		switch (LOWORD(wParam))
		{
		case IDCANCEL:EndDialog(hWnd, 0);ONCBK(OnEndDialog);break;
		case IDC_CHECK_TURN_ON_OFF:ONCBK(OnCheckTurnOnOff);break;
		case IDC_SLIDER_SPEED:ONCBK(OnSliderSpeed);break;
		case IDC_BUTTON_MODE_HELP:ONCBK(OnButtonModeHelp);break;
		case IDC_BUTTON_STATUS_HELP:ONCBK(OnButtonStatusHelp);break;
		case IDC_BUTTON_STATUS_FONT:ONCBK(OnButtonStatusFont);break;
		case IDC_BUTTON_SPEED_TEXT:ONCBK(OnButtonSpeedText);break;
		case IDC_CHECK_USE_SYSTEM_DPI:ONCBK(OnCheckUseSystemDPI);break;
		case IDC_COMBO_MODE:ONCBK(OnComboMode);break;
		case IDC_COMBO_STATUS_POSITION:ONCBK(OnComboStatusPosition);break;
		case IDC_COMBO_STATUS_BACKGROUND:ONCBK(OnComboStatusBackground);break;
		case IDC_COMBO_STATUS_FORMAT:ONCBK(OnComboStatusFormat);break;
		case IDC_HOTKEY_ON_OFF:ONCBK(OnHotkeyOnOff);break;
		case IDC_HOTKEY_RESET_SPEED:ONCBK(OnHotkeyResetSpeed);break;
		case IDC_HOTKEY_SPEED_UP:ONCBK(OnHotkeySpeedUp);break;
		case IDC_HOTKEY_SLOW_DOWN:ONCBK(OnHotkeySlowDown);break;
		case IDC_HOTKEY_SLIGHT_FASTER:ONCBK(OnHotkeySlightFaster);break;
		case IDC_HOTKEY_SLIGHT_SLOWER:ONCBK(OnHotkeySlightSlower);break;
		}
		break;
	case WM_PAINT:ONCBK(OnPaint);break;
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_MINIMIZE:ONCBK(OnMinimize);break;
		case SC_RESTORE:ONCBK(OnRestore);break;
		case ID_MENU_ABOUT:ONCBK(OnAbout);break;
		case ID_MENU_LAUNCH_ANOTHER_ARCH:ONCBK(OnLaunchAnotherArch);break;
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_CUSTOMDRAW:
			switch (((LPNMHDR)lParam)->idFrom)
			{
			case IDC_SLIDER_SPEED:ONCBK(OnSliderSpeed);break;
			}
			break;
		}
		break;
	case WM_CTLCOLORSTATIC:
		if (GetDlgCtrlID((HWND)lParam) == IDC_STATIC_STATUS_PREVIEW)
			return OnCtlColorStaticStatusPreview(hWnd, msg, wParam, lParam);
		break;
	case SG_MESSAGE_UPDATE_LIST:ONCBK(OnSGMessageUpdateList); break;
	}
	return 0;
}

int WINAPI _tWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPTSTR param, _In_ int iShow)
{
	INITCOMMONCONTROLSEX iex = { sizeof(INITCOMMONCONTROLSEX),ICC_BAR_CLASSES };
	InitCommonControlsEx(&iex);
	return (int)DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, DlgCallback);
}



//程序主要功能实现

#define APPNAME "SpeedGear"
#ifdef _M_IX86
#define DLGTITLE APPNAME " x86"
#else
#define DLGTITLE APPNAME " x64"
#endif

BOOL IsMyAppFocused()
{
	//获取我的进程PID
	DWORD pid;
	GetWindowThreadProcessId(GetActiveWindow(), &pid);
	return pid == GetCurrentProcessId();
}

#define SPEEDGEAR_MIN_SPEED 0.125f
#define SPEEDGEAR_MAX_SPEED 8.0f

#define SPEEDGEAR_BEEP_SPEED_UP 1
#define SPEEDGEAR_BEEP_SLOW_DOWN 2
#define SPEEDGEAR_BEEP_ORIGINAL 0
#define SPEEDGEAR_BEEP_ERROR 3
#define SPEEDGEAR_BEEP_PREC_UP 4
#define SPEEDGEAR_BEEP_PREC_DOWN 5

#define SPEEDGEAR_PRECISE_ADJUSTMENT 0.125f

void SpeedGearBeep(int type)
{
	std::thread([](int type)
		{
			switch (type)
			{
			case SPEEDGEAR_BEEP_ORIGINAL:Beep(1000, 100); break;
			case SPEEDGEAR_BEEP_SPEED_UP:Beep(2000, 100); break;
			case SPEEDGEAR_BEEP_SLOW_DOWN:Beep(500, 100); break;
			case SPEEDGEAR_BEEP_ERROR:default:Beep(750, 500); break;
			case SPEEDGEAR_BEEP_PREC_UP:Beep(DWORD(1000 * (1 + SPEEDGEAR_PRECISE_ADJUSTMENT)), 100); break;
			case SPEEDGEAR_BEEP_PREC_DOWN:Beep(DWORD(1000 * (1 - SPEEDGEAR_PRECISE_ADJUSTMENT)), 100); break;
			}
			if (type != SPEEDGEAR_BEEP_ERROR && type != SPEEDGEAR_BEEP_ORIGINAL)
				Beep(DWORD(1000 * SpeedGear_GetSharedMemory()->hookSpeed), 100);
		}, type).detach();
}

BOOL InitSpeedSlider(HWND hwnd)
{
	HWND hSlider = GetDlgItem(hwnd, IDC_SLIDER_SPEED);
	SendMessage(hSlider, TBM_SETRANGE, FALSE, MAKELPARAM(0, 48));
	SendMessage(hSlider, TBM_SETPOS, TRUE, 24);
	return TRUE;
}

void SetSpeedSlider(HWND hwnd, float speed)
{
	char buf[16] = "";
	sprintf_s(buf, ARRAYSIZE(buf), "%.3f&x", speed);
	SetDlgItemTextA(hwnd, IDC_BUTTON_SPEED_TEXT, buf);
	float v = 24.0f + log2f(speed) * 24.0f / 3.0f;
	SendMessage(GetDlgItem(hwnd, IDC_SLIDER_SPEED), TBM_SETPOS, TRUE, (LPARAM)v);
}

float GetSpeedSlider(HWND hwnd)
{
	return powf(2.0f, (SendMessage(GetDlgItem(hwnd, IDC_SLIDER_SPEED), TBM_GETPOS, 0, 0) - 24.0f) * 3.0f / 24.0f);
}

BOOL InitProcessList(HWND hwnd)
{
	HWND hList = GetDlgItem(hwnd, IDC_LIST_PROCESS);
	SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	LVCOLUMN col{};
	col.mask = LVCF_WIDTH | LVCF_IMAGE;
	col.cx = DPI_SCALED_VALUE(hList, 24);
	ListView_InsertColumn(hList, 0, &col);
	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	col.fmt = LVCFMT_RIGHT;
	int baseWidth = DPI_SCALED_VALUE(hList, 60);
	col.cx = baseWidth;
	col.pszText = TEXT("PID");
	ListView_InsertColumn(hList, 1, &col);
	col.fmt = LVCFMT_LEFT;
	col.cx = baseWidth * 4;
	TCHAR buf[16] = TEXT("");
	LoadString(NULL, IDS_STRING_PROCESS_NAME, buf, ARRAYSIZE(buf));
	col.pszText = buf;
	ListView_InsertColumn(hList, 2, &col);
	col.cx = baseWidth;
	col.pszText = TEXT("API");
	ListView_InsertColumn(hList, 3, &col);
	return TRUE;
}

char _iniSaveIntBuf[16] = "";
#define INI_READ_INT(key) GetPrivateProfileIntA("SpeedGear",key,0,".\\sg.ini")
#define INI_READ_INT2(key,def) GetPrivateProfileIntA("SpeedGear",key,def,".\\sg.ini")
#define INI_SAVE_INT(key,value) wsprintfA(_iniSaveIntBuf,"%d",value);WritePrivateProfileStringA("SpeedGear",key,_iniSaveIntBuf,".\\sg.ini")
#define INI_READ_STR(key,pStr,szStr) GetPrivateProfileStringA("SpeedGear",key,"",pStr,szStr,".\\sg.ini")
#define INI_READ_STR2(key,pStr,szStr,def) GetPrivateProfileStringA("SpeedGear",key,def,pStr,szStr,".\\sg.ini")
#define INI_SAVE_STR(key,pStr) WritePrivateProfileStringA("SpeedGear",key,pStr,".\\sg.ini")

LOGFONTA g_logFont = { 0 };
CHOOSEFONTA g_cf = { sizeof(CHOOSEFONTA),0,0,&g_logFont,0,CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_SCREENFONTS,0 };
HWND hMain;

typedef struct
{
	BOOL hookIsOn;
	WORD keyOnOff;
	WORD keyResetSpeed;
	WORD keySpeedUp;
	WORD keySlowDown;
	WORD keySlightFaster;
	WORD keySlightSlower;
}GUI_MEMORY;
GUI_MEMORY guiMem;

int GetEditComboBoxText(HWND hwndCtrl, char* pBuf, int len);
void RefreshPreviewText(HWND hwnd)
{
	int p = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_COMBO_STATUS_POSITION));
	HWND hStatic = GetDlgItem(hwnd, IDC_STATIC_STATUS_PREVIEW);
	switch (p % 3)
	{
	case 0:SetWindowLong(hStatic, GWL_STYLE, (GetWindowStyle(hStatic) & 0xFFFFFFFC) | SS_LEFT); break;
	case 1:SetWindowLong(hStatic, GWL_STYLE, (GetWindowStyle(hStatic) & 0xFFFFFFFC) | SS_CENTER); break;
	case 2:SetWindowLong(hStatic, GWL_STYLE, (GetWindowStyle(hStatic) & 0xFFFFFFFC) | SS_RIGHT); break;
	}
	switch (p / 3)
	{
	case 0:SetWindowLong(hStatic, GWL_STYLE, GetWindowStyle(hStatic) & ~SS_CENTERIMAGE); break;
	case 1:SetWindowLong(hStatic, GWL_STYLE, GetWindowStyle(hStatic) | SS_CENTERIMAGE); break;
	case 2:SetWindowLong(hStatic, GWL_STYLE, GetWindowStyle(hStatic) | SS_CENTERIMAGE); break;
	}
	char text[256] = "", fmttext[256] = "";
	GetEditComboBoxText(GetDlgItem(hwnd, IDC_COMBO_STATUS_FORMAT), text, ARRAYSIZE(text));
	SpeedGear_FormatText(fmttext, ARRAYSIZE(fmttext), text, 1.0f, 60, 800, 600, 9, 0, 0,"API");
	SetDlgItemTextA(hwnd, IDC_STATIC_STATUS_PREVIEW, fmttext);
}

HWND hFontTip;

void SetButtonFontText(HWND hwnd)
{
	char buf[256] = "";
	char szFontSettings[32] = "", szItalic[16] = ",";
	LoadStringA(NULL, IDS_STRING_FONT_SETTINGS, szFontSettings, ARRAYSIZE(szFontSettings));
	LoadStringA(NULL, IDS_STRING_ITALIC, szItalic + 1, ARRAYSIZE(szItalic) - sizeof(*buf));
	wsprintfA(buf, "%s(&T) [%s,%d%s,%d,#%06X]", szFontSettings, g_logFont.lfFaceName, g_logFont.lfWeight, g_logFont.lfItalic ? szItalic : "", PIXEL_TO_LOGICAL_UNIT(hwnd, abs(g_logFont.lfHeight)), g_cf.rgbColors);
	if (lstrlenA(g_logFont.lfFaceName) == 0)
		*strchr(buf, ' ') = 0;
	SetDlgItemTextA(hwnd, IDC_BUTTON_STATUS_FONT, buf);
	wsprintfA(buf, "%s,%d%s,%d,#%06X", g_logFont.lfFaceName, g_logFont.lfWeight, g_logFont.lfItalic ? szItalic : "", PIXEL_TO_LOGICAL_UNIT(hwnd, abs(g_logFont.lfHeight)), g_cf.rgbColors);
	SetToolTipA(GetDlgItem(hwnd, IDC_BUTTON_STATUS_FONT), hFontTip, buf, szFontSettings);
	HFONT hFont = CreateFontA(g_logFont.lfHeight, 0, 0, 0, g_logFont.lfWeight, g_logFont.lfItalic, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, g_logFont.lfFaceName);
	HWND hStatic = GetDlgItem(hwnd, IDC_STATIC_STATUS_PREVIEW);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
}

BOOL GuiReadMem(HWND hwnd)
{
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_COMBO_MODE), pMem->hookMode);
	SetWindowTextA(GetDlgItem(hwnd, IDC_COMBO_STATUS_FORMAT), pMem->statusFormat);
	ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_COMBO_STATUS_POSITION), pMem->statusPosition);

	lstrcpyA(g_logFont.lfFaceName, pMem->fontName);
	g_logFont.lfWeight = pMem->fontWeight;
	g_logFont.lfItalic = pMem->fontItalic;
	g_logFont.lfHeight = -LOGICAL_UNIT_TO_PIXEL(hwnd, pMem->fontSize);
	g_cf.rgbColors = pMem->fontColor;

	SetButtonFontText(hwnd);
	CheckDlgButton(hwnd, IDC_CHECK_USE_SYSTEM_DPI, pMem->useSystemDPI);

	CheckDlgButton(hwnd, IDC_CHECK_TURN_ON_OFF, guiMem.hookIsOn);
	SetSpeedSlider(hwnd, pMem->hookSpeed);

	SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_ON_OFF), HKM_SETHOTKEY, (WPARAM)guiMem.keyOnOff, 0);
	SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_RESET_SPEED), HKM_SETHOTKEY, (WPARAM)guiMem.keyResetSpeed, 0);
	SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_SPEED_UP), HKM_SETHOTKEY, (WPARAM)guiMem.keySpeedUp, 0);
	SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_SLOW_DOWN), HKM_SETHOTKEY, (WPARAM)guiMem.keySlowDown, 0);
	SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_SLIGHT_FASTER), HKM_SETHOTKEY, (WPARAM)guiMem.keySlightFaster, 0);
	SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_SLIGHT_SLOWER), HKM_SETHOTKEY, (WPARAM)guiMem.keySlightSlower, 0);
	return TRUE;
}

int GetEditComboBoxText(HWND hwndCtrl, char* pBuf, int len)
{
	int s = ComboBox_GetCurSel(hwndCtrl);
	if (s == -1)
		return GetWindowTextA(hwndCtrl, pBuf, len);
	return (int)SendMessageA(hwndCtrl, CB_GETLBTEXT, (WPARAM)s, (LPARAM)pBuf);
}

BOOL GuiSaveMem(HWND hwnd)
{
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	pMem->hookMode = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_COMBO_MODE));
	GetEditComboBoxText(GetDlgItem(hwnd, IDC_COMBO_STATUS_FORMAT), pMem->statusFormat, ARRAYSIZE(pMem->statusFormat));
	pMem->statusPosition = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_COMBO_STATUS_POSITION));

	lstrcpyA(pMem->fontName, g_logFont.lfFaceName);
	pMem->fontWeight = g_logFont.lfWeight;
	pMem->fontItalic = g_logFont.lfItalic;
	pMem->fontSize = PIXEL_TO_LOGICAL_UNIT(hwnd, abs(g_logFont.lfHeight));
	pMem->fontColor = g_cf.rgbColors;
	pMem->useSystemDPI = IsDlgButtonChecked(hwnd, IDC_CHECK_USE_SYSTEM_DPI);

	guiMem.hookIsOn = IsDlgButtonChecked(hwnd, IDC_CHECK_TURN_ON_OFF);
	pMem->hookSpeed = GetSpeedSlider(hwnd);

	guiMem.keyOnOff = (WORD)SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_ON_OFF), HKM_GETHOTKEY, 0, 0);
	guiMem.keyResetSpeed = (WORD)SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_RESET_SPEED), HKM_GETHOTKEY, 0, 0);
	guiMem.keySpeedUp = (WORD)SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_SPEED_UP), HKM_GETHOTKEY, 0, 0);
	guiMem.keySlowDown = (WORD)SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_SLOW_DOWN), HKM_GETHOTKEY, 0, 0);
	guiMem.keySlightFaster = (WORD)SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_SLIGHT_FASTER), HKM_GETHOTKEY, 0, 0);
	guiMem.keySlightSlower = (WORD)SendMessage(GetDlgItem(hwnd, IDC_HOTKEY_SLIGHT_SLOWER), HKM_GETHOTKEY, 0, 0);
	return TRUE;
}

BOOL MemReadIni()
{
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	pMem->hookMode = INI_READ_INT2("hookMode",1);
	INI_READ_STR2("statusFormat", pMem->statusFormat, ARRAYSIZE(pMem->statusFormat),"FPS:{fps:%3d}");
	pMem->statusPosition = INI_READ_INT("statusPosition");
	INI_READ_STR2("fontName", pMem->fontName, ARRAYSIZE(pMem->fontName),"SimSun");
	char szDefaultPath[MAX_PATH] = "";
	size_t c;
	getenv_s(&c, szDefaultPath, "windir");
	lstrcatA(szDefaultPath, "\\Fonts\\SimSun.ttc:0");
	INI_READ_STR2("fontPath", pMem->fontPath, ARRAYSIZE(pMem->fontPath),szDefaultPath);
	pMem->useSystemDPI = INI_READ_INT2("useSystemDPI",TRUE);
	pMem->fontSize = INI_READ_INT2("fontSize",24);
	pMem->fontItalic = INI_READ_INT("fontItalic");
	pMem->fontWeight = INI_READ_INT2("fontWeight",400);
	pMem->fontColor = INI_READ_INT2("fontColor",0x0000FFFF);
	guiMem.keyOnOff = INI_READ_INT("keyOnOff");
	guiMem.keyResetSpeed = INI_READ_INT("keyResetSpeed");
	guiMem.keySpeedUp = INI_READ_INT("keySpeedUp");
	guiMem.keySlowDown = INI_READ_INT("keySlowDown");
	guiMem.keySlightFaster = INI_READ_INT("keySlightFaster");
	guiMem.keySlightSlower = INI_READ_INT("keySlightSlower");
	return TRUE;
}

BOOL MemSaveIni()
{
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	INI_SAVE_INT("hookMode", pMem->hookMode);
	INI_SAVE_STR("statusFormat", pMem->statusFormat);
	INI_SAVE_INT("statusPosition", pMem->statusPosition);
	INI_SAVE_STR("fontName", pMem->fontName);
	INI_SAVE_STR("fontPath", pMem->fontPath);
	INI_SAVE_INT("fontSize", pMem->fontSize);
	INI_SAVE_INT("fontItalic", pMem->fontItalic);
	INI_SAVE_INT("fontWeight", pMem->fontWeight);
	INI_SAVE_INT("fontColor", pMem->fontColor);
	INI_SAVE_INT("useSystemDPI", pMem->useSystemDPI);
	INI_SAVE_INT("keyOnOff", guiMem.keyOnOff);
	INI_SAVE_INT("keyResetSpeed", guiMem.keyResetSpeed);
	INI_SAVE_INT("keySpeedUp", guiMem.keySpeedUp);
	INI_SAVE_INT("keySlowDown", guiMem.keySlowDown);
	INI_SAVE_INT("keySlightFaster", guiMem.keySlightFaster);
	INI_SAVE_INT("keySlightSlower", guiMem.keySlightSlower);
	return TRUE;
}

HHOOK hHookKb = NULL;

BOOL IsKeyPress(DWORD vk, WORD vc)
{
	if (HIBYTE(vc) & HOTKEYF_ALT)
	{
		if ((GetAsyncKeyState(VK_MENU) & 0x8000) == 0)
			return FALSE;
	}
	if (HIBYTE(vc) & HOTKEYF_SHIFT)
	{
		if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0)
			return FALSE;
	}
	if (HIBYTE(vc) & HOTKEYF_CONTROL)
	{
		if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0)
			return FALSE;
	}
	return LOBYTE(vc) == (vk & 0xFF);
}

BOOL StartSpeedGear();
BOOL StopSpeedGear();

LRESULT CALLBACK KbHookProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		switch (wParam)
		{
		case WM_KEYUP:case WM_SYSKEYUP:
			if (!IsMyAppFocused())
			{
				SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
				PKBDLLHOOKSTRUCT pk = (PKBDLLHOOKSTRUCT)lParam;
				if (IsKeyPress(pk->vkCode, guiMem.keyOnOff))
				{
					guiMem.hookIsOn = (guiMem.hookIsOn + 1) % 2;
					GuiReadMem(hMain);
					if (guiMem.hookIsOn)
						StartSpeedGear();
					else
						StopSpeedGear();
				}
				else if (guiMem.hookIsOn)
				{
					if (IsKeyPress(pk->vkCode, guiMem.keyResetSpeed))
					{
						pMem->hookSpeed = 1.0f;
						GuiReadMem(hMain);
						SpeedGearBeep(SPEEDGEAR_BEEP_ORIGINAL);
					}
					else if (IsKeyPress(pk->vkCode, guiMem.keySpeedUp))
					{
						if (pMem->hookSpeed == SPEEDGEAR_MAX_SPEED)
						{
							SpeedGearBeep(SPEEDGEAR_BEEP_ERROR);
						}
						else
						{
							pMem->hookSpeed = min(pMem->hookSpeed * 2.0f, SPEEDGEAR_MAX_SPEED);
							GuiReadMem(hMain);
							SpeedGearBeep(SPEEDGEAR_BEEP_SPEED_UP);
						}
					}
					else if (IsKeyPress(pk->vkCode, guiMem.keySlowDown))
					{
						if (pMem->hookSpeed == SPEEDGEAR_MIN_SPEED)
						{
							SpeedGearBeep(SPEEDGEAR_BEEP_ERROR);
						}
						else
						{
							pMem->hookSpeed = max(pMem->hookSpeed / 2.0f, SPEEDGEAR_MIN_SPEED);
							GuiReadMem(hMain);
							SpeedGearBeep(SPEEDGEAR_BEEP_SLOW_DOWN);
						}
					}
					else if (IsKeyPress(pk->vkCode, guiMem.keySlightFaster))
					{
						if (pMem->hookSpeed == SPEEDGEAR_MAX_SPEED)
						{
							SpeedGearBeep(SPEEDGEAR_BEEP_ERROR);
						}
						else
						{
							pMem->hookSpeed = min(pMem->hookSpeed + SPEEDGEAR_PRECISE_ADJUSTMENT, SPEEDGEAR_MAX_SPEED);
							GuiReadMem(hMain);
							SpeedGearBeep(SPEEDGEAR_BEEP_PREC_UP);
						}
					}
					else if (IsKeyPress(pk->vkCode, guiMem.keySlightSlower))
					{
						if (pMem->hookSpeed == SPEEDGEAR_MIN_SPEED)
						{
							SpeedGearBeep(SPEEDGEAR_BEEP_ERROR);
						}
						else
						{
							pMem->hookSpeed = max(pMem->hookSpeed - SPEEDGEAR_PRECISE_ADJUSTMENT, SPEEDGEAR_MIN_SPEED);
							GuiReadMem(hMain);
							SpeedGearBeep(SPEEDGEAR_BEEP_PREC_DOWN);
						}
					}
				}
			}
			break;
		}
	}
	return CallNextHookEx(hHookKb, nCode, wParam, lParam);
}

HHOOK hHookSGList[8] = { NULL };

BOOL InitKbHook()
{
	if (hHookKb)
		return FALSE;
	hHookKb = SetWindowsHookEx(WH_KEYBOARD_LL, KbHookProc, GetModuleHandle(NULL), NULL);
	return TRUE;
}

BOOL ReleaseKbHook()
{
	return UnhookWindowsHookEx(hHookKb);
}

BOOL StartSpeedGear()
{
	int hookType = INI_READ_INT2("hookType",WH_SHELL);
	char* dllName[] = {
#ifdef _M_IX86
		"sgd8.dll","sgd9.dll","sgd11.dll","sggl.dll","sgd10.dll","sgdd.dll","sgvk.dll","sgd12.dll"
#else
		"sg64d9.dll","sg64d11.dll","sg64gl.dll","sg64d10.dll","sg64dd.dll","sg64vk.dll","sg64d12.dll"
#endif
	};
#ifdef _M_IX86
	static_assert(ARRAYSIZE(hHookSGList) == ARRAYSIZE(dllName), "Count doesn't match.");
#else
	static_assert(ARRAYSIZE(hHookSGList) == ARRAYSIZE(dllName) + 1, "Count doesn't match.");
#endif
	char buf[256] = "";
	int success_count = 0;
	lstrcpyA(buf, DLGTITLE " - ");
	int len = lstrlenA(buf);
	LoadStringA(GetModuleHandle(NULL), IDS_STRING_RUNNING, buf + len, ARRAYSIZE(buf) - len * sizeof(*buf));
	for (int i = 0; i < ARRAYSIZE(dllName); i++)
	{
		if (hHookSGList[i])
			UnhookWindowsHookEx(hHookSGList[i]);
		HMODULE hDll = LoadLibraryA(dllName[i]);
		/*if (hDll == NULL)
		{
			StopSpeedGear();
			char msg[256] = "";
			DWORD err = GetLastError();
			wsprintfA(msg, "%#x\n", err);
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, GetUserDefaultLangID(), buf, ARRAYSIZE(buf), NULL);
			lstrcatA(msg, buf);
			MessageBoxA(NULL, msg, NULL, MB_ICONERROR);
			return FALSE;
		}*/
		if (hDll)
		{
			HOOKPROC fProc = (HOOKPROC)GetProcAddress(hDll, SPEEDGEAR_PROC_STR);
			hHookSGList[i] = SetWindowsHookEx(hookType, fProc, hDll, 0);
		}
		/*if (hHookSGList[i] == NULL)
		{
			StopSpeedGear();
			TCHAR msg[16] = TEXT("");
			LoadString(GetModuleHandle(NULL), IDS_STRING_CANNOT_START_HOOK, buf, ARRAYSIZE(buf));
			wsprintf(msg, TEXT("%#x"), GetLastError());
			MessageBox(NULL, buf, msg, MB_ICONERROR);
			return FALSE;
		}*/
		if (hHookSGList[i])
		{
			if (success_count == 0)
				lstrcatA(buf, " [");
			else
				lstrcatA(buf, ", ");
			lstrcatA(buf, dllName[i]);
			success_count++;
		}
	}
	if (success_count > 0)
		lstrcatA(buf, "]");
	SetWindowTextA(hMain, buf);
	return TRUE;
}

BOOL StopSpeedGear()
{
	for (int i = 0; i < ARRAYSIZE(hHookSGList); i++)
	{
		if (hHookSGList[i])
		{
			UnhookWindowsHookEx(hHookSGList[i]);
			hHookSGList[i] = NULL;
		}
	}
	SetWindowTextA(hMain, DLGTITLE);
	return TRUE;
}


//回调函数实现

BOOL OnInitDialog(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//调用MFC资源的初始化
	OnMFCInitDialog(hWnd, msg, wParam, lParam);
	hMain = hWnd;
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_SG2));
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	hFontTip = CreateToolTipForRectA(GetDlgItem(hWnd, IDC_BUTTON_STATUS_FONT), NULL);
	char buf[256] = "";
	LoadStringA(NULL, IDS_STRING_TIP_USE_SYSTEM_API, buf, ARRAYSIZE(buf));
	CreateToolTipForRectA(GetDlgItem(hWnd, IDC_CHECK_USE_SYSTEM_DPI), buf);
	LoadStringA(NULL, IDS_STRING_TIP_RESET_SPEED, buf, ARRAYSIZE(buf));
	CreateToolTipForRectA(GetDlgItem(hWnd, IDC_BUTTON_SPEED_TEXT), buf);
	SetWindowTextA(hWnd, DLGTITLE);
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	AppendMenuA(hMenu, MF_SEPARATOR, 0, 0);
#ifdef _M_IX86
	LoadStringA(NULL, IDS_STRING_LAUNCH_ARCH_X64, buf, ARRAYSIZE(buf));
#else
	LoadStringA(NULL, IDS_STRING_LAUNCH_ARCH_X86, buf, ARRAYSIZE(buf));
#endif
	AppendMenuA(hMenu, MF_STRING, ID_MENU_LAUNCH_ANOTHER_ARCH, buf);
	LoadStringA(NULL, IDS_STRING_MENU_ABOUT, buf, ARRAYSIZE(buf));
	AppendMenuA(hMenu, MF_STRING, ID_MENU_ABOUT, buf);

	InitSpeedSlider(hWnd);
	ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_COMBO_STATUS_BACKGROUND), 0);

	InitProcessList(hWnd);

	if (SpeedGear_InitializeSharedMemory(TRUE) == FALSE)
	{
		LoadStringA(NULL, IDS_STRING_INIT_SM_FAIL, buf, ARRAYSIZE(buf));
		MessageBoxA(hWnd, buf, NULL, MB_ICONERROR);
		return FALSE;
	}
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	pMem->hwndGui = (DWORD)hMain;
	pMem->hookSpeed = 1.0f;
	MemReadIni();
	GuiReadMem(hWnd);
	RefreshPreviewText(hWnd);
	InitKbHook();

	ComboBox_DeleteString(GetDlgItem(hWnd, IDC_COMBO_MODE), 0);
	ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_COMBO_MODE), 0);

	return TRUE;
}

BOOL OnMinimize(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//TODO:最小化到托盘
	return FALSE;
}

BOOL OnRestore(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//TODO:恢复窗口
	return FALSE;
}

BOOL OnEndDialog(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ReleaseKbHook();
	if (SpeedGear_ReleaseSharedMemory() == FALSE)
	{
		char buf[256] = "";
		LoadStringA(NULL, IDS_STRING_RELEASE_SM_FAIL, buf, ARRAYSIZE(buf));
		MessageBoxA(hWnd, buf, NULL, MB_ICONERROR);
	}
	return FALSE;
}

BOOL OnCheckTurnOnOff(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (IsDlgButtonChecked(hWnd, IDC_CHECK_TURN_ON_OFF))
	{
		if (!StartSpeedGear())
			CheckDlgButton(hWnd, IDC_CHECK_TURN_ON_OFF, BST_UNCHECKED);
	}
	else
		StopSpeedGear();
	return FALSE;
}

BOOL OnSliderSpeed(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char buf[16] = "";
	sprintf_s(buf, ARRAYSIZE(buf), "%.3f&x", GetSpeedSlider(hWnd));
	SetDlgItemTextA(hWnd, IDC_BUTTON_SPEED_TEXT, buf);
	GuiSaveMem(hWnd);
	MemSaveIni();
	return TRUE;
}

BOOL OnButtonModeHelp(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char buf[256] = "", szHelp[16] = "";
	LoadStringA(NULL, IDS_STRING_HELP, szHelp, ARRAYSIZE(szHelp));
	LoadStringA(NULL, IDS_STRING_MODE_HELP, buf, ARRAYSIZE(buf));
#ifdef _M_IX86
	lstrcatA(buf, "Direct3D 8, ");
#endif
	lstrcatA(buf, "Direct3D 9, Direct3D 10, Direct3D 11, Direct3D 12, DirectDraw, OpenGL, Vulkan");
	MessageBoxA(hWnd, buf, szHelp, MB_OK);
	return TRUE;
}

BOOL OnButtonStatusHelp(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char buf[256] = "", szHelp[16] = "";
	LoadStringA(NULL, IDS_STRING_HELP, szHelp, ARRAYSIZE(szHelp));
	LoadStringA(NULL, IDS_STRING_STATUS_FMT_HELP, buf, ARRAYSIZE(buf));
	MessageBoxA(hWnd, buf, szHelp, MB_OK);
	return TRUE;
}

BOOL OnButtonStatusFont(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	g_cf.hwndOwner = hWnd;
	if (ChooseFontA(&g_cf))
	{
		SetButtonFontText(hWnd);
		GuiSaveMem(hWnd);
		MemSaveIni();
		InvalidateRect(hWnd, NULL, FALSE);
		return TRUE;
	}
	return FALSE;
}

BOOL OnButtonSpeedText(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SpeedGear_GetSharedMemory()->hookSpeed = 1.0f;
	SetSpeedSlider(hWnd, 1.0f);
	return TRUE;
}

BOOL OnCheckUseSystemDPI(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GuiSaveMem(hWnd);
	MemSaveIni();
	return TRUE;
}

BOOL OnComboMode(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) != CBN_SELCHANGE)
		return FALSE;
	GuiSaveMem(hWnd);
	MemSaveIni();
	return TRUE;
}

BOOL OnComboStatusPosition(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) != CBN_SELCHANGE)
		return FALSE;
	GuiSaveMem(hWnd);
	MemSaveIni();
	RefreshPreviewText(hWnd);
	InvalidateRect(hWnd, NULL, FALSE);
	return TRUE;
}

BOOL OnComboStatusBackground(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RefreshPreviewText(hWnd);
	InvalidateRect(hWnd, NULL, FALSE);
	return TRUE;
}

BOOL OnComboStatusFormat(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (HIWORD(wParam))
	{
	case CBN_EDITCHANGE:
	case CBN_SELCHANGE:
		GuiSaveMem(hWnd);
		MemSaveIni();
		RefreshPreviewText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	return TRUE;
}

BOOL OnPaint(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int pos = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_COMBO_STATUS_POSITION));
	if (pos < 6)
		return FALSE;
	PAINTSTRUCT ps;
	HWND hStatic = GetDlgItem(hWnd, IDC_STATIC_STATUS_PREVIEW);
	HDC hdc = BeginPaint(hStatic, &ps);
	int sob[] = { BLACK_BRUSH,WHITE_BRUSH,GRAY_BRUSH,LTGRAY_BRUSH };
	HBRUSH hbr = (HBRUSH)GetStockObject(sob[ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_COMBO_STATUS_BACKGROUND))]);
	RECT r;
	GetClientRect(hStatic, &r);
	FillRect(hdc, &r, hbr);
	char t[256] = "";
	GetDlgItemTextA(hWnd, IDC_STATIC_STATUS_PREVIEW, t, ARRAYSIZE(t));
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, g_cf.rgbColors);
	HFONT hOldFont = SelectFont(hdc, CreateFontIndirectA(&g_logFont));
	DrawTextA(hdc, t, lstrlenA(t), &r, DT_SINGLELINE | DT_BOTTOM | (pos - 6));
	DeleteObject(SelectFont(hdc, hOldFont));
	EndPaint(hStatic, &ps);
	return FALSE;
}

INT_PTR OnCtlColorStaticStatusPreview(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdcStatic = (HDC)wParam;
	SetBkMode(hdcStatic, TRANSPARENT);
	SetTextColor(hdcStatic, g_cf.rgbColors);
	int sob[] = { BLACK_BRUSH,WHITE_BRUSH,GRAY_BRUSH,LTGRAY_BRUSH };
	//SetBkColor(hdcStatic, RGB(0, 255, 0));//文字背景色，如果不设置BkMode为透明的话则需要设置此项
	return (INT_PTR)GetStockObject(sob[ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_COMBO_STATUS_BACKGROUND))]);//空白区背景色
}

BOOL OnHotkeyOnOff(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == EN_CHANGE)
	{
		GuiSaveMem(hWnd);
		MemSaveIni();
	}
	return TRUE;
}

BOOL OnHotkeyResetSpeed(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == EN_CHANGE)
	{
		GuiSaveMem(hWnd);
		MemSaveIni();
	}
	return TRUE;
}

BOOL OnHotkeySpeedUp(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == EN_CHANGE)
	{
		GuiSaveMem(hWnd);
		MemSaveIni();
	}
	return TRUE;
}

BOOL OnHotkeySlowDown(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == EN_CHANGE)
	{
		GuiSaveMem(hWnd);
		MemSaveIni();
	}
	return TRUE;
}

BOOL OnHotkeySlightFaster(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == EN_CHANGE)
	{
		GuiSaveMem(hWnd);
		MemSaveIni();
	}
	return TRUE;
}

BOOL OnHotkeySlightSlower(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == EN_CHANGE)
	{
		GuiSaveMem(hWnd);
		MemSaveIni();
	}
	return TRUE;
}

BOOL OnAbout(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TASKDIALOGCONFIG tdc{};
	tdc.cbSize = sizeof(tdc);
	tdc.hwndParent = hWnd;
	tdc.hInstance = GetModuleHandle(NULL);
	tdc.dwFlags = TDF_ENABLE_HYPERLINKS;
	tdc.dwCommonButtons = TDCBF_OK_BUTTON;
	tdc.pszWindowTitle = TEXT(APPNAME);
	tdc.pszMainIcon = TD_INFORMATION_ICON;
	TCHAR buf[256] = TEXT("");
	LoadString(NULL, IDS_STRING_ABOUT_MSG, buf, ARRAYSIZE(buf));
	std::wstring msgWithURL = buf;
	msgWithURL = std::regex_replace(msgWithURL, std::basic_regex<TCHAR>(TEXT("(https?://[A-Za-z0-9_\\-\\./]+)")), TEXT("<a href=\"$1\">$1</a>"));
	tdc.pszContent = msgWithURL.c_str();
	tdc.pfCallback = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
	{
		switch (msg)
		{
		case TDN_HYPERLINK_CLICKED:
			ShellExecute(hwnd, TEXT("open"), (LPCWSTR)lParam, NULL, NULL, SW_SHOWNORMAL);
			break;
		}
		return S_OK;
	};
	TaskDialogIndirect(&tdc, NULL, NULL, NULL);
	return TRUE;
}

BOOL OnLaunchAnotherArch(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#ifdef _M_IX86
#define LAUNCH_FILE "sg64gui.exe"
#else
#define LAUNCH_FILE "sggui.exe"
#endif
	if ((INT_PTR)ShellExecute(hWnd, TEXT("open"), TEXT(LAUNCH_FILE), NULL, NULL, SW_SHOWNORMAL) <= 32)
	{
		char buf[256] = "";
		lstrcpyA(buf, LAUNCH_FILE "\n");
		int len = lstrlenA(buf);
		LoadStringA(NULL, IDS_STRING_LAUNCH_FAIL, buf + len, ARRAYSIZE(buf) - len * sizeof(*buf));
		MessageBoxA(hWnd, buf, NULL, MB_ICONERROR);
		return FALSE;
	}
	SendMessage(hWnd, WM_COMMAND, IDCANCEL, 0);
	return TRUE;
}

int GetProcessNameW(DWORD pid, LPWSTR buf, int c)
{
	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	int r = GetProcessImageFileName(h, buf, c);
	lstrcpy(buf, wcsrchr(buf, '\\') + 1);
	CloseHandle(h);
	return r;
}

BOOL OnSGMessageUpdateList(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hList = GetDlgItem(hWnd, IDC_LIST_PROCESS);
	DWORD pid = lParam;
	TCHAR buf[MAX_PATH] = TEXT("");
	TCHAR* szApi[] = { TEXT("NULL"),TEXT("D3D8"),TEXT("D3D9"),TEXT("D3D11"),TEXT("GL"),TEXT("D3D10"),TEXT("DDraw"),TEXT("Vulkan"),TEXT("D3D12") };
	int iApi = HIWORD(wParam) % ARRAYSIZE(szApi);
	LVITEM lvi{};
	int rc = ListView_GetItemCount(hList);
	switch (LOWORD(wParam))
	{
	case SG_UPDATE_LIST_REFRESH:
		/*TODO*/
		MessageBox(NULL, TEXT("TODO"), NULL, NULL);
		break;
	case SG_UPDATE_LIST_ADD:
		lvi.mask = LVIF_IMAGE;
		lvi.iItem = rc;
		lvi.iSubItem = 0;
		ListView_InsertItem(hList, &lvi);
		wsprintf(buf, TEXT("%d"), pid);
		ListView_SetItemText(hList, rc, 1, buf);
		GetProcessNameW(pid, buf, ARRAYSIZE(buf));
		ListView_SetItemText(hList, rc, 2, buf);
		ListView_SetItemText(hList, rc, 3, szApi[iApi]);
		break;
	case SG_UPDATE_LIST_REMOVE:
		for (int i = 0; i < rc; i++)
		{
			ListView_GetItemText(hList, i, 1, buf, ARRAYSIZE(buf));
			if (pid == _ttoi(buf))
			{
				ListView_GetItemText(hList, i, 3, buf, ARRAYSIZE(buf));
				if (lstrcmp(szApi[iApi], buf) == 0)
				{
					ListView_DeleteItem(hList, i);
					break;
				}
			}
		}
		break;
	}
	return TRUE;
}
