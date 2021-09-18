//使用微软的方法创建材质图片，字体巨TM繁琐，这个加载器类用于简化这些过程。

#include "ResLoader.h"
#include <fstream>
#include <wincodec.h>
#include <Shlwapi.h>
#include "..\DirectXTK\Inc\WICTextureLoader.h"
#include "..\DirectXTK\Inc\ScreenGrab.h"
#include "..\DirectXTex\DirectXTex\DirectXTex.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"windowscodecs.lib")

#define MAX_TEXTURE_DIMENSION 8192//参考：https://msdn.microsoft.com/library/windows/desktop/ff476876.aspx

HRESULT _check_hr = S_OK;
#define C(sth) _check_hr=sth;if(FAILED(_check_hr))return _check_hr

using Microsoft::WRL::ComPtr;
using namespace DirectX;

int PointToDip(int pointsize)
{
	//https://www.codeproject.com/articles/376597/outline-text-with-directwrite#source
	//原则上第二个参数应为GetDeviceCaps(GetDC(NULL), LOGPIXELSX或LOGPIXELSY),但此处是用于游戏，且游戏通常不考虑DPI，因此直接指定96
	return MulDiv(pointsize, USER_DEFAULT_SCREEN_DPI, 72);
}

float PointToDip(float pointsize)
{
	return pointsize * USER_DEFAULT_SCREEN_DPI / 72.0f;
}

HRESULT LoadTextureFromFile(ID3D11Device* device, LPWSTR fpath, ID3D11ShaderResourceView** pTex, int* pw, int* ph, bool convertpmalpha)
{
	ComPtr<ID3D11Resource> tmpRes;
	if (convertpmalpha)
	{
		ScratchImage image, pmaimage;
		Blob pmaimageres;
		C(LoadFromWICFile(fpath, WIC_FLAGS_NONE, NULL, image));
		C(PremultiplyAlpha(image.GetImages(), image.GetImageCount(), image.GetMetadata(), TEX_PMALPHA_DEFAULT, pmaimage));
		C(SaveToWICMemory(pmaimage.GetImages(), pmaimage.GetImageCount(), WIC_FLAGS_NONE, GUID_ContainerFormatPng,
			pmaimageres));
		C(CreateWICTextureFromMemory(device, (BYTE*)pmaimageres.GetBufferPointer(), pmaimageres.GetBufferSize(),
			&tmpRes, pTex));
	}
	else
	{
		C(CreateWICTextureFromFile(device, fpath, &tmpRes, pTex));
	}
	ComPtr<ID3D11Texture2D> tmpTex2D;
	C(tmpRes.As(&tmpTex2D));
	CD3D11_TEXTURE2D_DESC tmpDesc;
	tmpTex2D->GetDesc(&tmpDesc);
	if (pw)*pw = tmpDesc.Width;
	if (ph)*ph = tmpDesc.Height;
	return S_OK;
}

//COM函数，保存图像至文件
HRESULT SaveWicBitmapToFile(IWICBitmap* wicbitmap, LPCWSTR path)
{
	//http://blog.csdn.net/augusdi/article/details/9051723
	IWICImagingFactory* wicfactory;
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(void**)&wicfactory));
	IWICStream* wicstream;
	C(wicfactory->CreateStream(&wicstream));
	C(wicstream->InitializeFromFilename(path, GENERIC_WRITE));
	IWICBitmapEncoder* benc;
	IWICBitmapFrameEncode* bfenc;
	C(wicfactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &benc));
	C(benc->Initialize(wicstream, WICBitmapEncoderNoCache));
	C(benc->CreateNewFrame(&bfenc, NULL));
	C(bfenc->Initialize(NULL));
	UINT w, h;
	C(wicbitmap->GetSize(&w, &h));
	C(bfenc->SetSize(w, h));
	WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
	C(bfenc->SetPixelFormat(&format));
	C(bfenc->WriteSource(wicbitmap, NULL));
	C(bfenc->Commit());
	C(benc->Commit());
	benc->Release();
	bfenc->Release();
	wicstream->Release();
	wicfactory->Release();
	return S_OK;
}

