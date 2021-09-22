#pragma once
#pragma once
#include <Windows.h>
#define SPEEDGEAR_MEMORY_MAPPING_NAME "SPEEDGEAR_SHARED_MEMORY"
#define SPEEDGEAR_MAX_HOOK_PROCESS 8
#define SPEEDGEAR_PROC SGProc
#define SPEEDGEAR_PROC_STR _CRT_STRINGIZE(SPEEDGEAR_PROC)

typedef struct
{
	DWORD hookCount;
	DWORD hookPid[SPEEDGEAR_MAX_HOOK_PROCESS];
	float hookSpeed;
	int hookMode;
	int statusPosition;
	BOOL fontItalic;
	LONG fontHeight;
	DWORD fontWeight;
	DWORD fontColor;
	char fontName[32];
	char statusFormat[256];
}SPEEDGEAR_SHARED_MEMORY;

#define FONTHEIGHT_TO_POUND(fs) -MulDiv(fs,72,GetDeviceCaps(GetDC(NULL),LOGPIXELSY))
#define POUND_TO_FONTHEIGHT(p) -MulDiv(p,GetDeviceCaps(GetDC(NULL),LOGPIXELSY),72)
#define DPI_SCALED_VALUE(hwnd,x) MulDiv(x,GetDeviceCaps(GetDC(hwnd),LOGPIXELSY),USER_DEFAULT_SCREEN_DPI)

#ifdef __cplusplus
extern "C"{
#endif
BOOL SpeedGear_InitializeSharedMemory(BOOL bCreate);
BOOL SpeedGear_ReleaseSharedMemory();
BOOL SpeedGear_IsSharedMemoryInitialized();
SPEEDGEAR_SHARED_MEMORY* SpeedGear_GetSharedMemory();

char* SpeedGear_FormatText(char* buf, int len, const char* fmt, float speed, int fps, int width, int height, int hour, int minute, int second);
#ifdef __cplusplus
}
#endif
