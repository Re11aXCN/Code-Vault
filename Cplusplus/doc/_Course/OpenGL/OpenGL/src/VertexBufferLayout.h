#pragma once

#include <vector>
#include <GL/glew.h>
#include "Renderer.h"

/// <summary>
/// 灵活的顶点缓冲区布局管理类，通过模板方法 Push 可以添加不同数据类型的顶点数据，
/// 并计算出整个布局的步长，以便正确配置顶点属性指针（在 VertexArray::AddBuffer 中使用）并与顶点数组对象关联。
/// </summary>
struct VertexBufferElement
{
	unsigned int type;          // 数据类型（如 GL_FLOAT, GL_UNSIGNED_INT）
	unsigned int count;         // 数据个数
	unsigned char normalized;   // 是否标准化

	static unsigned int GetSizeOfType(unsigned int type)
	{
		switch (type)
		{
		case GL_FLOAT:          return 4;  // float 类型占4个字节
		case GL_UNSIGNED_INT:   return 4;  // unsigned int 类型占4个字节
		case GL_UNSIGNED_BYTE:  return 1;  // unsigned byte 类型占1个字节
		}
		ASSERT(false);  // 如果出现未知类型，触发断言
		return 0;
	}
};


class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_Elements;  // 顶点缓冲区元素列表
	unsigned int m_Stride;                       // 步长（总字节数）

public:
	VertexBufferLayout() : m_Stride(0) {}

	// 模板方法，用于向顶点缓冲区布局添加数据类型和数量
	template<typename T>
	void Push(unsigned int count);

	// Push<float> 特化
	template<>
	void Push<float>(unsigned int count)
	{
		m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_Stride += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count;
	}

	// Push<unsigned int> 特化
	template<>
	void Push<unsigned int>(unsigned int count)
	{
		m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_Stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
	}

	// Push<unsigned char> 特化
	template<>
	void Push<unsigned char>(unsigned int count)
	{
		m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_Stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE) * count;
	}

	/*
	在类定义中的成员函数：类内部定义的成员函数如果被标记为 inline，意味着编译器可以选择在
	每个使用它的地方将其直接展开，而不是在每个编译单元中生成单独的函数副本。这样做可以减少
	函数调用的开销，并且有助于提高代码的执行效率。

	在全局变量或函数：全局变量或函数如果被标记为 inline，则表示编译器会尝试将其放在每个
	引用它的文件中的位置展开，以避免多次定义的冲突。
	*/
	// 获取顶点缓冲区元素列表
	inline const std::vector<VertexBufferElement> GetElements() const { return m_Elements; }

	// 获取步长
	inline unsigned int GetStride() const { return m_Stride; }
};
