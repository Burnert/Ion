#pragma once

#include "Core/CoreTypes.h"
#include "Core/CoreUtility.h"

namespace Ion
{
	class VertexBuffer;
	class IndexBuffer;
	class Material;

	class IDrawable
	{
	public:
		virtual const TShared<VertexBuffer>& GetVertexBuffer() const = 0;
		virtual const TShared<IndexBuffer>& GetIndexBuffer() const = 0;
		virtual const TShared<Material>& GetMaterial() const = 0;
		virtual const FMatrix4& GetTransformMatrix() const = 0;
	};
}
