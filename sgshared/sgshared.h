#pragma once
#pragma once
#include <Windows.h>
#define SPEEDGEAR_MEMORY_MAPPING_NAME "SPEEDGEAR_SHARED_MEMORY"
#define SPEEDGEAR_PROC SGProc
#ifdef _WIN64
#define SPEEDGEAR_PROC_STR _CRT_STRINGIZE(SPEEDGEAR_PROC)
#else
#define SPEEDGEAR_PROC_STR "_" _CRT_STRINGIZE(SPEEDGEAR_PROC) "@12"
#endif

typedef struct
{
	DWORD hookCount;
	DWORD hwndGui;
	float hookSpeed;
	int hookMode;
	int statusPosition;
	BOOL useSystemDPI;
	BOOL fontItalic;
	DWORD fontSize;//Point, LOGFONT::lfHeight(Pixel)
	DWORD fontWeight;
	DWORD fontColor;
	char fontName[32];
	char statusFormat[256];
	char fontPath[MAX_PATH];
}SPEEDGEAR_SHARED_MEMORY;

//WPARAM: LOW:SG_UPDATE_LIST_* HIGH:SG_UPDATE_LIST_API_*
//LPARAM: PID
#define SG_MESSAGE_UPDATE_LIST WM_USER+2000
#define SG_UPDATE_LIST_REFRESH 0
#define SG_UPDATE_LIST_ADD 1
#define SG_UPDATE_LIST_REMOVE 2
#define SG_UPDATE_LIST_API_D3D8 1
#define SG_UPDATE_LIST_API_D3D9 2
#define SG_UPDATE_LIST_API_D3D11 3
#define SG_UPDATE_LIST_API_GL 4
#define SG_UPDATE_LIST_API_D3D10 5
#define SG_UPDATE_LIST_API_DDRAW 6
#define SG_UPDATE_LIST_API_VULKAN 7

#define PIXEL_TO_LOGICAL_UNIT(hwnd,fs) MulDiv(fs,72,GetDeviceCaps(GetDC(hwnd),LOGPIXELSY))
#define LOGICAL_UNIT_TO_PIXEL(hwnd,p) MulDiv(p,GetDeviceCaps(GetDC(hwnd),LOGPIXELSY),72)
#define PIXEL_TO_LOGICAL_UNIT_96DPI(fs) MulDiv(fs,72,USER_DEFAULT_SCREEN_DPI)
#define LOGICAL_UNIT_TO_PIXEL_96DPI(p) MulDiv(p,USER_DEFAULT_SCREEN_DPI,72)
#define DPI_SCALED_VALUE(hwnd,x) MulDiv(x,GetDeviceCaps(GetDC(hwnd),LOGPIXELSY),USER_DEFAULT_SCREEN_DPI)

#ifdef __cplusplus
extern "C"{
#endif
BOOL SGSendMessageUpdateList(DWORD option,DWORD api,DWORD pid);

BOOL SpeedGear_InitializeSharedMemory(BOOL bCreate);
BOOL SpeedGear_ReleaseSharedMemory();
BOOL SpeedGear_IsSharedMemoryInitialized();
SPEEDGEAR_SHARED_MEMORY* SpeedGear_GetSharedMemory();

char* SpeedGear_FormatText(char* buf, int len, const char* fmt, float speed, int fps, int width, int height, int hour, int minute, int second,const char*api);
#ifdef __cplusplus
}
#endif
