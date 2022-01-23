#pragma once

#include "Renderer/Renderer.h"
#include "DX11.h"

namespace Ion
{
	class DX11Shader;
	class DX11VertexBuffer;
	class DX11IndexBuffer;

	class ION_API DX11Renderer : public Renderer
	{
	public:
		DX11Renderer();
		virtual ~DX11Renderer() override;

		virtual void Init() override;

		virtual void Clear() const override;
		virtual void Clear(const Vector4& color) const override;

		virtual void Draw(const RPrimitiveRenderProxy& primitive, const TShared<Scene>& targetScene = nullptr) const override;
		virtual void DrawScreenTexture(const TShared<Texture>& texture) const override;

		virtual void RenderScene(const TShared<Scene>& scene) override;

		virtual void SetCurrentScene(const TShared<Scene>& scene) override;
		virtual const TShared<Scene>& GetCurrentScene() const override;

		virtual void SetVSyncEnabled(bool bEnabled) const override;
		virtual bool IsVSyncEnabled() const override;

		virtual void SetViewportDimensions(const ViewportDimensions& dimensions) const override;
		virtual ViewportDimensions GetViewportDimensions() const override;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const override;
		virtual EPolygonDrawMode GetPolygonDrawMode() const override;

		virtual void SetRenderTarget(const TShared<Texture>& targetTexture) override;

	private:
		TShared<Scene> m_CurrentScene;

		ID3D11RenderTargetView* m_CurrentRenderTarget;
	};
}
