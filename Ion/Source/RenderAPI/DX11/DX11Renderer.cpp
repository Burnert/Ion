#include "IonPCH.h"

#include "DX11Renderer.h"
#include "DX11Buffer.h"
#include "DX11Shader.h"

#include "Core/Platform/Windows/WindowsMacros.h"
#include "Core/Platform/Windows/WindowsUtility.h"

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

		ID3D11DeviceContext* context = DX11::GetContext();

		DX11VertexBuffer* vb = (DX11VertexBuffer*)primitive.VertexBuffer;
		DX11IndexBuffer* ib = (DX11IndexBuffer*)primitive.IndexBuffer;
		DX11UniformBuffer* ub = (DX11UniformBuffer*)primitive.UniformBuffer;
		DX11Shader* shader = (DX11Shader*)primitive.Shader;

		shader->Bind();
		vb->Bind();
		vb->BindLayout();
		ib->Bind();
		ub->Bind();

		dxcall_v(context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		dxcall_v(context->DrawIndexed(ib->GetIndexCount(), 0, 0));
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
		TRACE_FUNCTION();

		HRESULT hResult = S_OK;

		D3D11_VIEWPORT viewport { };
		viewport.TopLeftX = (float)dimensions.X;
		viewport.TopLeftY = (float)dimensions.Y;
		viewport.Width = (float)dimensions.Width;
		viewport.Height = (float)dimensions.Height;

		DX11::s_Context->RSSetViewports(1, &viewport);
		DX11::ResizeBuffers(dimensions.Width, dimensions.Height);
	}

	ViewportDimensions DX11Renderer::GetViewportDimensions() const
	{
		TRACE_FUNCTION();

		D3D11_VIEWPORT viewport { };
		uint32 nViewports = 0;

		DX11::s_Context->RSGetViewports(&nViewports, &viewport);

		ViewportDimensions dimensions { };
		dimensions.X = (int32)viewport.TopLeftX;
		dimensions.Y = (int32)viewport.TopLeftY;
		dimensions.Width = (int32)viewport.Width;
		dimensions.Height = (int32)viewport.Height;

		return dimensions;
	}

	void DX11Renderer::SetPolygonDrawMode(EPolygonDrawMode drawMode) const
	{

	}

	EPolygonDrawMode DX11Renderer::GetPolygonDrawMode() const
	{
		return EPolygonDrawMode();
	}
}
