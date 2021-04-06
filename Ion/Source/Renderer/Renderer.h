#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

namespace Ion
{
	class ION_API Renderer
	{
	public:
		static TShared<Renderer> Create();

		virtual ~Renderer() { };

		virtual void Init() = 0;

		virtual void Clear() const = 0;
		virtual void Clear(const FVector4& color) const = 0;

		virtual void SetVSyncEnabled(bool bEnabled) const = 0;

	protected:
		Renderer() { }
	};
}
