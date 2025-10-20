#pragma once

#include "VertexBuffer.h"

/// <summary>
/// VertexArray 类实现了管理顶点数组对象的功能，包括生成、绑定、解绑和添加顶点缓冲区数据的操作
/// </summary>

class VertexBufferLayout;

class VertexArray
{
private:
	unsigned int m_RendererID;
public:
	VertexArray();
	~VertexArray();

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

	void Bind() const;
	void Unbind() const;
};