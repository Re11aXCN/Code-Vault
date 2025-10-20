#pragma once

/// <summary>
/// 管理顶点缓冲区对象（VertexBuffer）
/// </summary>

class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
private:
	unsigned int m_RendererID;
};