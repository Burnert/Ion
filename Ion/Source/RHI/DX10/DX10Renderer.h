#pragma once

#include "Renderer/Renderer.h"
#include "DX10.h"

namespace Ion
{
	class DX10Shader;
	class DX10VertexBuffer;
	class DX10IndexBuffer;

	class ION_API DX10Renderer : public Renderer
	{
	public:
		DX10Renderer();
		virtual ~DX10Renderer() override;

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

		virtual Result<void, RHIError> SetRenderTarget(const TRef<RHITexture>& targetTexture) override;
		virtual Result<void, RHIError> SetDepthStencil(const TRef<RHITexture>& targetTexture) override;

	private:
		ID3D10RenderTargetView* m_CurrentRTV;
		ID3D10DepthStencilView* m_CurrentDSV;
		ViewportDescription m_CurrentViewport;
	};
}
