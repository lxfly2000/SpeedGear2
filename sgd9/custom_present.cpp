#include"custom_present.h"
#include <d3dx9.h>
#include<map>
#include<string>
#include<ctime>

#pragma comment(lib,"d3dx9.lib")

#ifdef _DEBUG
#define C(x) if(FAILED(x)){MessageBox(NULL,TEXT(_CRT_STRINGIZE(x)),NULL,MB_ICONERROR);throw E_FAIL;}
#else
#define C(x) x
#endif

#include<sstream>
#include<thread>

#define SPEEDGEAR_HOTKEY_STICK_LSHIFT 1
#define SPEEDGEAR_HOTKEY_STICK_RSHIFT 2
#define SPEEDGEAR_HOTKEY_STICK_LCTRL 4
#define SPEEDGEAR_HOTKEY_STICK_RCTRL 8
#define SPEEDGEAR_HOTKEY_STICK_LALT 16
#define SPEEDGEAR_HOTKEY_STICK_RALT 32

#define SPEEDGEAR_MIN_SPEED 0.125f
#define SPEEDGEAR_MAX_SPEED 8.0f

#define SPEEDGEAR_BEEP_SPEED_UP 1
#define SPEEDGEAR_BEEP_SLOW_DOWN 2
#define SPEEDGEAR_BEEP_ORIGINAL 0
#define SPEEDGEAR_BEEP_ERROR 3
#define SPEEDGEAR_BEEP_PREC_UP 4
#define SPEEDGEAR_BEEP_PREC_DOWN 5

namespace SpeedGear
{

	float current_speed = 1.0f;
	float fPreciseAdjustment = 0.25f;

	DWORD vkSpeedUp, vkSpeedDown, vkPreciseSpeedUp, vkPreciseSpeedDown, vkSpeedReset;
	int globalApply;

	float GetCurrentSpeed()
	{
		return current_speed;
	}


	BOOL IsMyAppFocused()
	{
		//获取我的进程PID
		DWORD pid;
		GetWindowThreadProcessId(GetActiveWindow(), &pid);
		return pid == GetCurrentProcessId();
	}

	void ParseKey(LPCTSTR str, DWORD* vk, DWORD* stick)
	{
		std::wistringstream iss(str);
		iss >> *vk;
		*stick = 0;
		while (!iss.eof())
		{
			std::wstring inb;
			iss >> inb;
			if (_wcsicmp(inb.c_str(), L"lshift") == 0)
				*stick |= SPEEDGEAR_HOTKEY_STICK_LSHIFT;
			if (_wcsicmp(inb.c_str(), L"rshift") == 0)
				*stick |= SPEEDGEAR_HOTKEY_STICK_RSHIFT;
			if (_wcsicmp(inb.c_str(), L"lctrl") == 0)
				*stick |= SPEEDGEAR_HOTKEY_STICK_LCTRL;
			if (_wcsicmp(inb.c_str(), L"rctrl") == 0)
				*stick |= SPEEDGEAR_HOTKEY_STICK_RCTRL;
			if (_wcsicmp(inb.c_str(), L"lalt") == 0)
				*stick |= SPEEDGEAR_HOTKEY_STICK_LALT;
			if (_wcsicmp(inb.c_str(), L"ralt") == 0)
				*stick |= SPEEDGEAR_HOTKEY_STICK_RALT;
		}
	}

