#include<Windows.h>
#include<ddraw.h>
#include"..\minhook\include\MinHook.h"
#include"..\sgshared\sgshared.h"
#pragma comment(lib,"ddraw.lib")

#include"custom_blt.h"

void CustomBlt(LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX);
void CustomBltFast(LPDIRECTDRAWSURFACE, DWORD, DWORD, LPDIRECTDRAWSURFACE, LPRECT, DWORD);
void CustomFlip(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, DWORD);

typedef HRESULT(WINAPI* PFIDirectDrawSurface_Blt)(LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX);
typedef HRESULT(WINAPI* PFIDirectDrawSurface_BltFast)(LPDIRECTDRAWSURFACE, DWORD, DWORD, LPDIRECTDRAWSURFACE, LPRECT, DWORD);
typedef HRESULT(WINAPI* PFIDirectDrawSurface_Flip)(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, DWORD);
static PFIDirectDrawSurface_Blt pfBlt = nullptr, pfOriginalBlt = nullptr;
static PFIDirectDrawSurface_BltFast pfBltFast = nullptr, pfOriginalBltFast = nullptr;
static PFIDirectDrawSurface_Flip pfFlip = nullptr, pfOriginalFlip = nullptr;

static int SpeedGear_frameCounter = 0;
LPDIRECTDRAW g_pDD = nullptr;

void WaitForVSync()
{
	g_pDD->WaitForVerticalBlank(DDWAITVB_BLOCKEND, NULL);
}

bool bUpdateList = true;

//Present是STDCALL调用方式，只需把THIS指针放在第一项就可按非成员函数调用
HRESULT __stdcall HookedIDirectDrawSurface_Blt(LPDIRECTDRAWSURFACE p, LPRECT rcTo, LPDIRECTDRAWSURFACE sSource, LPRECT rcSrc, DWORD flag, LPDDBLTFX fx)
{
	HRESULT hrLastPresent = S_OK;
	CustomBlt(p, rcTo, sSource, rcSrc, flag, fx);
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	if (pMem == NULL)
	{
		if (!SpeedGear_InitializeSharedMemory(FALSE))
			return pfOriginalBlt(p, rcTo, sSource, rcSrc, flag, fx);
		pMem = SpeedGear_GetSharedMemory();
	}
	if (bUpdateList)
	{
		bUpdateList = false;
		SGSendMessageUpdateList(SG_UPDATE_LIST_ADD, SG_UPDATE_LIST_API_DDRAW, GetCurrentProcessId());
	}
	float capturedHookSpeed = pMem->hookSpeed;//线程不安全变量
	//此时函数被拦截，只能通过指针调用，否则要先把HOOK关闭，调用p->Present，再开启HOOK
	if (capturedHookSpeed >= 1.0f)
	{
		if (SpeedGear_frameCounter == 0)
			hrLastPresent = pfOriginalBlt(p, rcTo, sSource, rcSrc, flag, fx);
		SpeedGear_frameCounter = (SpeedGear_frameCounter + 1) % static_cast<int>(capturedHookSpeed);
	}
	else
	{
		hrLastPresent = pfOriginalBlt(p, rcTo, sSource, rcSrc, flag, fx);
		for (int i = 0; i < (int)(1.0f / capturedHookSpeed); i++)
			WaitForVSync();
	}
	return hrLastPresent;
}

HRESULT WINAPI HookedIDirectDrawSurface_BltFast(LPDIRECTDRAWSURFACE p, DWORD toX, DWORD toY, LPDIRECTDRAWSURFACE sSource, LPRECT rcSrc, DWORD flag)
{
	HRESULT hrLastPresent = S_OK;
	CustomBltFast(p, toX, toY, sSource, rcSrc, flag);
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	if (pMem == NULL)
	{
		if (!SpeedGear_InitializeSharedMemory(FALSE))
			return pfOriginalBltFast(p, toX, toY, sSource, rcSrc, flag);
		pMem = SpeedGear_GetSharedMemory();
	}
	if (bUpdateList)
	{
		bUpdateList = false;
		SGSendMessageUpdateList(SG_UPDATE_LIST_ADD, SG_UPDATE_LIST_API_DDRAW, GetCurrentProcessId());
	}
	float capturedHookSpeed = pMem->hookSpeed;//线程不安全变量
	//此时函数被拦截，只能通过指针调用，否则要先把HOOK关闭，调用p->Present，再开启HOOK
	if (capturedHookSpeed >= 1.0f)
	{
		if (SpeedGear_frameCounter == 0)
			hrLastPresent = pfOriginalBltFast(p, toX, toY, sSource, rcSrc, flag);
		SpeedGear_frameCounter = (SpeedGear_frameCounter + 1) % static_cast<int>(capturedHookSpeed);
	}
	else
	{
		hrLastPresent = pfOriginalBltFast(p, toX, toY, sSource, rcSrc, flag);
		for (int i = 0; i < (int)(1.0f / capturedHookSpeed); i++)
			WaitForVSync();
	}
	return hrLastPresent;
}

