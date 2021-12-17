#pragma once

#include "VertexAttribute.h"
#include "VertexLayout.h"

namespace Ion
{
	class Shader;

	class ION_API VertexBuffer
	{
	public:
		static TShared<VertexBuffer> Create(float* vertexAttributes, uint64 count);

		virtual ~VertexBuffer() { }

		virtual void SetLayout(const TShared<VertexLayout>& layout, const TShared<Shader>& shader) = 0;

		virtual uint32 GetVertexCount() const = 0;

	protected:
		VertexBuffer() { }

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};
}