//COM函数，保存图像至内存，需要手动释放(delete)
HRESULT SaveWicBitmapToMemory(IWICBitmap* wicbitmap, std::unique_ptr<BYTE>& outMem, size_t* pbytes)
{
	//http://blog.csdn.net/augusdi/article/details/9051723
	IWICImagingFactory* wicfactory;
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(void**)&wicfactory));
	IWICStream* wicstream;
	IStream* memstream = SHCreateMemStream(NULL, 0);
	if (!memstream)return -1;
	C(wicfactory->CreateStream(&wicstream));
	C(wicstream->InitializeFromIStream(memstream));
	IWICBitmapEncoder* benc;
	IWICBitmapFrameEncode* bfenc;
	C(wicfactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &benc));
	C(benc->Initialize(wicstream, WICBitmapEncoderNoCache));
	C(benc->CreateNewFrame(&bfenc, NULL));
	C(bfenc->Initialize(NULL));
	UINT w, h;
	C(wicbitmap->GetSize(&w, &h));
	C(bfenc->SetSize(w, h));
	WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;
	C(bfenc->SetPixelFormat(&format));
	C(bfenc->WriteSource(wicbitmap, NULL));
	C(bfenc->Commit());
	C(benc->Commit());

	ULARGE_INTEGER ulsize;
	C(wicstream->Seek({ 0,0 }, STREAM_SEEK_SET, NULL));
	C(IStream_Size(wicstream, &ulsize));
	*pbytes = (size_t)ulsize.QuadPart;
	outMem.reset(new BYTE[*pbytes]);
	ULONG readbytes;
	C(wicstream->Read(outMem.get(), (ULONG)ulsize.QuadPart, &readbytes));
	if (readbytes != ulsize.QuadPart)
		return -1;
	benc->Release();
	bfenc->Release();
	wicstream->Release();
	memstream->Release();
	wicfactory->Release();
	return S_OK;
}

HRESULT LoadWicBitmapFromMemory(ComPtr<IWICBitmap>& outbitmap, BYTE* mem, size_t memsize)
{
	IWICImagingFactory* wicfactory;
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(void**)&wicfactory));
	IStream* memstream = SHCreateMemStream(mem, (UINT)memsize);
	IWICBitmapDecoder* wicimgdecoder;
	C(wicfactory->CreateDecoderFromStream(memstream, NULL, WICDecodeMetadataCacheOnDemand, &wicimgdecoder));
	IWICFormatConverter* wicimgsource;
	C(wicfactory->CreateFormatConverter(&wicimgsource));
	IWICBitmapFrameDecode* picframe;
	C(wicimgdecoder->GetFrame(0, &picframe));
	C(wicimgsource->Initialize(picframe, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f,
		WICBitmapPaletteTypeCustom));
	C(wicfactory->CreateBitmapFromSource(wicimgsource, WICBitmapNoCache, outbitmap.ReleaseAndGetAddressOf()));
	C(picframe->Release());
	C(wicimgsource->Release());
	C(wicimgdecoder->Release());
	C(memstream->Release());
	C(wicfactory->Release());
	return S_OK;
}

int ReadFileToMemory(const char* pfilename, std::unique_ptr<char>& memout, size_t* memsize, bool removebom)
{
	std::ifstream fin(pfilename, std::ios::in | std::ios::binary);
	if (!fin)return -1;
	int startpos = 0;
	if (removebom)
	{
		char bom[3] = "";
		fin.read(bom, sizeof bom);
		if (strncmp(bom, "\xff\xfe", 2) == 0)startpos = 2;
		if (strncmp(bom, "\xfe\xff", 2) == 0)startpos = 2;
		if (strncmp(bom, "\xef\xbb\xbf", 3) == 0)startpos = 3;
	}
	fin.seekg(0, std::ios::end);
	*memsize = (int)fin.tellg() - startpos;
	memout.reset(new char[*memsize]);
	fin.seekg(startpos, std::ios::beg);
	fin.read(memout.get(), *memsize);
	return 0;
}

