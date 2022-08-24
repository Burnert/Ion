#pragma once

#include "RHICore.h"
#include "VertexAttribute.h"
#include "VertexLayout.h"

namespace Ion
{
	class RHIShader;

	class ION_API RHIVertexBuffer : public RefCountable
	{
	public:
		static TRef<RHIVertexBuffer> Create(float* vertexAttributes, uint64 count);

		virtual ~RHIVertexBuffer() { }

		virtual void SetLayout(const std::shared_ptr<RHIVertexLayout>& layout) = 0;
		virtual Result<void, RHIError> SetLayoutShader(const std::shared_ptr<RHIShader>& shader) = 0;

		virtual uint32 GetVertexCount() const = 0;

	protected:
		RHIVertexBuffer() { }

		virtual Result<void, RHIError> Bind() const = 0;
		virtual Result<void, RHIError> BindLayout() const = 0;
		virtual Result<void, RHIError> Unbind() const = 0;

		friend class Renderer;
	};
}
