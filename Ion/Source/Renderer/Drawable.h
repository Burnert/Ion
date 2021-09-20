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
		virtual const VertexBuffer* GetVertexBufferRaw() const = 0;
		virtual const IndexBuffer* GetIndexBufferRaw() const = 0;
		virtual const Material* GetMaterialRaw() const = 0;
		virtual const FMatrix4& GetTransformMatrix() const = 0;
	};
}