HRESULT WicBitmapConvertPremultiplyAlpha(IWICBitmap* wicbitmap, ComPtr<IWICBitmap>& outbitmap, bool pmalpha)
{
	std::unique_ptr<uint8_t> mem;
	size_t memsize;
	C(SaveWicBitmapToMemory(wicbitmap, mem, &memsize));

	ScratchImage imgold, imgnew;
	Blob imgres;
	C(LoadFromWICMemory(mem.get(), memsize, WIC_FLAGS_NONE, NULL, imgold));
	C(PremultiplyAlpha(imgold.GetImages(), imgold.GetImageCount(), imgold.GetMetadata(),
		pmalpha ? TEX_PMALPHA_DEFAULT : TEX_PMALPHA_REVERSE, imgnew));
	C(SaveToWICMemory(imgnew.GetImages(), imgnew.GetImageCount(), WIC_FLAGS_NONE, GUID_ContainerFormatPng, imgres));
	C(LoadWicBitmapFromMemory(outbitmap, (BYTE*)imgres.GetBufferPointer(), imgres.GetBufferSize()));
	return S_OK;
}

//COM函数
HRESULT LoadFontFromSystem(ID3D11Device* device, std::unique_ptr<SpriteFont>& outSF, unsigned textureWidth, unsigned textureHeight,
	LPWSTR fontName, float fontSize, const D2D1_COLOR_F& fontColor, DWRITE_FONT_WEIGHT fontWeight,
	wchar_t* pszCharacters, float pxBetweenChar, bool convertpmalpha)
{
	//这个字体加载真心是恶心得要死，以前有D3DX的时候还好点，到现在微软直接把字体支持完全给抛弃了！
	ID2D1Factory* d2dfactory;
	IWICImagingFactory* wicfactory;
	IWICBitmap* fontBitmap;
	ID2D1RenderTarget* fontRT;
	ID2D1SolidColorBrush* fontColorBrush;
	IDWriteFactory* dwfactory;
	IDWriteTextFormat* textformat;
	IDWriteTextLayout* textlayout;
	std::vector<SpriteFont::Glyph>glyphs;
	//创建D2D
	C(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dfactory));
	//创建图像
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(void**)&wicfactory));
	if (textureWidth > MAX_TEXTURE_DIMENSION || textureHeight > MAX_TEXTURE_DIMENSION)return -1;
	C(wicfactory->CreateBitmap(textureWidth, textureHeight, GUID_WICPixelFormat32bppPBGRA,//PBGRA中的P表示预乘透明度
		WICBitmapCacheOnLoad, &fontBitmap));
	//创建图像的RT
	C(d2dfactory->CreateWicBitmapRenderTarget(fontBitmap, D2D1::RenderTargetProperties(), &fontRT));
	//创建字体
	C(fontRT->CreateSolidColorBrush(fontColor, &fontColorBrush));
	C(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwfactory), (IUnknown**)&dwfactory));
	C(dwfactory->CreateTextFormat(fontName, NULL, fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		PointToDip(fontSize), L"", &textformat));
	C(textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
	C(textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
	//绘制
	WCHAR str[2];
	ZeroMemory(str, sizeof str);
	int curx = 0, cury = 0;
	DWRITE_TEXT_METRICS textMet;
	DWRITE_OVERHANG_METRICS ovhMet;
	D2D_POINT_2F drawPos = { 0.0f,0.0f };
	SpriteFont::Glyph eachglyph;
	ZeroMemory(&eachglyph, sizeof eachglyph);
	float chHeight = 0.0f;//每行最大字符高度
	wchar_t* usingCharacters;
	int usingChSize;
	if (pszCharacters)
	{
		usingCharacters = pszCharacters;
		usingChSize = lstrlen(usingCharacters);
	}
	else
	{
		usingChSize = 128 - ' ';
		usingCharacters = new wchar_t[usingChSize];
		for (int i = 0; i < usingChSize; i++)
			usingCharacters[i] = ' ' + i;
	}
	fontRT->BeginDraw();
	for (int i = 0; i < usingChSize; i++)
	{
		eachglyph.Character = usingCharacters[i];
		str[0] = usingCharacters[i];//DirectXTK不支持代理对，因此把字符直接当成16位数处理就行了。
		C(dwfactory->CreateTextLayout(str, lstrlen(str), textformat,
			(float)textureWidth - drawPos.x, (float)textureHeight - drawPos.y, &textlayout));
		C(textlayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
		C(textlayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

		C(textlayout->GetMetrics(&textMet));
		C(textlayout->GetOverhangMetrics(&ovhMet));//上下采用排版间距，左右0间距
#pragma region 浮点数取整
		textMet.height = std::roundf(textMet.height);
		ovhMet.left = std::ceilf(ovhMet.left);
		ovhMet.right = std::ceilf(ovhMet.right);
#pragma endregion

		if (drawPos.x + textMet.widthIncludingTrailingWhitespace + pxBetweenChar > textureWidth)
		{
			drawPos.x = 0;
			drawPos.y += chHeight + pxBetweenChar;
			chHeight = 0.0f;
		}
		chHeight = max(chHeight, textMet.height);
		if (drawPos.y + chHeight > textureHeight)return -1;
		eachglyph.Subrect.left = std::lround(drawPos.x);
		eachglyph.Subrect.top = std::lround(drawPos.y);
		//可绘制彩色文字（绘制彩色文字功能要求系统为Win8.1/10，在Win7中会使D2D区域无法显示）
		fontRT->DrawText(str, lstrlen(str), textformat, D2D1::RectF(drawPos.x + ovhMet.left, drawPos.y,
			(float)textureWidth, (float)textureHeight), fontColorBrush);
		float chPxWidth = textlayout->GetMaxWidth() + ovhMet.left + ovhMet.right;
		drawPos.x += chPxWidth;
		eachglyph.Subrect.right = std::lround(drawPos.x);
		drawPos.x += pxBetweenChar;
		eachglyph.Subrect.bottom = std::lround(drawPos.y + textMet.height);
		eachglyph.XOffset = -ovhMet.left;
		eachglyph.XAdvance = textMet.widthIncludingTrailingWhitespace - chPxWidth + ovhMet.left;
#ifdef _DEBUG
		if (eachglyph.Character >= '0' && eachglyph.Character <= '9')
		{
			WCHAR info[40];
			swprintf_s(info, L"字符\'%c\'的宽度应是：%f，实际是：%f\n", eachglyph.Character,
				textMet.widthIncludingTrailingWhitespace,
				eachglyph.XOffset + eachglyph.Subrect.right - eachglyph.Subrect.left + eachglyph.XAdvance);
			OutputDebugString(info);
		}
#endif

		glyphs.push_back(eachglyph);
		textlayout->Release();
	}
	fontRT->EndDraw();
	if (!pszCharacters)
		delete usingCharacters;
	//由图像生成材质
	std::unique_ptr<BYTE> membitmap;
	size_t memsize;
	C(SaveWicBitmapToMemory(fontBitmap, membitmap, &memsize));
#ifdef _DEBUG
	WCHAR _debug_font_filename[MAX_PATH];
	wsprintf(_debug_font_filename, L"%s_%d_%02X%02X%02X%02X.png", fontName, (int)fontSize,
		(int)(fontColor.r * 255), (int)(fontColor.g * 255), (int)(fontColor.b * 255), (int)(fontColor.a * 255));
	C(SaveWicBitmapToFile(fontBitmap, _debug_font_filename));
#endif
	ID3D11Resource* fontTexture;
	ID3D11ShaderResourceView* fontTextureView;
	if (convertpmalpha)
	{
		ScratchImage teximage, pmteximage;
		Blob pmteximageres;
		C(LoadFromWICMemory(membitmap.get(), memsize, WIC_FLAGS_NONE, NULL, teximage));
		C(PremultiplyAlpha(teximage.GetImages(), teximage.GetImageCount(), teximage.GetMetadata(), TEX_PMALPHA_DEFAULT, pmteximage));
		C(SaveToWICMemory(pmteximage.GetImages(), pmteximage.GetImageCount(), WIC_FLAGS_NONE, GUID_ContainerFormatPng,
			pmteximageres));
		C(CreateWICTextureFromMemory(device, (BYTE*)pmteximageres.GetBufferPointer(),
			pmteximageres.GetBufferSize(), &fontTexture, &fontTextureView));
	}
	else
	{
		C(CreateWICTextureFromMemory(device, membitmap.get(), memsize, &fontTexture, &fontTextureView));
	}
	//创建SpriteFont
	outSF.reset(new SpriteFont(fontTextureView, glyphs.data(), glyphs.size(), chHeight));
	outSF.get()->SetDefaultCharacter('?');
	//释放
	fontTextureView->Release();
	fontTexture->Release();
	textformat->Release();
	dwfactory->Release();
	fontColorBrush->Release();
	fontRT->Release();
	fontBitmap->Release();
	wicfactory->Release();
	d2dfactory->Release();
	return S_OK;
}

HRESULT DrawTextToTexture(ID3D11Device* device, LPWSTR text, ID3D11ShaderResourceView** pTex, int* pw, int* ph,
	LPWSTR fontName, float fontSize, const D2D1_COLOR_F& fontColor, DWRITE_FONT_WEIGHT fontWeight, bool convertpmalpha)
{
	ID2D1Factory* d2dfactory;
	IWICImagingFactory* wicfactory;
	IWICBitmap* fontBitmap;
	ID2D1RenderTarget* fontRT;
	ID2D1SolidColorBrush* fontColorBrush;
	IDWriteFactory* dwfactory;
	IDWriteTextFormat* textformat;
	IDWriteTextLayout* textlayout;
	//创建字体
	C(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwfactory), (IUnknown**)&dwfactory));
	C(dwfactory->CreateTextFormat(fontName, NULL, fontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		PointToDip(fontSize), L"", &textformat));
	C(textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
	C(textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));
	//计算绘制文字大小
	DWRITE_TEXT_METRICS textMet;
	C(dwfactory->CreateTextLayout(text, lstrlen(text), textformat, (float)MAX_TEXTURE_DIMENSION, (float)MAX_TEXTURE_DIMENSION, &textlayout));
	C(textlayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
	C(textlayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

	C(textlayout->GetMetrics(&textMet));
	//创建D2D
	C(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dfactory));
	//创建图像
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&wicfactory));
	C(wicfactory->CreateBitmap((UINT)ceilf(textMet.width), (UINT)ceilf(textMet.height), GUID_WICPixelFormat32bppPBGRA,
		WICBitmapCacheOnLoad, &fontBitmap));
	//创建图像的RT
	C(d2dfactory->CreateWicBitmapRenderTarget(fontBitmap, D2D1::RenderTargetProperties(), &fontRT));
	//绘制
	C(fontRT->CreateSolidColorBrush(fontColor, &fontColorBrush));
	fontRT->BeginDraw();

	//可绘制彩色文字（绘制彩色文字功能要求系统为Win8.1/10，在Win7中会使D2D区域无法显示）
	fontRT->DrawText(text, lstrlen(text), textformat, D2D1::RectF(0.0f, 0.0f, textMet.width, textMet.height), fontColorBrush);
	C(fontRT->EndDraw());
	WICRect wicrect{ 0,0,(int)textMet.width,(int)textMet.height };
	//由图像生成材质
	std::unique_ptr<BYTE>membitmap;
	size_t memsize;
	C(SaveWicBitmapToMemory(fontBitmap, membitmap, &memsize));

	ComPtr<ID3D11Resource>fontTexture;
	if (convertpmalpha)
	{
		ScratchImage teximage, pmteximage;
		Blob pmteximageres;
		C(LoadFromWICMemory(membitmap.get(), memsize, WIC_FLAGS_NONE, NULL, teximage));
		C(PremultiplyAlpha(teximage.GetImages(), teximage.GetImageCount(), teximage.GetMetadata(), TEX_PMALPHA_DEFAULT, pmteximage));
		C(SaveToWICMemory(pmteximage.GetImages(), pmteximage.GetImageCount(), WIC_FLAGS_NONE, GUID_ContainerFormatPng,
			pmteximageres));
		C(CreateWICTextureFromMemory(device, (BYTE*)pmteximageres.GetBufferPointer(),
			pmteximageres.GetBufferSize(), &fontTexture, pTex));
	}
	else
	{
		C(CreateWICTextureFromMemory(device, membitmap.get(), memsize, &fontTexture, pTex));
	}
	ComPtr<ID3D11Texture2D>tmpTex2D;
	C(fontTexture.As(&tmpTex2D));
	CD3D11_TEXTURE2D_DESC tmpDesc;
	tmpTex2D->GetDesc(&tmpDesc);
	if (pw)
		*pw = tmpDesc.Width;
	if (ph)
		*ph = tmpDesc.Height;
	//释放
	textformat->Release();
	dwfactory->Release();
	fontColorBrush->Release();
	fontRT->Release();
	fontBitmap->Release();
	wicfactory->Release();
	d2dfactory->Release();
	return S_OK;
}

