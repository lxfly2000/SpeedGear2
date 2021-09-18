#include<Windows.h>
#include<d3d9.h>
#include"..\minhook\include\MinHook.h"
#pragma comment(lib,"d3d9.lib")

#include"custom_present.h"

typedef HRESULT(WINAPI* PFIDirect3DDevice9_Present)(LPDIRECT3DDEVICE9, LPCRECT, LPCRECT, HWND, const RGNDATA*);
static PFIDirect3DDevice9_Present pfPresent = nullptr, pfOriginalPresent = nullptr;
static HMODULE hDllModule;

DWORD GetDLLPath(LPTSTR path, DWORD max_length)
{
	return GetModuleFileName(hDllModule, path, max_length);
}

static int SpeedGear_frameCounter = 0;
#include <d3dkmthk.h>
D3DKMT_WAITFORVERTICALBLANKEVENT getVBlankHandle() {
	//https://docs.microsoft.com/en-us/windows/desktop/gdi/getting-information-on-a-display-monitor
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(DISPLAY_DEVICE);

	DWORD deviceNum = 0;
	while (EnumDisplayDevices(NULL, deviceNum, &dd, 0)) {
		if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			break;
		/*
		DISPLAY_DEVICE newdd = {0};
		newdd.cb = sizeof(DISPLAY_DEVICE);
		DWORD monitorNum = 0;
		while (EnumDisplayDevices(dd.DeviceName, monitorNum, &newdd, 0)) {
			DumpDevice(newdd, 4);
			monitorNum++;
		}
		*/
		deviceNum++;
	}

	HDC hdc = CreateDC(NULL, dd.DeviceName, NULL, NULL);

	D3DKMT_OPENADAPTERFROMHDC OpenAdapterData;

	OpenAdapterData.hDc = hdc;
	if (0 == D3DKMTOpenAdapterFromHdc(&OpenAdapterData)) {
		DeleteDC(hdc);
	}
	else {
		DeleteDC(hdc);
	}
	D3DKMT_WAITFORVERTICALBLANKEVENT we;
	we.hAdapter = OpenAdapterData.hAdapter;
	we.hDevice = 0; //optional. maybe OpenDeviceHandle will give it to us, https://docs.microsoft.com/en-us/windows/desktop/api/dxva2api/nf-dxva2api-idirect3ddevicemanager9-opendevicehandle
	we.VidPnSourceId = OpenAdapterData.VidPnSourceId;

	return we;
}

D3DKMT_WAITFORVERTICALBLANKEVENT wv;
bool wvget = true;

HRESULT hrLastPresent = S_OK;

//Present是STDCALL调用方式，只需把THIS指针放在第一项就可按非成员函数调用
HRESULT __stdcall HookedIDirect3DDevice9_Present(LPDIRECT3DDEVICE9 pDevice, LPCRECT pSrc, LPCRECT pDest, HWND hwnd, const RGNDATA* pRgn)
{
	CustomPresent(pDevice, hrLastPresent);
	//此时函数被拦截，只能通过指针调用，否则要先把HOOK关闭，调用p->Present，再开启HOOK
	//hrLastPresent = pfOriginalPresent(pDevice, pSrc, pDest, hwnd, pRgn);
	if (SpeedGear::GetCurrentSpeed() >= 1.0f)
	{
		if (SpeedGear_frameCounter == 0)
			hrLastPresent = pfOriginalPresent(pDevice, pSrc, pDest, hwnd, pRgn);
		SpeedGear_frameCounter = (SpeedGear_frameCounter + 1) % static_cast<int>(SpeedGear::GetCurrentSpeed());
	}
	else
	{
		hrLastPresent = pfOriginalPresent(pDevice, pSrc, pDest, hwnd, pRgn);
		if (wvget)
		{
			wv = getVBlankHandle();
			wvget = false;
		}
		for (int i = 0; i < (int)(1.0f / SpeedGear::GetCurrentSpeed()); i++)
			D3DKMTWaitForVerticalBlankEvent(&wv);
	}
	return hrLastPresent;
}

PFIDirect3DDevice9_Present GetPresentVAddr()
{
	IDirect3D9* pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D9)
		return nullptr;
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof d3dpp);
	d3dpp.BackBufferWidth = 800;
	d3dpp.BackBufferHeight = 600;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetDesktopWindow();
	d3dpp.Windowed = TRUE;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	IDirect3DDevice9* pDevice;
	D3DCAPS9 caps;
	pD3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	int vp;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	if (FAILED(pD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(), vp, &d3dpp, &pDevice)))
		return nullptr;
	INT_PTR p = reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pDevice)[0])[17];//通过类定义查看函数所在位置
	pDevice->Release();
	pD3D9->Release();
	return reinterpret_cast<PFIDirect3DDevice9_Present>(p);
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StartHook()
{
	pfPresent = reinterpret_cast<PFIDirect3DDevice9_Present>(GetPresentVAddr());
	if (MH_Initialize() != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfPresent, HookedIDirect3DDevice9_Present, reinterpret_cast<void**>(&pfOriginalPresent)) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfPresent) != MH_OK)
		return FALSE;
	return TRUE;
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StopHook()
{
	if (MH_DisableHook(pfPresent) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfPresent) != MH_OK)
		return FALSE;
	if (MH_Uninitialize() != MH_OK)
		return FALSE;
	return TRUE;
}

extern "C" __declspec(dllexport) BOOL SpeedGear_StartHook()
{
	return SpeedGear::InitCustomTime();
}

extern "C" __declspec(dllexport) BOOL SpeedGear_StopHook()
{
	return SpeedGear::UninitCustomTime();
}
DWORD WINAPI TInitHook(LPVOID param)
{
	return StartHook();
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	hDllModule = hInstDll;
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hInstDll);
		SpeedGear_StartHook();//消息钩子不能单独开线程使用
		CreateThread(NULL, 0, TInitHook, NULL, 0, NULL);
		break;
	case DLL_PROCESS_DETACH:
		StopHook();
		SpeedGear_StopHook();
		break;
	case DLL_THREAD_ATTACH:break;
	case DLL_THREAD_DETACH:break;
	}
	return TRUE;
}

//SetWindowHookEx需要一个导出函数，否则DLL不会被加载
extern "C" __declspec(dllexport) LRESULT WINAPI HookProc(int code, WPARAM w, LPARAM l)
{
	return CallNextHookEx(NULL, code, w, l);
}
