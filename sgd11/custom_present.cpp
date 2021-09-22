#include"custom_present.h"
#include"ResLoader.h"
#include"..\DirectXTK\Inc\SimpleMath.h"
#include"..\sgshared\sgshared.h"
#include<map>
#include<string>
#include<ctime>
#include<atlconv.h>

#ifdef _DEBUG
#define C(x) if(FAILED(x)){MessageBox(NULL,TEXT(_CRT_STRINGIZE(x)),NULL,MB_ICONERROR);throw E_FAIL;}
#else
#define C(x) x
#endif


class D2DCustomPresent
{
private:
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;
	DirectX::SimpleMath::Vector2 textpos;
	float textanchorpos_x, textanchorpos_y;
	ID3D11DeviceContext* pContext;
	unsigned t1, t2, fcount;
	char display_text[256];
	int current_fps;
	int shad;
	UINT period_frames;

	DirectX::XMVECTOR calcColor, calcShadowColor;
	DirectX::XMFLOAT2 calcShadowPos;
	DXGI_SWAP_CHAIN_DESC sc_desc;
	ID3D11Device* m_pDevice;
	IDXGISwapChain* m_pSC;
public:
	D2DCustomPresent() :pContext(nullptr), t1(0), t2(0), fcount(0)
	{
	}
	D2DCustomPresent(D2DCustomPresent&& other)
	{
		spriteBatch = std::move(other.spriteBatch);
		spriteFont = std::move(other.spriteFont);
		textpos = std::move(other.textpos);
		pContext = std::move(other.pContext);
		t1 = std::move(other.t1);
		t2 = std::move(other.t2);
		fcount = std::move(other.fcount);
		m_pSC = std::move(other.m_pSC);
		m_pDevice = std::move(other.m_pDevice);
	}
	~D2DCustomPresent()
	{
		Uninit();
	}
	BOOL Init(IDXGISwapChain* pSC)
	{
		m_pSC = pSC;
		ID3D11Device* pDevice;
		C(pSC->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice));//这个不需要在释放时调用Release
		m_pDevice = pDevice;
		pDevice->GetImmediateContext(&pContext);
		spriteBatch = std::make_unique<DirectX::SpriteBatch>(pContext);
		SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();

		USES_CONVERSION;
		C(LoadFontFromSystem(m_pDevice, spriteFont, 1024, 1024, A2W(pMem->fontName), FONTHEIGHT_TO_POUND(pMem->fontHeight),
			D2D1::ColorF(D2D1::ColorF::White), (DWRITE_FONT_WEIGHT)pMem->fontWeight));
		C(pSC->GetDesc(&sc_desc));
		float fWidth = (float)sc_desc.BufferDesc.Width, fHeight = (float)sc_desc.BufferDesc.Height;
		textpos.x = (pMem->statusPosition % 3) / 2.0f * fWidth;
		textpos.y = (pMem->statusPosition / 3) / 2.0f * fHeight;
		if (pMem->statusPosition % 3 == 2)
			textanchorpos_x = 1.0f;
		else if (pMem->statusPosition % 3 == 1)
			textanchorpos_x = 0.5f;
		else
			textanchorpos_x = 0.0f;
		if (pMem->statusPosition / 3 == 2)
			textanchorpos_y = 1.0f;
		else if (pMem->statusPosition / 3 == 1)
			textanchorpos_y = 0.5f;
		else
			textanchorpos_y = 0.0f;
		shad = DPI_SCALED_VALUE(sc_desc.OutputWindow, 2);
		period_frames = sc_desc.BufferDesc.RefreshRate.Numerator / sc_desc.BufferDesc.RefreshRate.Denominator;
		calcShadowPos = DirectX::SimpleMath::Vector2(textpos.x + shad, textpos.y + shad);
		DirectX::XMFLOAT4 xm = DirectX::XMFLOAT4((pMem->fontColor & 0xFF) / 255.0f, ((pMem->fontColor >> 8) & 0xFF) / 255.0f,
			((pMem->fontColor >> 16) & 0xFF) / 255.0f, 1.0f);
		calcColor = DirectX::XMLoadFloat4(&xm);
		xm.x /= 2.0f;
		xm.y /= 2.0f;
		xm.z /= 2.0f;
		xm.w /= 2.0f;
		calcShadowColor = DirectX::XMLoadFloat4(&xm);

