#pragma once
#include<d3d9.h>
#ifdef __cplusplus
extern "C" {
#endif
	//自定义Present的附加操作
	void CustomPresent(LPDIRECT3DDEVICE9, HRESULT);
	DWORD GetDLLPath(LPTSTR path, DWORD max_length);
	namespace SpeedGear
	{
		BOOL InitCustomTime();
		BOOL UninitCustomTime();
		float GetCurrentSpeed();
	}
#ifdef __cplusplus
}
#endif
