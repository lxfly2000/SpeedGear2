#pragma once
#include <dwrite.h>
#include <d2d1.h>
#include <wrl\client.h>
#include "..\DirectXTK\Inc\SpriteFont.h"

//————————D3D函数————————
//从文件加载材质图片
HRESULT LoadTextureFromFile(ID3D11Device* device, LPWSTR fpath, ID3D11ShaderResourceView** pTex, int* pw, int* ph,
	bool convertpmalpha = true);
//COM函数，加载系统中的字体到字体表，需要手动释放(delete)
//pCharacters：指定使用的字符，UTF16LE编码，如果不指定则为ASCII字符（32～127）
//注意字符串必须保持增序，必须含有替代字符（默认为“?”，可以用SetDefaultCharacter修改）。
//字体大小的单位为通常意义的字号
//pxBetweenChar：指示字符的水平+垂直间距，该参数是为了避免绘制出的字符重合而设置的，不会影响实际显示的间距。
//【注意】该字体表加载功能存在字符位置偏移的Bug，如果是ClearType看不出来但是如果字体在特定字号下是以点阵字符
//显示时就会非常明显，因此不推荐小字号字体使用。推荐使用Direct2D+DirectWrite排版系统绘制文字等矢量图形。
HRESULT LoadFontFromSystem(ID3D11Device* device, std::unique_ptr<DirectX::SpriteFont>& outSF, unsigned textureWidth,
	unsigned textureHeight, LPWSTR fontName, float fontSize, const D2D1_COLOR_F& fontColor,
	DWRITE_FONT_WEIGHT fontWeight, wchar_t* pszCharacters = NULL, float pxBetweenChar = 1.0f,
	bool convertpmalpha = true);
//字体大小的单位为通常意义的字号
HRESULT DrawTextToTexture(ID3D11Device* device, LPWSTR text, ID3D11ShaderResourceView** pTex, int* pw, int* ph,
	LPWSTR fontName, float fontSize, const D2D1_COLOR_F& fontColor,
	DWRITE_FONT_WEIGHT fontWeight, bool convertpmalpha = true);
//保存屏幕图像至文件(PNG)
HRESULT TakeScreenShotToFile(ID3D11Device* device, IDXGISwapChain* swapChain, LPWSTR fpath);

//COM函数，保存图像至文件(PNG)
HRESULT SaveWicBitmapToFile(IWICBitmap* wicbitmap, LPCWSTR path);
//COM函数，保存图像至内存(PNG)
HRESULT SaveWicBitmapToMemory(IWICBitmap* wicbitmap, std::unique_ptr<BYTE>& outMem, size_t* pbytes);
//COM函数，从内存中打开图像
HRESULT LoadWicBitmapFromMemory(Microsoft::WRL::ComPtr<IWICBitmap>& outbitmap, BYTE* mem, size_t memsize);
int ReadFileToMemory(const char* pfilename, std::unique_ptr<char>& memout, size_t* memsize,
	bool removebom = false);
//将WIC图像转换成：
//pmalpha=true:预乘透明度格式（PNG）
//pmalpha=false:直接透明度格式（PNG）
//【注意】如果有颜色分量的值为0将无法转换为直接透明度格式。
HRESULT WicBitmapConvertPremultiplyAlpha(IWICBitmap* wicbitmap,
	Microsoft::WRL::ComPtr<IWICBitmap>& outbitmap, bool pmalpha);
//————————D2D函数————————
//COM函数，需要调用CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE),需要引入库 WindowsCodecs.lib.
//提示：DXTK有原生的图像绘制功能，可使用ResLoader::LoadTextureFromFile将图片直接加载为ID3D11ShaderResourceView类型的资源。
HRESULT CreateD2DImageFromFile(Microsoft::WRL::ComPtr<ID2D1Bitmap>& pic, ID2D1RenderTarget* prt, LPCWSTR ppath);
//————————DWrite函数————————
//需要引入库 DWrite.lib.
HRESULT CreateDWTextFormat(Microsoft::WRL::ComPtr<IDWriteTextFormat>& textformat,
	LPCWSTR fontName, DWRITE_FONT_WEIGHT fontWeight,
	FLOAT fontSize, DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL,
	DWRITE_FONT_STRETCH fontExpand = DWRITE_FONT_STRETCH_NORMAL, LPCWSTR localeName = L"");
//【注意】字体名称必须保证是有效的。
HRESULT CreateDWFontFace(Microsoft::WRL::ComPtr<IDWriteFontFace>& fontface, LPCWSTR fontName,
	DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontExpand);
HRESULT CreateDWFontFace(Microsoft::WRL::ComPtr<IDWriteFontFace>& fontface, IDWriteTextFormat* textformat);
//根据指定的文字生成相应的轮廓路径，可用于文字描边以及高级颜色填充，不适合频繁调用，因此只适用于确定的文本。
//factory必须指定为与渲染目标使用相同的对象。
//【注意】该函数不支持换行符。
HRESULT CreateD2DGeometryFromText(Microsoft::WRL::ComPtr<ID2D1PathGeometry>& geometry, ID2D1Factory* factory,
	IDWriteFontFace* pfontface, IDWriteTextFormat* textformat, const wchar_t* text, size_t textlength);
//创建一个仅一次变化的渐变笔刷。
//【注意】该函数中的坐标以所绘图形的左下角为原点，向右为X轴正方向，向上为Y轴正方向。
HRESULT CreateD2DLinearGradientBrush(Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush>& lgBrush, ID2D1RenderTarget* rt,
	float startX, float startY, float endX, float endY, D2D1_COLOR_F startColor, D2D1_COLOR_F endColor);
//绘制带有轮廓的路径，必须在BeginDraw和EndDraw之间调用。完成后渲染目标的位置将移至原点。
void D2DDrawGeometryWithOutline(ID2D1RenderTarget* rt, ID2D1Geometry* geometry, float x, float y,
	ID2D1Brush* fillBrush, ID2D1Brush* outlineBrush, float outlineWidth);
//创建一个圆弧，使用角度制，水平向右为0度，顺时针为正方向。
//endDegree必须大于startDegree；factory必须指定为与渲染目标使用相同的对象。
HRESULT CreateD2DArc(Microsoft::WRL::ComPtr<ID2D1PathGeometry>& arc, ID2D1Factory* factory, float r,
	float startDegree, float endDegree);
//绘制非封闭的路径，必须在BeginDraw和EndDraw之间调用。完成后渲染目标的位置将移至原点。
void D2DDrawPath(ID2D1RenderTarget* rt, ID2D1PathGeometry* path, float x, float y, ID2D1Brush* color, float width);