HRESULT TakeScreenShotToFile(ID3D11Device* device, IDXGISwapChain* swapChain, LPWSTR fpath)
{
	//https://github.com/Microsoft/DirectXTK/wiki/ScreenGrab#examples
	ComPtr<ID3D11DeviceContext> ctx;
	device->GetImmediateContext(&ctx);
	ComPtr<ID3D11Texture2D> texscreen;
	C(swapChain->GetBuffer(0, __uuidof(texscreen), (void**)texscreen.GetAddressOf()));
	return SaveWICTextureToFile(ctx.Get(), texscreen.Get(), GUID_ContainerFormatPng, fpath);
}

//COM函数
HRESULT CreateD2DImageFromFile(ComPtr<ID2D1Bitmap>& pic, ID2D1RenderTarget* prt, LPCWSTR ppath)
{
	IWICImagingFactory* wicfactory;
	C(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
		(void**)&wicfactory));
	IWICBitmapDecoder* wicimgdecoder;
	C(wicfactory->CreateDecoderFromFilename(ppath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicimgdecoder));
	IWICFormatConverter* wicimgsource;
	C(wicfactory->CreateFormatConverter(&wicimgsource));
	IWICBitmapFrameDecode* picframe;
	C(wicimgdecoder->GetFrame(0, &picframe));
	C(wicimgsource->Initialize(picframe, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f,
		WICBitmapPaletteTypeCustom));
	if (FAILED(prt->CreateBitmapFromWicBitmap(wicimgsource, NULL, pic.ReleaseAndGetAddressOf())))
		return -2;
	C(picframe->Release());
	C(wicimgsource->Release());
	C(wicimgdecoder->Release());
	C(wicfactory->Release());
	return S_OK;
}

