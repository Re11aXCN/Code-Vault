#include "Shader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"

Shader::Shader(const std::string& filepath)
	:m_FilePath(filepath), m_RendererID(0)
{
	/* 从文件中解析着色器源码 */
	ShaderProgramSource source = ParseShader(filepath);
	/* 创建着色器程序 */
	m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
	GLCall(glDeleteProgram(m_RendererID));
}

void Shader::Bind() const
{
	GLCall(glUseProgram(m_RendererID));	// 使用着色器程序对象
}

void Shader::Unbind() const
{
	GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int value)
{
	GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1f(const std::string& name, float value)
{
	GLCall(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetUniform1iv(const std::string& name, int count, int* value)
{
	GLCall(glUniform1iv(GetUniformLocation(name), count, value));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{

	GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3)); /* 设置对应的统一变量 */
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
	// 这里只提供一个矩阵我们用的是glm,它存储了它的矩阵
	GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

ShaderProgramSource Shader::ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);/* 这里没判断文件是否能正常打开 is_open */

	enum class ShaderType
	{
		NONE = -1,     // 无类型（初始值）  
		VERTEX = 0,    // 顶点着色器  
		FRAGMENT = 1   // 片段着色器  
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)/* 找到#shader标记 */
		{
			if (line.find("vertex") != std::string::npos)/* 顶点着色器标记 */
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)/* 片段着色器标记 */
				type = ShaderType::FRAGMENT;
		}
		else
		{
			ss[(int)type] << line << "\n";
		}
	}
	return { ss[0].str(), ss[1].str() };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id;
	GLCall(id = glCreateShader(type));
	const char* src = source.c_str(); // 也可以使用 &source[0]  
	GLCall(glShaderSource(id, 1, &src, nullptr));/* 设置着色器源码 */
	GLCall(glCompileShader(id));

	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));

		// char message[length];不允许不确定的值在栈内申请内存，换个思维
		char* message = (char*)_malloca(length * sizeof(char));

		GLCall(glGetShaderInfoLog(id, length, &length, message));

		std::cout << "Failed to compile shader!"
			<< (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< "shader!\n";
		std::cout << message << std::endl;

		GLCall(glDeleteShader(id));

		return 0;
	}

	return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program;
	GLCall(program = glCreateProgram()); // 创建着色器程序对象

	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader); // 编译顶点着色器
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader); // 编译片段着色器

	// 将着色器附加到程序对象上
	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program)); // 链接程序
	GLCall(glValidateProgram(program)); // 验证程序

	GLCall(glDeleteShader(vs)); // 删除顶点着色器对象
	GLCall(glDeleteShader(fs)); // 删除片段着色器对象

	return program; // 返回创建的着色器程序对象 ID
}


int Shader::GetUniformLocation(const std::string& name) const
{
	// 如果已经缓存了 uniform 变量的位置，则直接返回
	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
		return m_UniformLocationCache[name];
	}

	int location;
	GLCall(location = glGetUniformLocation(m_RendererID, name.c_str())); // 获取 uniform 变量的位置

	// 如果位置为 -1，则说明 uniform 变量不存在
	if (location == -1) {
		std::cout << "Warning: uniform '" << name << "' doesn't exist" << std::endl;
	}

	// 将位置缓存到 map 中
	m_UniformLocationCache[name] = location;
	return location; // 返回 uniform 变量的位置
}

