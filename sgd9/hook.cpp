#include<Windows.h>
#include<d3d9.h>
#include"..\minhook\include\MinHook.h"
#include"..\sgshared\sgshared.h"
#pragma comment(lib,"d3d9.lib")

#include"custom_present.h"

//关于具体需要Hook哪些函数参考这篇文章：https://blog.csdn.net/fanxiushu/article/details/89426367
typedef HRESULT(WINAPI* PFIDirect3DDevice9_Present)(LPDIRECT3DDEVICE9, LPCRECT, LPCRECT, HWND, const RGNDATA*);
typedef HRESULT(WINAPI* PFIDirect3DDevice9_Reset)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
typedef HRESULT(WINAPI* PFIDirect3DDevice9Ex_PresentEx)(LPDIRECT3DDEVICE9EX, LPCRECT, LPCRECT, HWND, const RGNDATA*, DWORD);
typedef HRESULT(WINAPI* PFIDirect3DDevice9Ex_ResetEx)(LPDIRECT3DDEVICE9EX, D3DPRESENT_PARAMETERS*, D3DDISPLAYMODEEX*);
typedef HRESULT(WINAPI* PFIDirect3DSwapChain9_Present)(LPDIRECT3DSWAPCHAIN9, LPCRECT, LPCRECT, HWND, const RGNDATA*, DWORD);
static PFIDirect3DDevice9_Present pfPresent = nullptr, pfOriginalPresent = nullptr;
static PFIDirect3DDevice9_Reset pfReset = nullptr, pfOriginalReset = nullptr;
static PFIDirect3DDevice9Ex_PresentEx pfPresentEx = nullptr, pfOriginalPresentEx = nullptr;
static PFIDirect3DDevice9Ex_ResetEx pfResetEx = nullptr, pfOriginalResetEx = nullptr;
static PFIDirect3DSwapChain9_Present pfSCPresent = nullptr, pfOriginalSCPresent = nullptr;

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

//Present是STDCALL调用方式，只需把THIS指针放在第一项就可按非成员函数调用
HRESULT __stdcall HookedIDirect3DDevice9_Present(LPDIRECT3DDEVICE9 pDevice, LPCRECT pSrc, LPCRECT pDest, HWND hwnd, const RGNDATA* pRgn)
{
	HRESULT hrLastPresent = S_OK;
	CustomPresent(pDevice);
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	if (pMem == NULL)
	{
		if (!SpeedGear_InitializeSharedMemory(FALSE))
			return pfOriginalPresent(pDevice, pSrc, pDest, hwnd, pRgn);
		pMem = SpeedGear_GetSharedMemory();
	}
	float capturedHookSpeed = pMem->hookSpeed;//线程不安全变量
	//此时函数被拦截，只能通过指针调用，否则要先把HOOK关闭，调用p->Present，再开启HOOK
	if (capturedHookSpeed >= 1.0f)
	{
		if (SpeedGear_frameCounter == 0)
			hrLastPresent = pfOriginalPresent(pDevice, pSrc, pDest, hwnd, pRgn);
		SpeedGear_frameCounter = (SpeedGear_frameCounter + 1) % static_cast<int>(capturedHookSpeed);
	}
	else
	{
		hrLastPresent = pfOriginalPresent(pDevice, pSrc, pDest, hwnd, pRgn);
		if (wvget)
		{
			wv = getVBlankHandle();
			wvget = false;
		}
		for (int i = 0; i < (int)(1.0f / capturedHookSpeed); i++)
			(void)D3DKMTWaitForVerticalBlankEvent(&wv);
	}
	return hrLastPresent;
}

HRESULT WINAPI HookedIDirect3DDevice9_Reset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pParam)
{
	CustomReset(pDevice, pParam);
	return pfOriginalReset(pDevice, pParam);
}

HRESULT WINAPI HookedIDirect3DDevice9Ex_PresentEx(LPDIRECT3DDEVICE9EX pDevice, LPCRECT pSrc, LPCRECT pDest, HWND hwnd, const RGNDATA* pRgn, DWORD f)
{
	CustomPresentEx(pDevice);
	return pfOriginalPresentEx(pDevice, pSrc, pDest, hwnd, pRgn, f);
}

HRESULT WINAPI HookedIDirect3DDevice9Ex_ResetEx(LPDIRECT3DDEVICE9EX pDevice, D3DPRESENT_PARAMETERS* pParam, D3DDISPLAYMODEEX* pMode)
{
	CustomResetEx(pDevice, pParam, pMode);
	return pfOriginalResetEx(pDevice, pParam, pMode);
}

