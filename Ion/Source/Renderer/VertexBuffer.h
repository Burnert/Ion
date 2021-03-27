#pragma once

namespace Ion
{
	class ION_API VertexBuffer
	{
	public:
		static Shared<VertexBuffer> Create(float* vertices, uint count);

		virtual ~VertexBuffer() { };

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

	protected:
		VertexBuffer() { }
	};
}
