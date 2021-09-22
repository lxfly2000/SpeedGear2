#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // 包含glad来获取所有的必须OpenGL头文件

class Shader
{
public:
	// 程序ID
	unsigned int ID;

	// 构造器读取并构建着色器
	void Init(const GLchar * vShaderCode, const GLchar * fShaderCode);
	// 使用/激活程序
	void use();
	// uniform工具函数
	void setBool(const char *name, bool value) const;
	void setInt(const char *name, int value) const;
	void setFloat(const char *name, float value) const;
};

#endif
