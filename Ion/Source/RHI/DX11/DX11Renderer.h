#pragma once

#include "Renderer/Renderer.h"
#include "DX11.h"

namespace Ion
{
	class DX11Shader;
	class DX11VertexBuffer;
	class DX11IndexBuffer;

	// @TODO: Move the rendering code to the Renderer class and leave only the render api specific things here

	class ION_API DX11Renderer : public Renderer
	{
	public:
		DX11Renderer();
		virtual ~DX11Renderer() override;

		virtual void Init() override;

		virtual void Clear(const RendererClearOptions& options) const override;

		virtual void DrawIndexed(uint32 indexCount) const override;

		virtual void UnbindResources() const override;

		virtual void SetBlendingEnabled(bool bEnable) const override;

		virtual void SetVSyncEnabled(bool bEnabled) const override;
		virtual bool IsVSyncEnabled() const override;

		virtual void SetViewport(const ViewportDescription& dimensions) override;
		virtual ViewportDescription GetViewport() const override;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const override;
		virtual EPolygonDrawMode GetPolygonDrawMode() const override;

		virtual void SetRenderTarget(const TShared<Texture>& targetTexture) override;
		virtual void SetDepthStencil(const TShared<Texture>& targetTexture) override;

	private:
		ID3D11RenderTargetView* m_CurrentRTV;
		ID3D11DepthStencilView* m_CurrentDSV;
		ViewportDescription m_CurrentViewport;
	};
}
