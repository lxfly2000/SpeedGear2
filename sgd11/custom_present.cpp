#include"custom_present.h"
#include"..\FW1FontWrapper\FW1FontWrapper\Source\FW1FontWrapper.h"
#include"..\sgshared\sgshared.h"
#include<DirectXMath.h>
#include<map>
#include<string>
#include<ctime>

#ifdef _DEBUG
#define C(x) if(FAILED(x)){MessageBox(NULL,TEXT(_CRT_STRINGIZE(x)),NULL,MB_ICONERROR);throw E_FAIL;}
#else
#define C(x) x
#endif


class D2DCustomPresent
{
private:
	IFW1FontWrapper* fw1FontWrapper;
	DirectX::XMFLOAT2 textpos;
	float textanchorpos_x, textanchorpos_y;
	ID3D11DeviceContext* pContext;
	ULONGLONG t1, t2;
	char display_text[256];
	wchar_t wdisplay_text[256];
	int current_fps;
	int shad;
	UINT period_frames, fcount;
	float fontSize;
	UINT fw1Flags;

	UINT32 calcColor, calcShadowColor;
	DirectX::XMFLOAT2 calcShadowPos;
	DXGI_SWAP_CHAIN_DESC sc_desc;
	ID3D11Device* m_pDevice;
	IDXGISwapChain* m_pSC;
public:
	D2DCustomPresent() :pContext(nullptr), t1(0), t2(0), fcount(0), textanchorpos_x(0), textanchorpos_y(0), calcColor(), calcShadowColor(),
		calcShadowPos(), current_fps(0), display_text(), m_pDevice(NULL), m_pSC(NULL), period_frames(0), sc_desc(), shad(0),
		fw1FontWrapper(nullptr), wdisplay_text(),fontSize(24.0f),fw1Flags(FW1_RESTORESTATE),textpos()
	{
	}
	~D2DCustomPresent()
	{
		Uninit();
	}
	BOOL Init(IDXGISwapChain* pSC)
	{
		m_pSC = pSC;
		ID3D11Device* pDevice;
		C(pSC->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice));//这个不需要在释放时调用Release
		m_pDevice = pDevice;
		pDevice->GetImmediateContext(&pContext);
		SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
		if (pMem == NULL)
		{
			if (!SpeedGear_InitializeSharedMemory(FALSE))
				return FALSE;
			pMem = SpeedGear_GetSharedMemory();
		}

		wchar_t wbuf[32];
		size_t wbc = 0;
		C(pSC->GetDesc(&sc_desc));
		wbc = MultiByteToWideChar(CP_ACP, NULL, pMem->fontName, ARRAYSIZE(pMem->fontName), wbuf, ARRAYSIZE(wbuf));
		IFW1Factory* fw1Factory;
		C(FW1CreateFactory(FW1_VERSION, &fw1Factory));
		C(fw1Factory->CreateFontWrapper(pDevice, wbuf, &fw1FontWrapper));
		fw1Factory->Release();
		fontSize = pMem->useSystemDPI ? LOGICAL_UNIT_TO_PIXEL(sc_desc.OutputWindow, pMem->fontSize) : LOGICAL_UNIT_TO_PIXEL_96DPI(pMem->fontSize);
		float fWidth = (float)sc_desc.BufferDesc.Width, fHeight = (float)sc_desc.BufferDesc.Height;
		textpos.x = (pMem->statusPosition % 3) / 2.0f * fWidth;
		textpos.y = (pMem->statusPosition / 3) / 2.0f * fHeight;
		if (pMem->statusPosition % 3 == 2)
			fw1Flags |= FW1_RIGHT;
		else if (pMem->statusPosition % 3 == 1)
			fw1Flags |= FW1_CENTER;
		else
			fw1Flags |= FW1_LEFT;
		if (pMem->statusPosition / 3 == 2)
			fw1Flags |= FW1_BOTTOM;
		else if (pMem->statusPosition / 3 == 1)
			fw1Flags |= FW1_VCENTER;
		else
			fw1Flags |= FW1_TOP;
		shad = pMem->useSystemDPI ? DPI_SCALED_VALUE(sc_desc.OutputWindow, 2) : 2;
		//注意此处根据微软官方文档的说明，此处的分母是可以指定为0的！！
		period_frames = sc_desc.BufferDesc.RefreshRate.Numerator / max(1, sc_desc.BufferDesc.RefreshRate.Denominator);
		if (period_frames <= 1)
		{
			DEVMODE dm{};
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
			period_frames = dm.dmDisplayFrequency;
		}
		calcShadowPos = { textpos.x + shad, textpos.y + shad };
		calcColor = pMem->fontColor | 0xFF000000;
		calcShadowColor = (calcColor & 0xFEFEFEFE) >> 1;

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
		textpos.x = (pMem->statusPosition % 3) / 2.0f * fWidth;
		textpos.y = (pMem->statusPosition / 3) / 2.0f * fHeight;
		calcShadowPos = { textpos.x + shad, textpos.y + shad };
	}
	void Uninit()
	{
		if (pContext)
		{
			pContext->Release();
			fw1FontWrapper->Release();
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
			current_fps = period_frames * 1000 / (int)(t2 - t1);
			time_t t1 = time(NULL);
			tm tm1;
			localtime_s(&tm1, &t1);
			SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
			SpeedGear_FormatText(display_text, ARRAYSIZE(display_text), pMem->statusFormat, pMem->hookSpeed, current_fps,
				sc_desc.BufferDesc.Width, sc_desc.BufferDesc.Height, tm1.tm_hour, tm1.tm_min, tm1.tm_sec,"D3D11");
			MultiByteToWideChar(CP_ACP, NULL, display_text, ARRAYSIZE(display_text), wdisplay_text, ARRAYSIZE(wdisplay_text));
		}
		//当Viewport大小为0时SpriteBatch会引发异常
		UINT nvp = 1;
		D3D11_VIEWPORT vp;
		pContext->RSGetViewports(&nvp, &vp);
		if (nvp < 1 || vp.Width == 0 || vp.Height == 0)
			return;
		fw1FontWrapper->DrawString(pContext, wdisplay_text, fontSize, calcShadowPos.x, calcShadowPos.y, calcShadowColor, fw1Flags);
		fw1FontWrapper->DrawString(pContext, wdisplay_text, fontSize, textpos.x, textpos.y, calcColor, fw1Flags);
	}
};

static std::map<IDXGISwapChain*, D2DCustomPresent> cp;

void CustomPresent(IDXGISwapChain* pSC)
{
	if (cp.find(pSC) == cp.end())
	{
		cp.insert(std::make_pair(pSC, D2DCustomPresent()));
		cp[pSC].Init(pSC);
	}
	cp[pSC].Draw();
}

void CustomResizeBuffers(IDXGISwapChain* p, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	if (cp.find(p) != cp.end())
		cp[p].CalcRect(p, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}
