#pragma once

namespace Ion
{
	class ION_API IndexBuffer
	{
	public:
		static TShared<IndexBuffer> Create(uint* indices, uint count);

		virtual ~IndexBuffer() { }

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

	protected:
		IndexBuffer() { }
	};
}