		return TRUE;
	}
	void CalcRect(IDXGISwapChain* pSC, UINT BufferCount, UINT width, UINT height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		sc_desc.BufferCount = BufferCount;
		sc_desc.BufferDesc.Width = width;
		sc_desc.BufferDesc.Height = height;
		sc_desc.BufferDesc.Format = NewFormat;
		sc_desc.Flags = SwapChainFlags;
		float fWidth = (float)sc_desc.BufferDesc.Width, fHeight = (float)sc_desc.BufferDesc.Height;
		SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
		textpos.x = (pMem->statusPosition % 3) / 2.0f * fWidth;
		textpos.y = (pMem->statusPosition / 3) / 2.0f * fHeight;
		calcShadowPos = DirectX::SimpleMath::Vector2(textpos.x + shad, textpos.y + shad);
	}
	void Uninit()
	{
		if (pContext)
		{
			pContext->Release();
		}
	}
	void Draw()
	{
		if (fcount-- == 0)
		{
			fcount = period_frames;
			t1 = t2;
			t2 = GetTickCount();
			if (t1 == t2)
				t1--;
			current_fps = period_frames * 1000 / (t2 - t1);
			time_t t1 = time(NULL);
			tm tm1;
			localtime_s(&tm1, &t1);
			SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
			SpeedGear_FormatText(display_text, ARRAYSIZE(display_text), pMem->statusFormat, pMem->hookSpeed, current_fps,
				sc_desc.BufferDesc.Width, sc_desc.BufferDesc.Height, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
		}
		//使用SpriteBatch会破坏之前的渲染器状态并且不会自动保存和恢复原状态，画图前应先保存原来的状态，完成后恢复
		//参考：https://github.com/Microsoft/DirectXTK/wiki/SpriteBatch#state-management
		//https://github.com/ocornut/imgui/blob/master/examples/imgui_impl_dx11.cpp#L130
		//在我写的另一个程序里测试时发现只要运行Hook后不管是否停止都没法再画出三角形了，可能有的资源是无法恢复的吧
#pragma region 获取原来的状态
		ID3D11BlendState* blendState; FLOAT blendFactor[4]; UINT sampleMask;
		ID3D11SamplerState* samplerStateVS0;
		ID3D11DepthStencilState* depthStencilState; UINT stencilRef;
		ID3D11Buffer* indexBuffer; DXGI_FORMAT indexBufferFormat; UINT indexBufferOffset;
		ID3D11InputLayout* inputLayout;
		ID3D11PixelShader* pixelShader; ID3D11ClassInstance* psClassInstances[256]; UINT psNClassInstances = 256;
		D3D11_PRIMITIVE_TOPOLOGY primitiveTopology;
		ID3D11RasterizerState* rasterState;
		ID3D11SamplerState* samplerStatePS0;
		ID3D11ShaderResourceView* resourceViewPS0;
		ID3D11Buffer* vb0; UINT stridesVB0, offsetVB0;
		ID3D11VertexShader* vertexShader; ID3D11ClassInstance* vsClassInstances[256]; UINT vsNClassInstances = 256;
		pContext->OMGetBlendState(&blendState, blendFactor, &sampleMask);
		pContext->VSGetSamplers(0, 1, &samplerStateVS0);
		pContext->OMGetDepthStencilState(&depthStencilState, &stencilRef);
		pContext->IAGetIndexBuffer(&indexBuffer, &indexBufferFormat, &indexBufferOffset);
		pContext->IAGetInputLayout(&inputLayout);//Need check
		pContext->PSGetShader(&pixelShader, psClassInstances, &psNClassInstances);//Need check
		pContext->IAGetPrimitiveTopology(&primitiveTopology);
		pContext->RSGetState(&rasterState);
		pContext->PSGetSamplers(0, 1, &samplerStatePS0);//Need check
		pContext->PSGetShaderResources(0, 1, &resourceViewPS0);
		pContext->IAGetVertexBuffers(0, 1, &vb0, &stridesVB0, &offsetVB0);
		pContext->VSGetShader(&vertexShader, vsClassInstances, &vsNClassInstances);//Need check
#pragma endregion
#pragma region 用SpriteBatch绘制
		spriteBatch->Begin();
		auto v = spriteFont->MeasureString(display_text);
		DirectX::XMFLOAT2 textanchorpos = { textanchorpos_x * DirectX::XMVectorGetX(v),textanchorpos_y * DirectX::XMVectorGetY(v) };
		spriteFont->DrawString(spriteBatch.get(), display_text, calcShadowPos, calcShadowColor, 0.0f, textanchorpos);
		spriteFont->DrawString(spriteBatch.get(), display_text, textpos, calcColor, 0.0f, textanchorpos);
		spriteBatch->End();
#pragma endregion
#pragma region 恢复原来的状态
		pContext->OMSetBlendState(blendState, blendFactor, sampleMask);
		pContext->VSSetSamplers(0, 1, &samplerStateVS0);
		pContext->OMSetDepthStencilState(depthStencilState, stencilRef);
		pContext->IASetIndexBuffer(indexBuffer, indexBufferFormat, indexBufferOffset);
		pContext->IASetInputLayout(inputLayout);
		pContext->PSSetShader(pixelShader, psClassInstances, psNClassInstances);
		pContext->IASetPrimitiveTopology(primitiveTopology);
		pContext->RSSetState(rasterState);
		pContext->PSSetSamplers(0, 1, &samplerStatePS0);
		pContext->PSSetShaderResources(0, 1, &resourceViewPS0);
		pContext->IASetVertexBuffers(0, 1, &vb0, &stridesVB0, &offsetVB0);
		pContext->VSSetShader(vertexShader, vsClassInstances, vsNClassInstances);
#pragma endregion
	}
};

static std::map<IDXGISwapChain*, D2DCustomPresent> cp;

void CustomPresent(IDXGISwapChain* pSC)
{
	if (cp.find(pSC) == cp.end())
	{
		cp.insert(std::make_pair(pSC, D2DCustomPresent()));
		cp[pSC].Init(pSC);
	}
	cp[pSC].Draw();
}

void CustomResizeBuffers(IDXGISwapChain* p, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	if (cp.find(p) != cp.end())
		cp[p].CalcRect(p, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}
