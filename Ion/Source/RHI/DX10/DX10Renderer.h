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

		virtual void SetRenderTarget(const TShared<RHITexture>& targetTexture) override;
		virtual void SetDepthStencil(const TShared<RHITexture>& targetTexture) override;

	private:
		ID3D10RenderTargetView* m_CurrentRTV;
		ID3D10DepthStencilView* m_CurrentDSV;
		ViewportDescription m_CurrentViewport;
	};
}
