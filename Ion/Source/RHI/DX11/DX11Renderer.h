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

		virtual Result<void, RHIError> Clear(const RendererClearOptions& options) const override;

		virtual Result<void, RHIError> DrawIndexed(uint32 indexCount) const override;

		virtual Result<void, RHIError> UnbindResources() const override;

		virtual Result<void, RHIError> SetBlendingEnabled(bool bEnable) const override;

		virtual Result<void, RHIError> SetVSyncEnabled(bool bEnabled) const override;
		virtual bool IsVSyncEnabled() const override;

		virtual Result<void, RHIError> SetViewport(const ViewportDescription& dimensions) override;
		virtual Result<ViewportDescription, RHIError> GetViewport() const override;

		virtual Result<void, RHIError> SetPolygonDrawMode(EPolygonDrawMode drawMode) const override;
		virtual Result<EPolygonDrawMode, RHIError> GetPolygonDrawMode() const override;

		virtual Result<void, RHIError> SetRenderTarget(const std::shared_ptr<RHITexture>& targetTexture) override;
		virtual Result<void, RHIError> SetDepthStencil(const std::shared_ptr<RHITexture>& targetTexture) override;

	private:
		ID3D11RenderTargetView* m_CurrentRTV;
		ID3D11DepthStencilView* m_CurrentDSV;
		ViewportDescription m_CurrentViewport;
	};
}
