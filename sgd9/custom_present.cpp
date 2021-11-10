#include"custom_present.h"
#include"..\sgshared\sgshared.h"
#include<d3dx9.h>
#include<map>
#include<string>
#include<ctime>

#pragma comment(lib,"d3dx9.lib")

#ifdef _DEBUG
#define C(x) if(FAILED(x)){MessageBox(NULL,TEXT(_CRT_STRINGIZE(x)),NULL,MB_ICONERROR);throw E_FAIL;}
#else
#define C(x) x
#endif

ID3DXFont* pFont = nullptr;//经测试D3DXFont在此处不能创建多次
class D3DXCustomPresent
{
private:
	unsigned t1, t2, fcount;
	char display_text[256];
	int current_fps;
	int shad;

	UINT period_frames;
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
		SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
		if (pMem == NULL)
		{
			if (!SpeedGear_InitializeSharedMemory(FALSE))
				return FALSE;
			pMem = SpeedGear_GetSharedMemory();
		}

		D3DDEVICE_CREATION_PARAMETERS dcp;
		C(pDev->GetCreationParameters(&dcp));
		D3DXFONT_DESC df;
		ZeroMemory(&df, sizeof(D3DXFONT_DESC));
		df.Height = pMem->useSystemDPI ? POUND_TO_FONTHEIGHT(dcp.hFocusWindow, pMem->fontSize) : FONTHEIGHT_TO_POUND_96DPI(pMem->fontSize);
		df.Width = 0;
		df.Weight = pMem->fontWeight;
		df.MipLevels = D3DX_DEFAULT;
		df.Italic = false;
		df.CharSet = DEFAULT_CHARSET;
		df.OutputPrecision = 0;
		df.Quality = 0;
		df.PitchAndFamily = 0;
		swprintf_s(df.FaceName, TEXT("%S"), pMem->fontName);
		if (!pFont)
			C(D3DXCreateFontIndirect(pDev, &df, &pFont));
		C(pDev->GetViewport(&viewport));
		rText.left = 0;
		rText.right = (LONG)viewport.Width;
		rText.top = 0;
		rText.bottom = (LONG)viewport.Height;
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
		rText.left += (LONG)viewport.X;
		rText.top += (LONG)viewport.Y;
		rText.right += (LONG)viewport.X;
		rText.bottom += (LONG)viewport.Y;
		shad = pMem->useSystemDPI ? DPI_SCALED_VALUE(dcp.hFocusWindow, 2) : 2;
		rTextShadow.left = rText.left + (LONG)shad;
		rTextShadow.top = rText.top + (LONG)shad;
		rTextShadow.right = rText.right + (LONG)shad;
		rTextShadow.bottom = rText.bottom + (LONG)shad;
		color_shadow = D3DCOLOR_ARGB(128, (pMem->fontColor & 0xFF) / 2, ((pMem->fontColor >> 8) & 0xFF) / 2, ((pMem->fontColor >> 16) & 0xFF) / 2);
		color_text = D3DCOLOR_ARGB(255, pMem->fontColor & 0xFF, (pMem->fontColor >> 8) & 0xFF, (pMem->fontColor >> 16) & 0xFF);
		D3DDISPLAYMODE dm;
		pDev->GetDisplayMode(0, &dm);
		period_frames = dm.RefreshRate;
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
			time_t t1 = time(NULL);
			tm tm1;
			localtime_s(&tm1, &t1);
			SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
			SpeedGear_FormatText(display_text, ARRAYSIZE(display_text), pMem->statusFormat, pMem->hookSpeed, current_fps,
				viewport.Width, viewport.Height, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
		}
		pFont->DrawTextA(NULL, display_text, lstrlenA(display_text), &rTextShadow, formatFlag, color_shadow);
		pFont->DrawTextA(NULL, display_text, lstrlenA(display_text), &rText, formatFlag, color_text);
	}
};

static std::map<LPDIRECT3DDEVICE9, D3DXCustomPresent> cp;

void CustomPresent(LPDIRECT3DDEVICE9 p)
{
	if (cp.find(p) == cp.end())
	{
		cp.insert(std::make_pair(p, D3DXCustomPresent()));
		cp[p].Init(p);
	}
	cp[p].Draw();
}

void CustomPresentEx(LPDIRECT3DDEVICE9EX p)
{
	//TODO
}

void CustomReset(LPDIRECT3DDEVICE9 p, D3DPRESENT_PARAMETERS* m)
{
	//TODO
}

void CustomResetEx(LPDIRECT3DDEVICE9EX p, D3DPRESENT_PARAMETERS* m, D3DDISPLAYMODEEX* e)
{
	//TODO
}

void CustomSCPresent(LPDIRECT3DSWAPCHAIN9 p)
{
	//TODO
}
