#pragma once

#include "RHICore.h"

namespace Ion
{
	class ION_API RHIIndexBuffer
	{
	public:
		static RHIIndexBuffer* Create(uint32* indices, uint32 count);
		static TShared<RHIIndexBuffer> CreateShared(uint32* indices, uint32 count);

		virtual ~RHIIndexBuffer() { }

		virtual uint32 GetIndexCount() const = 0;
		virtual uint32 GetTriangleCount() const = 0;

	protected:
		RHIIndexBuffer() { }

		virtual Result<void, RHIError> Bind() const = 0;
		virtual Result<void, RHIError> Unbind() const = 0;

		friend class Renderer;
	};
}
