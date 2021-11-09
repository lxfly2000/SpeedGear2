//https://pastebin.com/f6d87dd03
#include<Windows.h>
#include"glad\glad.h"
#include"..\minhook\include\MinHook.h"
#include"..\sgshared\sgshared.h"
#include"custom_swapbuffers.h"

typedef BOOL(WINAPI*PFwglSwapBuffers)(HDC);
typedef PFNGLVIEWPORTPROC PFglViewport;
static PFwglSwapBuffers pfSwapBuffers = nullptr, pfOriginalSwapBuffers = nullptr;
static PFglViewport pfViewport = nullptr, pfOriginalViewport = nullptr;

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
BOOL wvget = true;

BOOL WINAPI HookedwglSwapBuffers(HDC p)
{
	CustomSwapBuffers(p);
	BOOL r = FALSE;
	float capturedHookSpeed = SpeedGear_GetSharedMemory()->hookSpeed;//线程不安全变量
	if (capturedHookSpeed >= 1.0f)
	{
		if (SpeedGear_frameCounter == 0)
			r = pfOriginalSwapBuffers(p);
		SpeedGear_frameCounter = (SpeedGear_frameCounter + 1) % static_cast<int>(capturedHookSpeed);
	}
	else
	{
		r = pfOriginalSwapBuffers(p);
		if (wvget)
		{
			wv = getVBlankHandle();
			wvget = false;
		}
		for (int i = 0; i < (int)(1.0f / capturedHookSpeed); i++)
			D3DKMTWaitForVerticalBlankEvent(&wv);
	}
	return r;
}

void WINAPI HookedglViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	CustomViewport(x, y, width, height);
	return pfOriginalViewport(x, y, width, height);
}

void WINAPI OriginalViewport(int x, int y, int width, int height)
{
	return pfOriginalViewport(x, y, width, height);
}

PFwglSwapBuffers GetSwapBuffersAddr()
{
	return reinterpret_cast<PFwglSwapBuffers>(GetProcAddress(LoadLibrary(TEXT("OpenGL32.dll")), "wglSwapBuffers"));
}

PFglViewport GetViewportAddr()
{
	return reinterpret_cast<PFglViewport>(GetProcAddress(LoadLibrary(TEXT("OpenGL32.dll")), "glViewport"));
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StartHook()
{
	pfSwapBuffers = GetSwapBuffersAddr();
	pfViewport = GetViewportAddr();
	if (MH_Initialize() != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfSwapBuffers, HookedwglSwapBuffers, reinterpret_cast<void**>(&pfOriginalSwapBuffers)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfViewport, HookedglViewport, reinterpret_cast<void**>(&pfOriginalViewport)) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfSwapBuffers) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfViewport) != MH_OK)
		return FALSE;
	return TRUE;
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StopHook()
{
	if (MH_DisableHook(pfViewport) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfSwapBuffers) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfViewport) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfSwapBuffers) != MH_OK)
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