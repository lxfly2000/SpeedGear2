#pragma once
#include<Windows.h>
#ifdef __cplusplus
extern "C" {
#endif
void CustomSwapBuffers(HDC);
void CustomViewport(int, int, int, int);
void WINAPI OriginalViewport(int, int, int, int);
#ifdef __cplusplus
}
#endif