HRESULT CreateDWTextFormat(ComPtr<IDWriteTextFormat>& textformat,
	LPCWSTR fontName, DWRITE_FONT_WEIGHT fontWeight,
	FLOAT fontSize, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontExpand, LPCWSTR localeName)
{
	ComPtr<IDWriteFactory> dwfactory;
	C(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwfactory), (IUnknown**)&dwfactory));
	C(dwfactory->CreateTextFormat(fontName, NULL, fontWeight, fontStyle, fontExpand, PointToDip(fontSize), localeName,
		textformat.ReleaseAndGetAddressOf()));
	return S_OK;
}

HRESULT CreateDWFontFace(ComPtr<IDWriteFontFace>& fontface, LPCWSTR fontName,
	DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontExpand)
{
	ComPtr<IDWriteFactory> dwfactory;
	C(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwfactory), (IUnknown**)&dwfactory));
	ComPtr<IDWriteFontCollection> fontcollection;
	C(dwfactory->GetSystemFontCollection(&fontcollection));
	UINT fontfamilypos;
	BOOL ffexists;
	C(fontcollection->FindFamilyName(fontName, &fontfamilypos, &ffexists));
	ComPtr<IDWriteFontFamily> ffamily;
	C(fontcollection->GetFontFamily(fontfamilypos, &ffamily));
	ComPtr<IDWriteFontList> flist;
	C(ffamily->GetMatchingFonts(fontWeight, fontExpand, fontStyle, &flist));
	ComPtr<IDWriteFont> dwfont;
	C(flist->GetFont(0, &dwfont));
	C(dwfont->CreateFontFace(&fontface));
	return S_OK;
}

