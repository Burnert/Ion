#pragma once

#include "VertexAttribute.h"
#include "VertexLayout.h"

namespace Ion
{
	class RHIShader;

	class ION_API RHIVertexBuffer
	{
	public:
		static TShared<RHIVertexBuffer> Create(float* vertexAttributes, uint64 count);

		virtual ~RHIVertexBuffer() { }

		virtual void SetLayout(const TShared<RHIVertexLayout>& layout) = 0;
		virtual void SetLayoutShader(const TShared<RHIShader>& shader) = 0;

		virtual uint32 GetVertexCount() const = 0;

	protected:
		RHIVertexBuffer() { }

		virtual void Bind() const = 0;
		virtual void BindLayout() const = 0;
		virtual void Unbind() const = 0;

		friend class Renderer;
	};
}
