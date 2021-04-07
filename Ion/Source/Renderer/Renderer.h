#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

namespace Ion
{
	struct SViewportDimensions
	{
		int X;
		int Y;
		int Width;
		int Height;
	};

	enum class EPolygonDrawMode : ubyte
	{
		Fill,
		Lines,
		Points,
	};

	class ION_API Renderer
	{
	public:
		static TShared<Renderer> Create();

		virtual ~Renderer() { };

		virtual void Init() = 0;

		virtual void Clear() const = 0;
		virtual void Clear(const FVector4& color) const = 0;

		virtual void SetVSyncEnabled(bool bEnabled) const = 0;
		virtual bool GetVSyncEnabled() const = 0;

		virtual void SetViewportDimensions(const SViewportDimensions& dimensions) const = 0;
		virtual SViewportDimensions GetViewportDimensions() const = 0;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const = 0;
		virtual EPolygonDrawMode GetPolygonDrawMode() const = 0;

	protected:
		Renderer() { }
	};
}