	class KeyManager
	{
	private:
		std::map<DWORD, DWORD>hotkeys;
	public:
		void AddHotkey(DWORD key, DWORD sticks)
		{
			hotkeys.insert(std::make_pair(key, sticks));
		}
		//返回相应快捷键，如果无则返回0
		DWORD IsHotkeyHit(DWORD key)
		{
			if (hotkeys.find(key) == hotkeys.end())
				return 0;
			DWORD sticks = hotkeys[key];
			if ((sticks & SPEEDGEAR_HOTKEY_STICK_LSHIFT) && (GetAsyncKeyState(VK_LSHIFT) & 0x8000) == 0)
				return 0;
			if ((sticks & SPEEDGEAR_HOTKEY_STICK_RSHIFT) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000) == 0)
				return 0;
			if ((sticks & SPEEDGEAR_HOTKEY_STICK_LCTRL) && (GetAsyncKeyState(VK_LCONTROL) & 0x8000) == 0)
				return 0;
			if ((sticks & SPEEDGEAR_HOTKEY_STICK_RCTRL) && (GetAsyncKeyState(VK_RCONTROL) & 0x8000) == 0)
				return 0;
			if ((sticks & SPEEDGEAR_HOTKEY_STICK_LALT) && (GetAsyncKeyState(VK_LMENU) & 0x8000) == 0)
				return 0;
			if ((sticks & SPEEDGEAR_HOTKEY_STICK_RALT) && (GetAsyncKeyState(VK_RMENU) & 0x8000) == 0)
				return 0;
			return key;
		}
		void Init()
		{
			hotkeys.clear();
		}
	};
	static KeyManager km;
	void SpeedGearBeep(int type)
	{
		if (!IsMyAppFocused())
			return;
		std::thread([](int type)
			{
				switch (type)
				{
				case SPEEDGEAR_BEEP_ORIGINAL:Beep(1000, 100); break;
				case SPEEDGEAR_BEEP_SPEED_UP:Beep(2000, 100); break;
				case SPEEDGEAR_BEEP_SLOW_DOWN:Beep(500, 100); break;
				case SPEEDGEAR_BEEP_ERROR:default:Beep(750, 500); break;
				case SPEEDGEAR_BEEP_PREC_UP:Beep(DWORD(1000 * (1 + fPreciseAdjustment)), 100); break;
				case SPEEDGEAR_BEEP_PREC_DOWN:Beep(DWORD(1000 * (1 - fPreciseAdjustment)), 100); break;
				}
				if (type != SPEEDGEAR_BEEP_ERROR && type != SPEEDGEAR_BEEP_ORIGINAL)
					Beep(DWORD(1000 * current_speed), 100);
			}, type).detach();
	}

	void OnKeydown(DWORD vkCode)
	{
		DWORD hk = km.IsHotkeyHit(vkCode);
		if (hk && (globalApply || IsMyAppFocused()))
		{
			if (hk == vkSpeedUp)
			{
				if (current_speed * 2.0f <= SPEEDGEAR_MAX_SPEED)
				{
					current_speed *= 2.0f;
					SpeedGearBeep(SPEEDGEAR_BEEP_SPEED_UP);
				}
				else
				{
					SpeedGearBeep(SPEEDGEAR_BEEP_ERROR);
				}
			}
			else if (hk == vkSpeedDown)
			{
				if (current_speed / 2.0f >= SPEEDGEAR_MIN_SPEED)
				{
					current_speed /= 2.0f;
					SpeedGearBeep(SPEEDGEAR_BEEP_SLOW_DOWN);
				}
				else
				{
					SpeedGearBeep(SPEEDGEAR_BEEP_ERROR);
				}
			}
			else if (hk == vkSpeedReset)
			{
				current_speed = 1.0f;
				SpeedGearBeep(SPEEDGEAR_BEEP_ORIGINAL);
			}
			else if (hk == vkPreciseSpeedUp)
			{
				if (current_speed + fPreciseAdjustment <= SPEEDGEAR_MAX_SPEED)
				{
					current_speed += fPreciseAdjustment;
					SpeedGearBeep(SPEEDGEAR_BEEP_PREC_UP);
				}
				else
				{
					SpeedGearBeep(SPEEDGEAR_BEEP_ERROR);
				}
			}
			else if (hk == vkPreciseSpeedDown)
			{
				if (current_speed - fPreciseAdjustment >= SPEEDGEAR_MIN_SPEED)
				{
					current_speed -= fPreciseAdjustment;
					SpeedGearBeep(SPEEDGEAR_BEEP_PREC_DOWN);
				}
				else
				{
					SpeedGearBeep(SPEEDGEAR_BEEP_ERROR);
				}
			}
		}
	}


	LRESULT CALLBACK ProcessHook(int c, WPARAM w, LPARAM l)
	{
		if (c == HC_ACTION)
		{
			switch (w)
			{
			case WM_KEYUP:case WM_SYSKEYUP:
				PKBDLLHOOKSTRUCT pk = (PKBDLLHOOKSTRUCT)l;
				if ((pk->vkCode == VK_RETURN) && (pk->flags & LLKHF_EXTENDED))
					pk->vkCode = VK_SEPARATOR;
				OnKeydown(pk->vkCode);
				break;
			}
		}
		return CallNextHookEx(NULL, c, w, l);
	}

	static HHOOK hhook = nullptr;

	BOOL InitCustomTime()
	{
		TCHAR szConfPath[MAX_PATH];
		GetDLLPath(szConfPath, ARRAYSIZE(szConfPath));
		lstrcpy(wcsrchr(szConfPath, '.'), TEXT(".ini"));
#define GetInitConfStr(key,def) GetPrivateProfileString(TEXT("SpeedGear"),TEXT(_STRINGIZE(key)),def,key,ARRAYSIZE(key),szConfPath)
#define GetInitConfInt(key,def) key=GetPrivateProfileInt(TEXT("SpeedGear"),TEXT(_STRINGIZE(key)),def,szConfPath)
#define F(_i_str) (float)_wtof(_i_str)
		TCHAR keySpeedUp[50], keySpeedDown[50], keyPreciseSpeedUp[50], keyPreciseSpeedDown[50], keySpeedReset[50], preciseAdjustment[20];
		GetInitConfStr(keySpeedUp, TEXT("187"));
		GetInitConfStr(keySpeedDown, TEXT("189"));
		GetInitConfStr(keySpeedReset, TEXT("48"));
		GetInitConfStr(keyPreciseSpeedUp, TEXT("221"));
		GetInitConfStr(keyPreciseSpeedDown, TEXT("219"));
		GetInitConfStr(preciseAdjustment, TEXT("0.25"));
		GetInitConfInt(globalApply, 0);
		km.Init();
		DWORD vStick;
		ParseKey(keySpeedUp, &vkSpeedUp, &vStick);
		km.AddHotkey(vkSpeedUp, vStick);
		ParseKey(keySpeedDown, &vkSpeedDown, &vStick);
		km.AddHotkey(vkSpeedDown, vStick);
		ParseKey(keySpeedReset, &vkSpeedReset, &vStick);
		km.AddHotkey(vkSpeedReset, vStick);
		ParseKey(keyPreciseSpeedUp, &vkPreciseSpeedUp, &vStick);
		km.AddHotkey(vkPreciseSpeedUp, vStick);
		ParseKey(keyPreciseSpeedDown, &vkPreciseSpeedDown, &vStick);
		km.AddHotkey(vkPreciseSpeedDown, vStick);
		fPreciseAdjustment = F(preciseAdjustment);

		hhook = SetWindowsHookEx(WH_KEYBOARD_LL, ProcessHook, GetModuleHandle(NULL), 0);
		return hhook != nullptr;
	}

	BOOL UninitCustomTime()
	{
		return UnhookWindowsHookEx(hhook);
	}


}

