#include"custom_swapbuffers.h"
#include"ftdraw.h"
#include"..\sgshared\sgshared.h"
#include<map>
#include<ctime>

#ifdef _M_X64
#pragma comment(lib,"../freetype-windows-binaries/release static/vs2015-2019/win64/freetype.lib")
#else
#pragma comment(lib,"../freetype-windows-binaries/release static/vs2015-2019/win32/freetype.lib")
#endif

#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"glu32.lib")


class SwapBuffersDraw
{
private:
	unsigned t1, t2, fcounter;
	RECT windowrect;
	FTDraw ftdraw;
	char display_text[256];
	int current_fps;
	int shad;

	int period_frames,font_face_index;

	glm::vec4 text_color, text_shadow_color;
	float calc_text_x, calc_text_y, calc_shadow_x, calc_shadow_y;
	float anchor_x, anchor_y;
public:
	SwapBuffersDraw():t1(0),t2(0),fcounter(0),windowrect(),anchor_x(0),anchor_y(0),calc_shadow_x(0),calc_shadow_y(0),calc_text_x(0),calc_text_y(0),
		current_fps(0),display_text(),font_face_index(0),period_frames(0),shad(0),text_color(),text_shadow_color()
	{
	}
	void CalcRect(HDC hdc,int x,int y,int width,int height)
	{
		windowrect.left = x;
		windowrect.top = y;
		windowrect.right = x+width;
		windowrect.bottom = y+height;
		calc_text_x = 0;
		calc_text_y = 0;
		SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
		shad = pMem->useSystemDPI ? DPI_SCALED_VALUE(WindowFromDC(hdc), 2) : 2;
		calc_shadow_x = calc_text_x + shad;
		calc_shadow_y = calc_text_y + shad;
		ftdraw.ResizeWindow(width, height);
	}
	void Init(HDC dc)
	{
		SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
		if (pMem == NULL)
		{
			if (!SpeedGear_InitializeSharedMemory(FALSE))
				return;
			pMem = SpeedGear_GetSharedMemory();
		}
		text_color = glm::vec4((pMem->fontColor & 0xFF) / 255.0f, ((pMem->fontColor >> 8) & 0xFF) / 255.0f, ((pMem->fontColor >> 16) & 0xFF) / 255.0f, 1.0f);
		text_shadow_color = text_color;
		text_shadow_color /= 2.0f;

		GetClientRect(WindowFromDC(dc), &windowrect);
		calc_text_x = 0;
		calc_text_y = 0;
		if (pMem->statusPosition % 3 == 2)
			anchor_x = 1.0f;
		else if (pMem->statusPosition % 3 == 1)
			anchor_x = 0.5f;
		else
			anchor_x = 0.0f;
		if (pMem->statusPosition / 3 == 2)
			anchor_y = 1.0f;
		else if (pMem->statusPosition / 3 == 1)
			anchor_y = 0.5f;
		else
			anchor_y = 0.0f;
		calc_shadow_x = calc_text_x + shad;
		calc_shadow_y = calc_text_y + shad;
		char fn[MAX_PATH];
		lstrcpyA(fn, pMem->fontPath);
		char* posc = strrchr(fn, ':');
		if (posc - fn > 1)
		{
			*posc = 0;
			font_face_index = atoi(posc + 1);
		}
		ftdraw.Init(windowrect.right - windowrect.left, windowrect.bottom - windowrect.top, fn, font_face_index,
			(UINT)(pMem->useSystemDPI ? LOGICAL_UNIT_TO_PIXEL(WindowFromDC(dc), pMem->fontSize) : LOGICAL_UNIT_TO_PIXEL_96DPI(pMem->fontSize)), NULL);
		DEVMODE dm;
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
		period_frames = dm.dmDisplayFrequency;
	}

	void Draw()
	{
		if (fcounter-- == 0)
		{
			fcounter = period_frames;
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
				windowrect.right - windowrect.left, windowrect.bottom - windowrect.top, tm1.tm_hour, tm1.tm_min, tm1.tm_sec,"GL");
		}
		//https://github.com/ocornut/imgui/blob/master/examples/imgui_impl_opengl3.cpp#L142
#pragma region Backup GL state
		GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
		glActiveTexture(GL_TEXTURE0);
		GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
		GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef GL_SAMPLER_BINDING
		GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
#endif
		GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#ifdef GL_POLYGON_MODE
		GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
		GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
		GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
		GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
		GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
		GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
		GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
		GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
		GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
		GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
		GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
		GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
		GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
		bool clip_origin_lower_left = true;
#ifdef GL_CLIP_ORIGIN
		GLenum last_clip_origin; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&last_clip_origin); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
		clip_origin_lower_left = (last_clip_origin == GL_LOWER_LEFT);
#endif
#pragma endregion
		//Notice that the origin point is at bottom-left of the screen
		ftdraw.RenderText(display_text, calc_shadow_x, calc_shadow_y,anchor_x,anchor_y, 1.0f, text_shadow_color);
		ftdraw.RenderText(display_text, calc_text_x, calc_text_y,anchor_x,anchor_y, 1.0f, text_color);
#pragma region Restore modified GL state
		glUseProgram(last_program);
		glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef GL_SAMPLER_BINDING
		glBindSampler(0, last_sampler);
#endif
		glActiveTexture(last_active_texture);
		glBindVertexArray(last_vertex_array);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
		glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
		if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
		if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
		glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
		OriginalViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
		glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
#pragma endregion
	}
};

static std::map<HDC, SwapBuffersDraw>cp;

void CustomSwapBuffers(HDC pDC)
{
	if (cp.find(pDC) == cp.end())
	{
		cp.insert(std::make_pair(pDC, SwapBuffersDraw()));
		cp[pDC].Init(pDC);
	}
	cp[pDC].Draw();
}

void CustomViewport(int x, int y, int width, int height)
{
	HDC dc = wglGetCurrentDC();
	if (cp.find(dc) != cp.end())
		cp[dc].CalcRect(dc,x, y, width, height);
}