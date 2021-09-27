#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "Light.h"
#include "Camera.h"
#include "Scene.h"

#include "Drawable.h"

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
		virtual void Clear(const Vector4& color) const = 0;

		virtual void Draw(const IDrawable* drawable, const TShared<Scene>& targetScene = nullptr) const = 0;

		virtual void SetCurrentScene(const TShared<Scene>& scene) = 0;
		virtual const TShared<Scene>& GetCurrentScene() const = 0;

		virtual void RenderScene(const TShared<Scene>& scene) = 0;

		virtual void SetVSyncEnabled(bool bEnabled) const = 0;
		virtual bool IsVSyncEnabled() const = 0;

		virtual void SetViewportDimensions(const SViewportDimensions& dimensions) const = 0;
		virtual SViewportDimensions GetViewportDimensions() const = 0;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const = 0;
		virtual EPolygonDrawMode GetPolygonDrawMode() const = 0;

	protected:
		Renderer() { }
	};
}
