#pragma once

#include "Drawable.h"

namespace Ion
{
	class ION_API IndexBuffer
	{
	public:
		static TShared<IndexBuffer> Create(uint32* indices, uint32 count);

		virtual ~IndexBuffer() { }

		virtual uint32 GetIndexCount() const = 0;
		virtual uint32 GetTriangleCount() const = 0;

	protected:
		IndexBuffer() { }

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};
}
