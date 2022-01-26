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
	struct ScreenTextureRenderData
	{
		TShared<Shader> Shader;
		TShared<VertexBuffer> VertexBuffer;
		TShared<IndexBuffer> IndexBuffer;
	};

	class ION_API Renderer
	{
	public:
		static Renderer* Create();
		inline static Renderer* Get()
		{
			ionassert(s_Instance);
			return s_Instance;
		}

		virtual ~Renderer() { };

		virtual void Init() = 0;

		virtual void Clear() const = 0;
		virtual void Clear(const Vector4& color) const = 0;

		virtual void Draw(const RPrimitiveRenderProxy& primitive, const TShared<Scene>& targetScene = nullptr) const = 0;
		virtual void DrawScreenTexture(const TShared<Texture>& texture) const = 0;

		virtual void SetCurrentScene(const TShared<Scene>& scene) = 0;
		virtual const TShared<Scene>& GetCurrentScene() const = 0;

		virtual void RenderScene(const TShared<Scene>& scene) = 0;

		virtual void SetVSyncEnabled(bool bEnabled) const = 0;
		virtual bool IsVSyncEnabled() const = 0;

		virtual void SetViewportDimensions(const ViewportDimensions& dimensions) const = 0;
		virtual ViewportDimensions GetViewportDimensions() const = 0;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const = 0;
		virtual EPolygonDrawMode GetPolygonDrawMode() const = 0;

		virtual void SetRenderTarget(const TShared<Texture>& targetTexture) = 0;

	protected:
		Renderer() { }

		void InitScreenTextureRendering();
		void BindScreenTexturePrimitives() const;

	private:
		void CreateScreenTexturePrimitives();

	private:
		ScreenTextureRenderData m_ScreenTextureRenderData;

		static Renderer* s_Instance;
	};
}
