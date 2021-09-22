#pragma once
#include<dxgi.h>
#ifdef __cplusplus
extern "C" {
#endif
	//自定义Present的附加操作
	void CustomPresent(IDXGISwapChain*);
	void CustomResizeBuffers(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
#ifdef __cplusplus
}
#endif