ID3DXFont* pFont = nullptr;//经测试D3DXFont在此处不能创建多次
class D3DXCustomPresent
{
private:
	unsigned t1, t2, fcount;
	std::wstring display_text;
	int current_fps;
	TCHAR time_text[32], fps_text[32], width_text[32], height_text[32], speed_text[32];

	TCHAR font_name[256], font_size[16], text_x[16], text_y[16], text_align[16], text_valign[16], display_text_fmt[256], fps_fmt[32], time_fmt[32], width_fmt[32], height_fmt[32], speed_fmt[32];
	TCHAR font_red[16], font_green[16], font_blue[16], font_alpha[16];
	TCHAR font_shadow_red[16], font_shadow_green[16], font_shadow_blue[16], font_shadow_alpha[16], font_shadow_distance[16];
	UINT font_weight, period_frames;
	RECT rText, rTextShadow;
	int formatFlag;
	D3DCOLOR color_text, color_shadow;
	D3DVIEWPORT9 viewport;
public:
	D3DXCustomPresent() :t1(0), t2(0), fcount(0), formatFlag(0)
	{
	}
	D3DXCustomPresent(D3DXCustomPresent&& other)
	{
		t1 = std::move(other.t1);
		t2 = std::move(other.t2);
		fcount = std::move(other.fcount);
		formatFlag = std::move(other.formatFlag);
	}
	~D3DXCustomPresent()
	{
		Uninit();
	}
	BOOL Init(LPDIRECT3DDEVICE9 pDev)
	{
		Uninit();
		TCHAR szConfPath[MAX_PATH];
		GetDLLPath(szConfPath, ARRAYSIZE(szConfPath));
		lstrcpy(wcsrchr(szConfPath, '.'), TEXT(".ini"));
#define GetInitConfStr(key,def) GetPrivateProfileString(TEXT("Init"),TEXT(_STRINGIZE(key)),def,key,ARRAYSIZE(key),szConfPath)
#define GetInitConfInt(key,def) key=GetPrivateProfileInt(TEXT("Init"),TEXT(_STRINGIZE(key)),def,szConfPath)
#define F(_i_str) (float)_wtof(_i_str)
		GetInitConfStr(font_name, TEXT("宋体"));
		GetInitConfStr(font_size, TEXT("48"));
		GetInitConfStr(font_red, TEXT("1"));
		GetInitConfStr(font_green, TEXT("1"));
		GetInitConfStr(font_blue, TEXT("0"));
		GetInitConfStr(font_alpha, TEXT("1"));
		GetInitConfStr(font_shadow_red, TEXT("0.5"));
		GetInitConfStr(font_shadow_green, TEXT("0.5"));
		GetInitConfStr(font_shadow_blue, TEXT("0"));
		GetInitConfStr(font_shadow_alpha, TEXT("1"));
		GetInitConfStr(font_shadow_distance, TEXT("2"));
		GetInitConfInt(font_weight, 400);
		GetInitConfStr(text_x, TEXT("0"));
		GetInitConfStr(text_y, TEXT("0"));
		GetInitConfStr(text_align, TEXT("left"));
		GetInitConfStr(text_valign, TEXT("top"));
		GetInitConfInt(period_frames, 60);
		GetInitConfStr(time_fmt, TEXT("%H:%M:%S"));
		GetInitConfStr(fps_fmt, TEXT("FPS:%3d"));
		GetInitConfStr(width_fmt, TEXT("%d"));
		GetInitConfStr(height_fmt, TEXT("%d"));
		GetInitConfStr(speed_fmt, TEXT("%.2f"));
		GetInitConfStr(display_text_fmt, TEXT("{fps}"));

		D3DXFONT_DESC df;
		ZeroMemory(&df, sizeof(D3DXFONT_DESC));
		df.Height = -MulDiv(_wtoi(font_size), USER_DEFAULT_SCREEN_DPI, 72);//此处不能直接用字体大小，需要将字体大小换算成GDI的逻辑单元大小
		df.Width = 0;
		df.Weight = font_weight;
		df.MipLevels = D3DX_DEFAULT;
		df.Italic = false;
		df.CharSet = DEFAULT_CHARSET;
		df.OutputPrecision = 0;
		df.Quality = 0;
		df.PitchAndFamily = 0;
		lstrcpy(df.FaceName, font_name);
		if (!pFont)
			C(D3DXCreateFontIndirect(pDev, &df, &pFont));
		C(pDev->GetViewport(&viewport));
		if (lstrcmpi(text_align, TEXT("right")) == 0)
		{
			formatFlag |= DT_RIGHT;
			rText.left = 0;
			rText.right = (LONG)(F(text_x) * viewport.Width);
		}
		else if (lstrcmpi(text_align, TEXT("center")) == 0)
		{
			formatFlag |= DT_CENTER;
			if (F(text_x) > 0.5f)
			{
				rText.left = 0;
				rText.right = (LONG)(2.0f * viewport.Width * F(text_x));
			}
			else
			{
				rText.left = (LONG)(2.0f * viewport.Width * F(text_x) - viewport.Width);
				rText.right = (LONG)viewport.Width;
			}
		}
		else
		{
			formatFlag |= DT_LEFT;
			rText.left = (LONG)(F(text_x) * viewport.Width);
			rText.right = (LONG)viewport.Width;
		}
		if (lstrcmpi(text_valign, TEXT("bottom")) == 0)
		{
			formatFlag |= DT_BOTTOM;
			rText.top = 0;
			rText.bottom = (LONG)(F(text_y) * viewport.Height);
		}
		else if (lstrcmpi(text_valign, TEXT("center")) == 0)
		{
			formatFlag |= DT_VCENTER;
			if (F(text_y) > 0.5f)
			{
				rText.top = 0;
				rText.bottom = (LONG)(2.0f * viewport.Height * F(text_y));
			}
			else
			{
				rText.top = (LONG)(2.0f * viewport.Height * F(text_y) - viewport.Height);
				rText.bottom = (LONG)viewport.Height;
			}
		}
		else
		{
			formatFlag |= DT_TOP;
			rText.top = (LONG)(F(text_y) * viewport.Height);
			rText.bottom = (LONG)viewport.Height;
		}
		rText.left += (LONG)viewport.X;
		rText.top += (LONG)viewport.Y;
		rText.right += (LONG)viewport.X;
		rText.bottom += (LONG)viewport.Y;
		rTextShadow.left = rText.left + (LONG)F(font_shadow_distance);
		rTextShadow.top = rText.top + (LONG)F(font_shadow_distance);
		rTextShadow.right = rText.right + (LONG)F(font_shadow_distance);
		rTextShadow.bottom = rText.bottom + (LONG)F(font_shadow_distance);
		color_shadow = D3DCOLOR_ARGB((DWORD)(255.0f * F(font_shadow_alpha)),
			(DWORD)(255.0f * F(font_shadow_red)),
			(DWORD)(255.0f * F(font_shadow_green)),
			(DWORD)(255.0f * F(font_shadow_blue)));
		color_text = D3DCOLOR_ARGB((DWORD)(255.0f * F(font_alpha)),
			(DWORD)(255.0f * F(font_red)),
			(DWORD)(255.0f * F(font_green)),
			(DWORD)(255.0f * F(font_blue)));

		D3DDEVICE_CREATION_PARAMETERS dcp;
		C(pDev->GetCreationParameters(&dcp));
		return TRUE;
	}
	void Uninit()
	{
		if (pFont)
		{
			LPDIRECT3DDEVICE9 pDev;
			C(pFont->GetDevice(&pDev));
			D3DDEVICE_CREATION_PARAMETERS dcp;
			C(pDev->GetCreationParameters(&dcp));
			pFont->Release();
			pFont = nullptr;
		}
	}
	void Draw()
	{
		if (fcount-- == 0)
		{
			fcount = period_frames;
			t1 = t2;
			t2 = GetTickCount();
			if (t1 == t2)
				t1--;
			current_fps = period_frames * 1000 / (t2 - t1);
			wsprintf(fps_text, fps_fmt, current_fps);//注意wsprintf不支持浮点数格式化
			time_t t1 = time(NULL);
			tm tm1;
			localtime_s(&tm1, &t1);
			wcsftime(time_text, ARRAYSIZE(time_text), time_fmt, &tm1);
			wsprintf(width_text, width_fmt, viewport.Width);
			wsprintf(height_text, height_fmt, viewport.Height);
			swprintf_s(speed_text, speed_fmt, SpeedGear::GetCurrentSpeed());
			display_text = display_text_fmt;
			size_t pos = display_text.find(TEXT("\\n"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 2, TEXT("\n"));
			pos = display_text.find(TEXT("{fps}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 5, fps_text);
			pos = display_text.find(TEXT("{time}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 6, time_text);
			pos = display_text.find(TEXT("{width}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 7, width_text);
			pos = display_text.find(TEXT("{height}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 8, height_text);
			pos = display_text.find(TEXT("{speed}"));
			if (pos != std::wstring::npos)
				display_text.replace(pos, 7, speed_text);
		}
		pFont->DrawText(NULL, display_text.c_str(), (int)display_text.length(), &rTextShadow, formatFlag, color_shadow);
		pFont->DrawText(NULL, display_text.c_str(), (int)display_text.length(), &rText, formatFlag, color_text);
	}
};

static std::map<LPDIRECT3DDEVICE9, D3DXCustomPresent> cp;

void CustomPresent(LPDIRECT3DDEVICE9 p, HRESULT hrLast)
{
	if (cp.find(p) == cp.end())
	{
		if (hrLast == D3D_OK && p->TestCooperativeLevel() == D3D_OK)
		{
			cp.insert(std::make_pair(p, D3DXCustomPresent()));
			cp[p].Init(p);
		}
	}
	else if (hrLast != D3D_OK || p->TestCooperativeLevel() != D3D_OK)
		cp.erase(p);
	else
		cp[p].Draw();
}
