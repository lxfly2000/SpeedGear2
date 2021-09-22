#pragma once

#include<glad/glad.h>
#include<map>
#include<glm/glm.hpp>
#include<string>
#include"Shader.h"

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	int Advance;    // Horizontal offset to advance to next glyph
};

class FTDraw
{
private:
	std::map<wchar_t, Character> Characters;
	GLuint ftVAO, ftVBO;
	glm::mat4 projection;
	Shader shader;
	int m_width, m_height;
	int m_face_line_spacing;

public:
	void Init(int width,int height,const char *fontfilename, long fontfaceindex, unsigned int fontsizeh, const wchar_t *usingChars);
	void ResizeWindow(int width, int height);
	void RenderText(const std::string &text, GLfloat x, GLfloat y, GLfloat anchor_x_factor, GLfloat anchor_y_factor, GLfloat scale, const glm::vec4 &color);
	void CalcDrawRect(const std::string &text, GLfloat scale, GLfloat* width, GLfloat* height);
};