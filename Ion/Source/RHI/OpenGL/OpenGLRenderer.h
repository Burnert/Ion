#pragma once

#include "Renderer/Renderer.h"
#include "OpenGL.h"

namespace Ion
{
	class ION_API OpenGLRenderer : public Renderer
	{
	public:
		OpenGLRenderer();
		virtual ~OpenGLRenderer() override;

		virtual void Init() override;

		virtual Result<void, RHIError> Clear(const RendererClearOptions& options) const override;

		virtual Result<void, RHIError> DrawIndexed(uint32 indexCount) const override;

		virtual Result<void, RHIError> UnbindResources() const override;

		virtual Result<void, RHIError> SetBlendingEnabled(bool bEnable) const override;

		virtual Result<void, RHIError> SetVSyncEnabled(bool bEnabled) const override;
		virtual bool IsVSyncEnabled() const override;

		virtual Result<void, RHIError> SetViewport(const ViewportDescription& viewport) override;
		virtual Result<ViewportDescription, RHIError> GetViewport() const override;

		virtual Result<void, RHIError> SetPolygonDrawMode(EPolygonDrawMode drawMode) const override;
		virtual Result<EPolygonDrawMode, RHIError> GetPolygonDrawMode() const override;

		virtual Result<void, RHIError> SetRenderTarget(const TRef<RHITexture>& targetTexture) override;
		virtual Result<void, RHIError> SetDepthStencil(const TRef<RHITexture>& targetTexture) override;

		FORCEINLINE static uint32 PolygonDrawModeToGLPolygonMode(EPolygonDrawMode drawMode)
		{
			switch (drawMode)
			{
			case EPolygonDrawMode::Fill:    return GL_FILL;
			case EPolygonDrawMode::Lines:   return GL_LINE;
			case EPolygonDrawMode::Points:  return GL_POINT;
			default:                        return 0;
			}
		}
		FORCEINLINE static EPolygonDrawMode GLPolygonModeToPolygonDrawMode(int32 polygonMode)
		{
			switch (polygonMode)
			{
			case GL_FILL:   return EPolygonDrawMode::Fill;
			case GL_LINE:   return EPolygonDrawMode::Lines;
			case GL_POINT:  return EPolygonDrawMode::Points;
			default:        return (EPolygonDrawMode)-1;
			}
		}

		inline uint32 GetCurrentRenderTarget() const
		{
			return m_CurrentRenderTarget;
		}

	private:
		uint32 m_CurrentRenderTarget;
	};
}