HRESULT CreateDWFontFace(ComPtr<IDWriteFontFace>& fontface, IDWriteTextFormat* textformat)
{
	TCHAR fontName[256];
	C(textformat->GetFontFamilyName(fontName, ARRAYSIZE(fontName)));
	C(CreateDWFontFace(fontface, fontName,
		textformat->GetFontWeight(), textformat->GetFontStyle(), textformat->GetFontStretch()));
	return S_OK;
}

HRESULT CreateD2DGeometryFromText(ComPtr<ID2D1PathGeometry>& geometry, ID2D1Factory* factory,
	IDWriteFontFace* pfontface, IDWriteTextFormat* textformat, const wchar_t* text, size_t textlength)
{
	std::vector<UINT32> unicode_ui32;
	for (size_t i = 0; i < textlength; i++)
		unicode_ui32.push_back(static_cast<UINT32>(text[i]));
	std::unique_ptr<UINT16>gidcs(new UINT16[textlength]);
	C(pfontface->GetGlyphIndicesW(unicode_ui32.data(), static_cast<UINT32>(unicode_ui32.size()), gidcs.get()));
	C(factory->CreatePathGeometry(geometry.ReleaseAndGetAddressOf()));
	ID2D1GeometrySink* gs;
	C(geometry->Open(&gs));
	C(pfontface->GetGlyphRunOutline(textformat->GetFontSize(), gidcs.get(), NULL, NULL,
		static_cast<UINT32>(textlength), FALSE, FALSE, gs));
	C(gs->Close());
	gs->Release();
	return S_OK;
}

