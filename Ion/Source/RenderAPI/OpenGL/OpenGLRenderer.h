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

		virtual void Clear() const override;
		virtual void Clear(const FVector4& color) const override;

		virtual void Draw(const TShared<IDrawable>& drawable, const TShared<Scene>& targetScene = nullptr) const override;

		virtual void RenderScene(const TShared<Scene>& scene) override;

		virtual void SetCurrentScene(const TShared<Scene>& scene) override;
		virtual const TShared<Scene>& GetCurrentScene() const override;

		virtual void SetVSyncEnabled(bool bEnabled) const override;
		virtual bool IsVSyncEnabled() const override;

		virtual void SetViewportDimensions(const SViewportDimensions& dimensions) const override;
		virtual SViewportDimensions GetViewportDimensions() const override;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const override;
		virtual EPolygonDrawMode GetPolygonDrawMode() const override;

		FORCEINLINE static uint PolygonDrawModeToGLPolygonMode(EPolygonDrawMode drawMode)
		{
			switch (drawMode)
			{
			case EPolygonDrawMode::Fill:    return GL_FILL;
			case EPolygonDrawMode::Lines:   return GL_LINE;
			case EPolygonDrawMode::Points:  return GL_POINT;
			default:                        return 0;
			}
		}
		FORCEINLINE static EPolygonDrawMode GLPolygonModeToPolygonDrawMode(int polygonMode)
		{
			switch (polygonMode)
			{
			case GL_FILL:   return EPolygonDrawMode::Fill;
			case GL_LINE:   return EPolygonDrawMode::Lines;
			case GL_POINT:  return EPolygonDrawMode::Points;
			default:        return (EPolygonDrawMode)-1;
			}
		}

	private:
		TShared<Scene> m_CurrentScene;
	};
}
