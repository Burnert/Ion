#pragma once

namespace Ion
{
	class ION_API RHIIndexBuffer
	{
	public:
		static TShared<RHIIndexBuffer> Create(uint32* indices, uint32 count);

		virtual ~RHIIndexBuffer() { }

		virtual uint32 GetIndexCount() const = 0;
		virtual uint32 GetTriangleCount() const = 0;

	protected:
		RHIIndexBuffer() { }

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		friend class Renderer;
	};
}
