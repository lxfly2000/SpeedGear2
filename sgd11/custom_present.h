#pragma once
#include<dxgi.h>
#ifdef __cplusplus
extern "C" {
#endif
	//�Զ���Present�ĸ��Ӳ���
	void CustomPresent(IDXGISwapChain*);
	void CustomResizeBuffers(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
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