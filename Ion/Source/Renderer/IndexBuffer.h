#pragma once

#include "Drawable.h"

namespace Ion
{
	class ION_API IndexBuffer
	{
	public:
		static TShared<IndexBuffer> Create(uint* indices, uint count);

		virtual ~IndexBuffer() { }

		virtual uint GetIndexCount() const = 0;
		virtual uint GetTriangleCount() const = 0;

	protected:
		IndexBuffer() { }

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};
}
