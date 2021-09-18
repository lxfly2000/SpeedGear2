#include"custom_present.h"
#include"ResLoader.h"
#include"..\DirectXTK\Inc\SimpleMath.h"
#include<map>
#include<string>
#include<ctime>

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

class D2DCustomPresent
{
private:
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;
	DirectX::SimpleMath::Vector2 textpos;
	float textanchorpos_x, textanchorpos_y;
	ID3D11DeviceContext* pContext;
	unsigned t1, t2, fcount;
	std::wstring display_text;
	int current_fps;
	TCHAR time_text[32], fps_text[32], width_text[32], height_text[32], speed_text[32];

	TCHAR font_name[256], font_size[16], text_x[16], text_y[16], text_align[16], text_valign[16], display_text_fmt[256], fps_fmt[32], time_fmt[32], width_fmt[32], height_fmt[32], speed_fmt[32];
	TCHAR font_red[16], font_green[16], font_blue[16], font_alpha[16];
	TCHAR font_shadow_red[16], font_shadow_green[16], font_shadow_blue[16], font_shadow_alpha[16], font_shadow_distance[16];
	int font_weight, period_frames;
	DirectX::XMVECTOR calcColor, calcShadowColor;
	DirectX::XMFLOAT2 calcShadowPos;
	DXGI_SWAP_CHAIN_DESC sc_desc;
	ID3D11Device* m_pDevice;
	IDXGISwapChain* m_pSC;
public:
	D2DCustomPresent() :pContext(nullptr), t1(0), t2(0), fcount(0)
	{
	}
	D2DCustomPresent(D2DCustomPresent&& other)
	{
		spriteBatch = std::move(other.spriteBatch);
		spriteFont = std::move(other.spriteFont);
		textpos = std::move(other.textpos);
		pContext = std::move(other.pContext);
		t1 = std::move(other.t1);
		t2 = std::move(other.t2);
		fcount = std::move(other.fcount);
		m_pSC = std::move(other.m_pSC);
		m_pDevice = std::move(other.m_pDevice);
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
		spriteBatch = std::make_unique<DirectX::SpriteBatch>(pContext);

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

		C(LoadFontFromSystem(m_pDevice, spriteFont, 1024, 1024, font_name, F(font_size),
			D2D1::ColorF(D2D1::ColorF::White), (DWRITE_FONT_WEIGHT)font_weight));
		C(pSC->GetDesc(&sc_desc));
		float fWidth = (float)sc_desc.BufferDesc.Width, fHeight = (float)sc_desc.BufferDesc.Height;
		textpos.x = F(text_x) * fWidth;
		textpos.y = F(text_y) * fHeight;
		if (lstrcmpi(text_align, TEXT("right")) == 0)
			textanchorpos_x = 1.0f;
		else if (lstrcmpi(text_align, TEXT("center")) == 0)
			textanchorpos_x = 0.5f;
		else if (lstrcmpi(text_align, TEXT("left")) == 0)
			textanchorpos_x = 0.0f;
		else
			textanchorpos_x = F(text_align);
		if (lstrcmpi(text_valign, TEXT("bottom")) == 0)
			textanchorpos_y = 1.0f;
		else if (lstrcmpi(text_valign, TEXT("center")) == 0)
			textanchorpos_y = 0.5f;
		else if (lstrcmpi(text_valign, TEXT("top")) == 0)
			textanchorpos_y = 0.0f;
		else
			textanchorpos_y = F(text_valign);
		calcShadowPos = DirectX::SimpleMath::Vector2(textpos.x + F(font_shadow_distance), textpos.y + F(font_shadow_distance));
		DirectX::XMFLOAT4 xm = DirectX::XMFLOAT4(F(font_red), F(font_green), F(font_blue), F(font_alpha));
		calcColor = DirectX::XMLoadFloat4(&xm);
		xm = DirectX::XMFLOAT4(F(font_shadow_red), F(font_shadow_green), F(font_shadow_blue), F(font_shadow_alpha));
		calcShadowColor = DirectX::XMLoadFloat4(&xm);

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
		textpos.x = F(text_x) * fWidth;
		textpos.y = F(text_y) * fHeight;
		calcShadowPos = DirectX::SimpleMath::Vector2(textpos.x + F(font_shadow_distance), textpos.y + F(font_shadow_distance));
	}
	void Uninit()
	{
		if (pContext)
		{
			pContext->Release();
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
			wsprintf(width_text, width_fmt, sc_desc.BufferDesc.Width);
			wsprintf(height_text, height_fmt, sc_desc.BufferDesc.Height);
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
		//使用SpriteBatch会破坏之前的渲染器状态并且不会自动保存和恢复原状态，画图前应先保存原来的状态，完成后恢复
		//参考：https://github.com/Microsoft/DirectXTK/wiki/SpriteBatch#state-management
		//https://github.com/ocornut/imgui/blob/master/examples/imgui_impl_dx11.cpp#L130
		//在我写的另一个程序里测试时发现只要运行Hook后不管是否停止都没法再画出三角形了，可能有的资源是无法恢复的吧
#pragma region 获取原来的状态
		ID3D11BlendState* blendState; FLOAT blendFactor[4]; UINT sampleMask;
		ID3D11SamplerState* samplerStateVS0;
		ID3D11DepthStencilState* depthStencilState; UINT stencilRef;
		ID3D11Buffer* indexBuffer; DXGI_FORMAT indexBufferFormat; UINT indexBufferOffset;
		ID3D11InputLayout* inputLayout;
		ID3D11PixelShader* pixelShader; ID3D11ClassInstance* psClassInstances[256]; UINT psNClassInstances = 256;
		D3D11_PRIMITIVE_TOPOLOGY primitiveTopology;
		ID3D11RasterizerState* rasterState;
		ID3D11SamplerState* samplerStatePS0;
		ID3D11ShaderResourceView* resourceViewPS0;
		ID3D11Buffer* vb0; UINT stridesVB0, offsetVB0;
		ID3D11VertexShader* vertexShader; ID3D11ClassInstance* vsClassInstances[256]; UINT vsNClassInstances = 256;
		pContext->OMGetBlendState(&blendState, blendFactor, &sampleMask);
		pContext->VSGetSamplers(0, 1, &samplerStateVS0);
		pContext->OMGetDepthStencilState(&depthStencilState, &stencilRef);
		pContext->IAGetIndexBuffer(&indexBuffer, &indexBufferFormat, &indexBufferOffset);
		pContext->IAGetInputLayout(&inputLayout);//Need check
		pContext->PSGetShader(&pixelShader, psClassInstances, &psNClassInstances);//Need check
		pContext->IAGetPrimitiveTopology(&primitiveTopology);
		pContext->RSGetState(&rasterState);
		pContext->PSGetSamplers(0, 1, &samplerStatePS0);//Need check
		pContext->PSGetShaderResources(0, 1, &resourceViewPS0);
		pContext->IAGetVertexBuffers(0, 1, &vb0, &stridesVB0, &offsetVB0);
		pContext->VSGetShader(&vertexShader, vsClassInstances, &vsNClassInstances);//Need check
#pragma endregion
#pragma region 用SpriteBatch绘制
		spriteBatch->Begin();
		auto v = spriteFont->MeasureString(display_text.c_str());
		DirectX::XMFLOAT2 textanchorpos = { textanchorpos_x * DirectX::XMVectorGetX(v),textanchorpos_y * DirectX::XMVectorGetY(v) };
		spriteFont->DrawString(spriteBatch.get(), display_text.c_str(), calcShadowPos, calcShadowColor, 0.0f, textanchorpos);
		spriteFont->DrawString(spriteBatch.get(), display_text.c_str(), textpos, calcColor, 0.0f, textanchorpos);
		spriteBatch->End();
#pragma endregion
#pragma region 恢复原来的状态
		pContext->OMSetBlendState(blendState, blendFactor, sampleMask);
		pContext->VSSetSamplers(0, 1, &samplerStateVS0);
		pContext->OMSetDepthStencilState(depthStencilState, stencilRef);
		pContext->IASetIndexBuffer(indexBuffer, indexBufferFormat, indexBufferOffset);
		pContext->IASetInputLayout(inputLayout);
		pContext->PSSetShader(pixelShader, psClassInstances, psNClassInstances);
		pContext->IASetPrimitiveTopology(primitiveTopology);
		pContext->RSSetState(rasterState);
		pContext->PSSetSamplers(0, 1, &samplerStatePS0);
		pContext->PSSetShaderResources(0, 1, &resourceViewPS0);
		pContext->IASetVertexBuffers(0, 1, &vb0, &stridesVB0, &offsetVB0);
		pContext->VSSetShader(vertexShader, vsClassInstances, vsNClassInstances);
#pragma endregion
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
