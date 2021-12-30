#pragma once
#include<ddraw.h>
#ifdef __cplusplus
extern "C" {
#endif
	void CustomBlt(LPDIRECTDRAWSURFACE, LPRECT, LPDIRECTDRAWSURFACE, LPRECT, DWORD, LPDDBLTFX);
	void CustomBltFast(LPDIRECTDRAWSURFACE, DWORD, DWORD, LPDIRECTDRAWSURFACE, LPRECT, DWORD);
	void CustomFlip(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, DWORD);
#ifdef __cplusplus
}
#endif