HRESULT WINAPI HookedIDirect3DSwapChain9_Present(LPDIRECT3DSWAPCHAIN9 pSC, LPCRECT pSrc, LPCRECT pDest, HWND hwnd, const RGNDATA* pRgn, DWORD f)
{
	CustomSCPresent(pSC);
	return pfOriginalSCPresent(pSC, pSrc, pDest, hwnd, pRgn, f);
}

BOOL GetPresentVAddr(PFIDirect3DDevice9_Present*pPresent,PFIDirect3DDevice9_Reset*pReset,
	PFIDirect3DDevice9Ex_PresentEx*pPresentEx,PFIDirect3DDevice9Ex_ResetEx*pResetEx,PFIDirect3DSwapChain9_Present*pSCPresent)
{
	IDirect3D9Ex* pD3D9 = nullptr;
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3D9)))
		return FALSE;
	if (!pD3D9)
		return FALSE;
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
	IDirect3DDevice9* pDevice = nullptr;
	IDirect3DDevice9Ex* pDeviceEx = nullptr;
	D3DCAPS9 caps;
	if (FAILED(pD3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)))
		return FALSE;
	int vp;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	if (FAILED(pD3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(), vp, &d3dpp, &pDevice)))
		return FALSE;
	IDirect3DSwapChain9* pSC = nullptr;
	pDevice->GetSwapChain(0, &pSC);
	if (FAILED(pD3D9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(), vp, &d3dpp, nullptr, &pDeviceEx)))
		return FALSE;
	INT_PTR pP = reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pDevice)[0])[17];//通过类定义查看函数所在位置
	INT_PTR pR = reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pDevice)[0])[16];
	INT_PTR pPEx = reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pDeviceEx)[0])[121];
	INT_PTR pREx = reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pDeviceEx)[0])[132];
	INT_PTR pSCP = reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pSC)[0])[3];
	*pPresent = reinterpret_cast<PFIDirect3DDevice9_Present>(pP);
	*pReset = reinterpret_cast<PFIDirect3DDevice9_Reset>(pR);
	*pPresentEx = reinterpret_cast<PFIDirect3DDevice9Ex_PresentEx>(pPEx);
	*pResetEx = reinterpret_cast<PFIDirect3DDevice9Ex_ResetEx>(pREx);
	*pSCPresent = reinterpret_cast<PFIDirect3DSwapChain9_Present>(pSCP);
	pDevice->Release();
	pDeviceEx->Release();
	pD3D9->Release();
	return TRUE;
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StartHook()
{
	if (!GetPresentVAddr(&pfPresent, &pfReset, &pfPresentEx, &pfResetEx, &pfSCPresent))
		return FALSE;
	if (MH_Initialize() != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfPresent, HookedIDirect3DDevice9_Present, reinterpret_cast<void**>(&pfOriginalPresent)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfReset, HookedIDirect3DDevice9_Reset, reinterpret_cast<void**>(&pfOriginalReset)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfPresentEx, HookedIDirect3DDevice9Ex_PresentEx, reinterpret_cast<void**>(&pfOriginalPresentEx)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfResetEx, HookedIDirect3DDevice9Ex_ResetEx, reinterpret_cast<void**>(&pfOriginalResetEx)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfSCPresent, HookedIDirect3DSwapChain9_Present, reinterpret_cast<void**>(&pfOriginalSCPresent)) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfPresent) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfReset) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfPresentEx) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfResetEx) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfSCPresent) != MH_OK)
		return FALSE;
	return TRUE;
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StopHook()
{
	if (MH_DisableHook(pfPresent) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfReset) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfPresentEx) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfResetEx) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfSCPresent) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfPresent) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfReset) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfPresentEx) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfResetEx) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfSCPresent) != MH_OK)
		return FALSE;
	if (MH_Uninitialize() != MH_OK)
		return FALSE;
	return TRUE;
}

DWORD WINAPI TInitHook(LPVOID param)
{
	return StartHook();
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hInstDll);
		CreateThread(NULL, 0, TInitHook, NULL, 0, NULL);
		break;
	case DLL_PROCESS_DETACH:
		StopHook();
		break;
	case DLL_THREAD_ATTACH:break;
	case DLL_THREAD_DETACH:break;
	}
	return TRUE;
}

//SetWindowHookEx需要一个导出函数，否则DLL不会被加载
extern "C" __declspec(dllexport) LRESULT WINAPI SPEEDGEAR_PROC(int code, WPARAM w, LPARAM l)
{
	return CallNextHookEx(NULL, code, w, l);
}
