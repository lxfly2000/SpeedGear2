#include"custom_blt.h"
#include<ddraw.h>
#include"..\sgshared\sgshared.h"
#include<map>
#include<string>
#include<ctime>

#ifdef _DEBUG
#define C(x) if(FAILED(x)){MessageBox(NULL,TEXT(_CRT_STRINGIZE(x)),NULL,MB_ICONERROR);throw E_FAIL;}
#else
#define C(x) x
#endif

HFONT hFont = NULL;
class GCustomBlt
{
private:
	ULONGLONG t1, t2;
	char display_text[256];
	int current_fps;
	int shad;

	UINT period_frames, fcount;
	RECT rText, rTextShadow;
	int formatFlag;
	COLORREF color_text, color_shadow;
	LPDIRECTDRAWSURFACE surfaceBack;
public:
	GCustomBlt() :t1(0), t2(0), fcount(0), formatFlag(0), color_shadow(0), color_text(0), current_fps(0), display_text(), period_frames(0),
		rTextShadow(), rText(), shad(0)
	{
	}
	GCustomBlt(GCustomBlt &&other)noexcept :GCustomBlt()
	{
		t1 = std::move(other.t1);
		t2 = std::move(other.t2);
		fcount = std::move(other.fcount);
		formatFlag = std::move(other.formatFlag);
	}
	~GCustomBlt()
	{
		Uninit();
	}
	BOOL Init(LPDIRECTDRAWSURFACE sFront, LPDIRECTDRAWSURFACE sBack, LPRECT rcSrc)
	{
		Uninit();

		SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
		if (pMem == NULL)
		{
			if (!SpeedGear_InitializeSharedMemory(FALSE))
				return FALSE;
			pMem = SpeedGear_GetSharedMemory();
		}
		surfaceBack = sBack;
		DDSURFACEDESC desc;
		desc.dwSize = sizeof(desc);
		C(sBack->GetSurfaceDesc(&desc));
		RECT rcRect = { 0 };
		if (!rcSrc)
		{
			rcSrc = &rcRect;
			rcRect.right = desc.dwWidth;
			rcRect.bottom = desc.dwHeight;
		}
		LOGFONT df;
		ZeroMemory(&df, sizeof(LOGFONT));
		df.lfHeight = -(pMem->useSystemDPI ? LOGICAL_UNIT_TO_PIXEL(GetDesktopWindow(), pMem->fontSize) : LOGICAL_UNIT_TO_PIXEL_96DPI(pMem->fontSize));
		df.lfWidth = 0;
		df.lfWeight = pMem->fontWeight;
		df.lfItalic = false;
		df.lfCharSet = DEFAULT_CHARSET;
		df.lfOutPrecision = 0;
		df.lfQuality = 0;
		df.lfPitchAndFamily = 0;
		swprintf_s(df.lfFaceName, TEXT("%S"), pMem->fontName);
		if (!hFont)
			hFont = CreateFontIndirect(&df);
		rText.left = 0;
		rText.right = (LONG)(rcSrc->right-rcSrc->left);
		rText.top = 0;
		rText.bottom = (LONG)(rcSrc->bottom-rcSrc->top);
		if (pMem->statusPosition % 3 == 2)
			formatFlag |= DT_RIGHT;
		else if (pMem->statusPosition % 3 == 1)
			formatFlag |= DT_CENTER;
		else
			formatFlag |= DT_LEFT;
		if (pMem->statusPosition / 3 == 2)
			formatFlag |= (DT_BOTTOM | DT_SINGLELINE);
		else if (pMem->statusPosition / 3 == 1)
			formatFlag |= (DT_VCENTER | DT_SINGLELINE);
		else
			formatFlag |= DT_TOP;
		rText.left += (LONG)rcSrc->left;
		rText.top += (LONG)rcSrc->top;
		rText.right += (LONG)rcSrc->left;
		rText.bottom += (LONG)rcSrc->top;
		shad = pMem->useSystemDPI ? DPI_SCALED_VALUE(GetDesktopWindow(), 2) : 2;
		rTextShadow.left = rText.left + (LONG)shad;
		rTextShadow.top = rText.top + (LONG)shad;
		rTextShadow.right = rText.right + (LONG)shad;
		rTextShadow.bottom = rText.bottom + (LONG)shad;
		color_shadow = RGB((pMem->fontColor & 0xFF) / 2, ((pMem->fontColor >> 8) & 0xFF) / 2, ((pMem->fontColor >> 16) & 0xFF) / 2);
		color_text = RGB(pMem->fontColor & 0xFF, (pMem->fontColor >> 8) & 0xFF, (pMem->fontColor >> 16) & 0xFF);
		period_frames = desc.dwRefreshRate;

		return TRUE;
	}
	void Uninit()
	{
		if (hFont)
		{
			DeleteObject(hFont);
			hFont = NULL;
		}
	}
	void Draw()
	{
		if (fcount-- == 0)
		{
			fcount = period_frames;
			t1 = t2;
			t2 = GetTickCount64();
			if (t1 == t2)
				t1--;
			current_fps = period_frames * 1000 / (UINT)(t2 - t1);
			time_t t1 = time(NULL);
			tm tm1;
			localtime_s(&tm1, &t1);
			SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
			SpeedGear_FormatText(display_text, ARRAYSIZE(display_text), pMem->statusFormat, pMem->hookSpeed, current_fps,
				rText.right, rText.bottom, tm1.tm_hour, tm1.tm_min, tm1.tm_sec, "DDraw");
		}
		HDC dc;
		surfaceBack->GetDC(&dc);
		SetBkMode(dc, TRANSPARENT);
		SelectObject(dc, hFont);
		SetTextColor(dc, color_shadow);
		DrawTextA(dc, display_text, lstrlenA(display_text), &rTextShadow, formatFlag);
		SetTextColor(dc, color_text);
		DrawTextA(dc, display_text, lstrlenA(display_text), &rText, formatFlag);
		surfaceBack->ReleaseDC(dc);
	}
};

static std::map<LPDIRECTDRAWSURFACE, GCustomBlt> cp;

void CustomBlt(LPDIRECTDRAWSURFACE p, LPRECT rcTo, LPDIRECTDRAWSURFACE sSource, LPRECT rcSrc, DWORD flag, LPDDBLTFX fx)
{
	if (cp.find(p) == cp.end())
	{
		cp.clear();
		cp.insert(std::make_pair(p, GCustomBlt()));
		cp[p].Init(p, sSource, rcSrc);
	}
	cp[p].Draw();
}

void CustomBltFast(LPDIRECTDRAWSURFACE p, DWORD toX, DWORD toY, LPDIRECTDRAWSURFACE sSource, LPRECT rcSrc, DWORD flag)
{
	if (cp.find(p) == cp.end())
	{
		cp.clear();
		cp.insert(std::make_pair(p, GCustomBlt()));
		cp[p].Init(p,sSource, rcSrc);
	}
	cp[p].Draw();
}

void CustomFlip(LPDIRECTDRAWSURFACE p, LPDIRECTDRAWSURFACE sSource, DWORD flag)
{
	if (cp.find(p) == cp.end())
	{
		cp.clear();
		cp.insert(std::make_pair(p, GCustomBlt()));
		DDSCAPS ddc = { DDSCAPS_BACKBUFFER };
		p->GetAttachedSurface(&ddc, &sSource);
		cp[p].Init(p, sSource, NULL);
	}
	cp[p].Draw();
}
