#include "IonPCH.h"

#include "DX11Renderer.h"

namespace Ion
{
	DX11Renderer::DX11Renderer() :
		m_CurrentScene({ })
	{
	}

	DX11Renderer::~DX11Renderer()
	{
	}

	void DX11Renderer::Init()
	{

	}

	void DX11Renderer::Clear() const
	{
		Clear(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void DX11Renderer::Clear(const Vector4& color) const
	{
		TRACE_FUNCTION();

		DX11::s_Context->ClearRenderTargetView(DX11::s_RenderTarget, (float*)&color);
	}

	void DX11Renderer::Draw(const RPrimitiveRenderProxy& primitive, const TShared<Scene>& targetScene) const
	{
		TRACE_FUNCTION();

	}

	void DX11Renderer::RenderScene(const TShared<Scene>& scene)
	{
		TRACE_FUNCTION();

		m_CurrentScene = scene;

		for (const RPrimitiveRenderProxy& primitive : scene->GetScenePrimitives())
		{
			Draw(primitive, scene);
		}
	}

	void DX11Renderer::SetCurrentScene(const TShared<Scene>& scene)
	{
		m_CurrentScene = scene;
	}

	const TShared<Scene>& DX11Renderer::GetCurrentScene() const
	{
		return m_CurrentScene;
	}

	void DX11Renderer::SetVSyncEnabled(bool bEnabled) const
	{
		DX11::SetSwapInterval((uint32)bEnabled);
	}

	bool DX11Renderer::IsVSyncEnabled() const
	{
		return DX11::GetSwapInterval();
	}

	void DX11Renderer::SetViewportDimensions(const ViewportDimensions& dimensions) const
	{
		// @TODO: Implement
		//DX11::s_SwapChain->ResizeBuffers
	}

	ViewportDimensions DX11Renderer::GetViewportDimensions() const
	{
		// @TODO: Implement
		return { };
	}

	void DX11Renderer::SetPolygonDrawMode(EPolygonDrawMode drawMode) const
	{

	}

	EPolygonDrawMode DX11Renderer::GetPolygonDrawMode() const
	{
		return EPolygonDrawMode();
	}
}
