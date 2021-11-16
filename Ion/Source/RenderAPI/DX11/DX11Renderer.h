#pragma once

#include "Renderer/Renderer.h"
#include "DX11.h"

namespace Ion
{
	class ION_API DX11Renderer : public Renderer
	{
	public:
		DX11Renderer();
		virtual ~DX11Renderer() override;

		virtual void Init() override;

		virtual void Clear() const override;
		virtual void Clear(const Vector4& color) const override;

		virtual void Draw(const RPrimitiveRenderProxy& primitive, const TShared<Scene>& targetScene = nullptr) const override;

		virtual void RenderScene(const TShared<Scene>& scene) override;

		virtual void SetCurrentScene(const TShared<Scene>& scene) override;
		virtual const TShared<Scene>& GetCurrentScene() const override;

		virtual void SetVSyncEnabled(bool bEnabled) const override;
		virtual bool IsVSyncEnabled() const override;

		virtual void SetViewportDimensions(const ViewportDimensions& dimensions) const override;
		virtual ViewportDimensions GetViewportDimensions() const override;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const override;
		virtual EPolygonDrawMode GetPolygonDrawMode() const override;

		//FORCEINLINE static uint32 PolygonDrawModeToGLPolygonMode(EPolygonDrawMode drawMode)
		//{
		//	switch (drawMode)
		//	{
		//	case EPolygonDrawMode::Fill:    return GL_FILL;
		//	case EPolygonDrawMode::Lines:   return GL_LINE;
		//	case EPolygonDrawMode::Points:  return GL_POINT;
		//	default:                        return 0;
		//	}
		//}
		//FORCEINLINE static EPolygonDrawMode GLPolygonModeToPolygonDrawMode(int32 polygonMode)
		//{
		//	switch (polygonMode)
		//	{
		//	case GL_FILL:   return EPolygonDrawMode::Fill;
		//	case GL_LINE:   return EPolygonDrawMode::Lines;
		//	case GL_POINT:  return EPolygonDrawMode::Points;
		//	default:        return (EPolygonDrawMode)-1;
		//	}
		//}

	private:
		TShared<Scene> m_CurrentScene;
	};
}
