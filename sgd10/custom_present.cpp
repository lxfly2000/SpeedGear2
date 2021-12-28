#include"custom_present.h"
#include"..\sgshared\sgshared.h"
#include<D3DX10.h>
#include<map>
#include<string>
#include<ctime>

#pragma comment(lib,"d3dx10.lib")

#ifdef _DEBUG
#define C(x) if(FAILED(x)){MessageBox(NULL,TEXT(_CRT_STRINGIZE(x)),NULL,MB_ICONERROR);throw E_FAIL;}
#else
#define C(x) x
#endif


ID3DX10Font* pFont = nullptr;//经测试D3DXFont在此处不能创建多次
class D3DXCustomPresent
{
private:
	ULONGLONG t1, t2;
	char display_text[256];
	int current_fps;
	int shad;

	UINT period_frames, fcount;
	RECT rText, rTextShadow;
	int formatFlag;
	D3DXCOLOR color_text, color_shadow;
	DXGI_SWAP_CHAIN_DESC sc_desc;
public:
	D3DXCustomPresent() :t1(0), t2(0), fcount(0), formatFlag(0), color_shadow(), color_text(), current_fps(0), display_text(), period_frames(0),
		rTextShadow(), rText(), shad(0), sc_desc()
	{
	}
	D3DXCustomPresent(D3DXCustomPresent&& other)noexcept :D3DXCustomPresent()
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
	BOOL Init(IDXGISwapChain* pSC)
	{
		Uninit();
		SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
		if (pMem == NULL)
		{
			if (!SpeedGear_InitializeSharedMemory(FALSE))
				return FALSE;
			pMem = SpeedGear_GetSharedMemory();
		}

		C(pSC->GetDesc(&sc_desc));
		D3DX10_FONT_DESC df;
		ZeroMemory(&df, sizeof(D3DX10_FONT_DESC));
		df.Height = -(pMem->useSystemDPI ? LOGICAL_UNIT_TO_PIXEL(sc_desc.OutputWindow, pMem->fontSize) : LOGICAL_UNIT_TO_PIXEL_96DPI(pMem->fontSize));
		df.Width = 0;
		df.Weight = pMem->fontWeight;
		df.MipLevels = D3DX10_DEFAULT;
		df.Italic = false;
		df.CharSet = DEFAULT_CHARSET;
		df.OutputPrecision = 0;
		df.Quality = 0;
		df.PitchAndFamily = 0;
		swprintf_s(df.FaceName, TEXT("%S"), pMem->fontName);

		ID3D10Device* pDev;
		C(pSC->GetDevice(__uuidof(ID3D10Device), (void**)&pDev));//这个不需要在释放时调用Release
		if (!pFont)
			C(D3DX10CreateFontIndirect(pDev, &df, &pFont));
		rText.left = 0;
		rText.right = (LONG)sc_desc.BufferDesc.Width;
		rText.top = 0;
		rText.bottom = (LONG)sc_desc.BufferDesc.Height;
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
		shad = pMem->useSystemDPI ? DPI_SCALED_VALUE(sc_desc.OutputWindow, 2) : 2;
		rTextShadow.left = rText.left + (LONG)shad;
		rTextShadow.top = rText.top + (LONG)shad;
		rTextShadow.right = rText.right + (LONG)shad;
		rTextShadow.bottom = rText.bottom + (LONG)shad;
		color_shadow = D3DXCOLOR((pMem->fontColor & 0xFF) / 512.0f, ((pMem->fontColor >> 8) & 0xFF) / 512.0f, ((pMem->fontColor >> 16) & 0xFF) / 512.0f, 0.5f);
		color_text = D3DXCOLOR((pMem->fontColor & 0xFF) / 256.0f, ((pMem->fontColor >> 8) & 0xFF) / 256.0f, ((pMem->fontColor >> 16) & 0xFF) / 256.0f, 1.0f);
		//注意此处根据微软官方文档的说明，此处的分母是可以指定为0的！！
		period_frames = sc_desc.BufferDesc.RefreshRate.Numerator / max(1, sc_desc.BufferDesc.RefreshRate.Denominator);
		if (period_frames <= 1)
		{
			DEVMODE dm{};
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
			period_frames = dm.dmDisplayFrequency;
		}
		return TRUE;
	}
	void CalcRect(IDXGISwapChain* pSC, UINT BufferCount, UINT width, UINT height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		sc_desc.BufferCount = BufferCount;
		sc_desc.BufferDesc.Width = width;
		sc_desc.BufferDesc.Height = height;
		sc_desc.BufferDesc.Format = NewFormat;
		sc_desc.Flags = SwapChainFlags;
		float fWidth = (float)sc_desc.BufferDesc.Width, fHeight = (float)sc_desc.BufferDesc.Height;
		SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
		rText.right = width;
		rText.bottom = height;
		rTextShadow.right = rText.right + (LONG)shad;
		rTextShadow.bottom = rText.bottom + (LONG)shad;
	}
	void Uninit()
	{
		if (pFont)
		{
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
			t2 = GetTickCount64();
			if (t1 == t2)
				t1--;
			current_fps = period_frames * 1000 / (UINT)(t2 - t1);
			time_t t1 = time(NULL);
			tm tm1;
			localtime_s(&tm1, &t1);
			SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
			SpeedGear_FormatText(display_text, ARRAYSIZE(display_text), pMem->statusFormat, pMem->hookSpeed, current_fps,
				sc_desc.BufferDesc.Width, sc_desc.BufferDesc.Height, tm1.tm_hour, tm1.tm_min, tm1.tm_sec, "D3D10");
		}
		pFont->DrawTextA(NULL, display_text, lstrlenA(display_text), &rTextShadow, formatFlag, color_shadow);
		pFont->DrawTextA(NULL, display_text, lstrlenA(display_text), &rText, formatFlag, color_text);
	}
};

static std::map<IDXGISwapChain*, D3DXCustomPresent> cp;

void CustomPresent(IDXGISwapChain* pSC)
{
	if (cp.find(pSC) == cp.end())
	{
		cp.insert(std::make_pair(pSC, D3DXCustomPresent()));
		cp[pSC].Init(pSC);
	}
	cp[pSC].Draw();
}

void CustomResizeBuffers(IDXGISwapChain* p, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	if (cp.find(p) != cp.end())
		cp[p].CalcRect(p, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}
