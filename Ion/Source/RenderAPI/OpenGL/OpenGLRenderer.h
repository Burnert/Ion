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

		virtual void Clear(const RendererClearOptions& options) const override;

		virtual void DrawIndexed(uint32 indexCount) const override;

		virtual void RenderEditorViewport(const EditorViewportTextures& editorViewportTextures) const override;

		virtual void SetVSyncEnabled(bool bEnabled) const override;
		virtual bool IsVSyncEnabled() const override;

		virtual void SetViewport(const ViewportDescription& viewport) override;
		virtual ViewportDescription GetViewport() const override;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const override;
		virtual EPolygonDrawMode GetPolygonDrawMode() const override;

		virtual void SetRenderTarget(const TShared<Texture>& targetTexture) override;
		virtual void SetDepthStencil(const TShared<Texture>& targetTexture) override;

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
