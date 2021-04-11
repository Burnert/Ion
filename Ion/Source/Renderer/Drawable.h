#pragma once

#include "Core/CoreTypes.h"
#include "Core/CoreUtility.h"

namespace Ion
{
	class VertexBuffer;
	class IndexBuffer;
	class Shader;

	class IDrawable
	{
	public:
		virtual TShared<VertexBuffer> GetVertexBuffer() const = 0;
		virtual TShared<IndexBuffer> GetIndexBuffer() const = 0;
		virtual TShared<Shader> GetShader() const = 0;
	};
}