HRESULT CreateD2DLinearGradientBrush(ComPtr<ID2D1LinearGradientBrush>& lgBrush, ID2D1RenderTarget* rt,
	float startX, float startY, float endX, float endY, D2D1_COLOR_F startColor, D2D1_COLOR_F endColor)
{
	//https://msdn.microsoft.com/zh-cn/library/dd756678(v=vs.85).aspx
	ComPtr<ID2D1GradientStopCollection> gsc;
	D2D1_GRADIENT_STOP gs[2] = { {0.0f,startColor},{1.0f,endColor} };
	C(rt->CreateGradientStopCollection(gs, ARRAYSIZE(gs), gsc.ReleaseAndGetAddressOf()));
	C(rt->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(startX, startY),
		D2D1::Point2F(endX, endY)), gsc.Get(), lgBrush.ReleaseAndGetAddressOf()));
	return S_OK;
}

void D2DDrawGeometryWithOutline(ID2D1RenderTarget* rt, ID2D1Geometry* geometry,
	float x, float y, ID2D1Brush* fillBrush, ID2D1Brush* outlineBrush, float outlineWidth)
{
	rt->SetTransform(D2D1::Matrix3x2F::Translation(x, y));
	rt->FillGeometry(geometry, fillBrush);
	rt->DrawGeometry(geometry, outlineBrush, outlineWidth);
	rt->SetTransform(D2D1::IdentityMatrix());
}

HRESULT CreateD2DArc(ComPtr<ID2D1PathGeometry>& arc, ID2D1Factory* factory, float r, float startDegree, float endDegree)
{
	ComPtr<ID2D1GeometrySink> sink;
	C(factory->CreatePathGeometry(arc.ReleaseAndGetAddressOf()));
	C(arc->Open(sink.ReleaseAndGetAddressOf()));
	sink->BeginFigure(D2D1::Point2F(r * cosf(XMConvertToRadians(startDegree)),
		r * sinf(XMConvertToRadians(startDegree))), D2D1_FIGURE_BEGIN_HOLLOW);
	//为避免大小弧与同起点到点的歧义，将弧均分成两段保证都是小弧
	sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(r * cosf(XMConvertToRadians((startDegree + endDegree) / 2.0f)),
		r * sinf(XMConvertToRadians((startDegree + endDegree) / 2.0f))), D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->AddArc(D2D1::ArcSegment(D2D1::Point2F(r * cosf(XMConvertToRadians(endDegree)),
		r * sinf(XMConvertToRadians(endDegree))), D2D1::SizeF(r, r), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
	sink->EndFigure(D2D1_FIGURE_END_OPEN);
	C(sink->Close());
	return S_OK;
}

void D2DDrawPath(ID2D1RenderTarget* rt, ID2D1PathGeometry* path, float x, float y, ID2D1Brush* color, float width)
{
	rt->SetTransform(D2D1::Matrix3x2F::Translation(x, y));
	rt->DrawGeometry(path, color, width);
	rt->SetTransform(D2D1::IdentityMatrix());
}