HRESULT WINAPI HookedIDirectDrawSurface_Flip(LPDIRECTDRAWSURFACE p, LPDIRECTDRAWSURFACE sSource, DWORD flag)
{
	HRESULT hrLastPresent = S_OK;
	CustomFlip(p, sSource, flag);
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	if (pMem == NULL)
	{
		if (!SpeedGear_InitializeSharedMemory(FALSE))
			return pfOriginalFlip(p, sSource, flag);
		pMem = SpeedGear_GetSharedMemory();
	}
	if (bUpdateList)
	{
		bUpdateList = false;
		SGSendMessageUpdateList(SG_UPDATE_LIST_ADD, SG_UPDATE_LIST_API_DDRAW, GetCurrentProcessId());
	}
	float capturedHookSpeed = pMem->hookSpeed;//线程不安全变量
	//此时函数被拦截，只能通过指针调用，否则要先把HOOK关闭，调用p->Present，再开启HOOK
	if (capturedHookSpeed >= 1.0f)
	{
		if (SpeedGear_frameCounter == 0)
			hrLastPresent = pfOriginalFlip(p, sSource, flag);
		SpeedGear_frameCounter = (SpeedGear_frameCounter + 1) % static_cast<int>(capturedHookSpeed);
	}
	else
	{
		hrLastPresent = pfOriginalFlip(p, sSource, flag);
		for (int i = 0; i < (int)(1.0f / capturedHookSpeed); i++)
			WaitForVSync();
	}
	return hrLastPresent;
}

BOOL GetPresentVAddr(PFIDirectDrawSurface_Blt* pBlt, PFIDirectDrawSurface_BltFast* pBltFast, PFIDirectDrawSurface_Flip* pFlip)
{
	if (FAILED(g_pDD->SetCooperativeLevel(NULL, DDSCL_NORMAL)))
		return FALSE;
	DDSURFACEDESC desc;
	desc.dwSize = sizeof(desc);
	IDirectDrawSurface*s;
	desc.dwFlags = DDSD_CAPS;
	desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (FAILED(g_pDD->CreateSurface(&desc, &s, NULL)))
		return FALSE;
	INT_PTR pB = reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(s)[0])[5];//通过类定义查看函数所在位置
	INT_PTR pF = reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(s)[0])[7];
	INT_PTR pL = reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(s)[0])[11];
	s->Release();
	*pBlt = reinterpret_cast<PFIDirectDrawSurface_Blt>(pB);
	*pBltFast = reinterpret_cast<PFIDirectDrawSurface_BltFast>(pF);
	*pFlip = reinterpret_cast<PFIDirectDrawSurface_Flip>(pL);
	return TRUE;
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StartHook()
{
	if (FAILED(DirectDrawCreate(NULL, &g_pDD, NULL)))
		return FALSE;
	if (!GetPresentVAddr(&pfBlt, &pfBltFast, &pfFlip))
		return FALSE;
	if (MH_Initialize() != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfBlt, HookedIDirectDrawSurface_Blt, reinterpret_cast<void**>(&pfOriginalBlt)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfBltFast, HookedIDirectDrawSurface_BltFast, reinterpret_cast<void**>(&pfOriginalBltFast)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfFlip, HookedIDirectDrawSurface_Flip, reinterpret_cast<void**>(&pfOriginalFlip)) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfBlt) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfBltFast) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfFlip) != MH_OK)
		return FALSE;
	return TRUE;
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StopHook()
{
	SGSendMessageUpdateList(SG_UPDATE_LIST_REMOVE, SG_UPDATE_LIST_API_DDRAW, GetCurrentProcessId());
	if (MH_DisableHook(pfBlt) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfBltFast) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfFlip) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfBlt) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfBltFast) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfFlip) != MH_OK)
		return FALSE;
	if (MH_Uninitialize() != MH_OK)
		return FALSE;
	if (g_pDD)
	{
		g_pDD->Release();
		g_pDD = nullptr;
	}
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
