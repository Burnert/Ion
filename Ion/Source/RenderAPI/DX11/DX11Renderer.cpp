#include "IonPCH.h"

#include "DX11Renderer.h"
#include "DX11Buffer.h"
#include "DX11Shader.h"
#include "DX11Texture.h"

#include "Renderer/Scene.h"

#include "Core/Platform/Windows/WindowsMacros.h"
#include "Core/Platform/Windows/WindowsUtility.h"

#include "Application/EnginePath.h"

#include "Engine/Entity/Entity.h"
#include "Engine/Components/MeshComponent.h"

namespace Ion
{
	DX11Renderer::DX11Renderer() :
		m_CurrentScene(nullptr),
		m_CurrentRTV(nullptr),
		m_CurrentDSV(nullptr),
		m_ViewportDimensions({ })
	{
	}

	DX11Renderer::~DX11Renderer()
	{
	}

	void DX11Renderer::Init()
	{
		TRACE_FUNCTION();

		InitScreenTextureRendering();
		InitShaders();
	}

	void DX11Renderer::Clear(const RendererClearOptions& options) const
	{
		TRACE_FUNCTION();

		ID3D11DeviceContext* context = DX11::GetContext();

		if (m_CurrentRTV)
		{
			if (options.bClearColor)
			{
				dxcall_v(context->ClearRenderTargetView(m_CurrentRTV, (float*)&options.ClearColorValue));
			}
		}
		if (m_CurrentDSV)
		{
			if (options.bClearDepth || options.bClearStencil)
			{
				dxcall_v(context->ClearDepthStencilView(m_CurrentDSV,
					FlagsIf(options.bClearDepth, D3D11_CLEAR_DEPTH) |
					FlagsIf(options.bClearStencil, D3D11_CLEAR_STENCIL),
					options.ClearDepthValue, options.ClearStencilValue));
			}
		}
	}

	void DX11Renderer::DrawIndexed(uint32 indexCount) const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall_v(context->DrawIndexed(indexCount, 0, 0));
	}

	void DX11Renderer::RenderEditorViewport(const TShared<Texture>& sceneFinalTexture, const TShared<Texture>& editorDataTexture) const
	{
		TRACE_FUNCTION();

		ID3D11DeviceContext* context = DX11::GetContext();

		BindScreenTexturePrimitives(GetEditorViewportShader().get());
		sceneFinalTexture->Bind(0);
		sceneFinalTexture->BindDepth(1);
		editorDataTexture->BindDepth(2);

		// Index count is always 6 (2 triangles)
		DrawIndexed(6);
	}

	void DX11Renderer::SetCurrentScene(const Scene* scene)
	{
		m_CurrentScene = scene;
	}

	const Scene* DX11Renderer::GetCurrentScene() const
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
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		DX11::s_Context->RSSetViewports(1, &viewport);
		DX11::ResizeBuffers(dimensions.Width, dimensions.Height);

		m_ViewportDimensions = dimensions;
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
		HRESULT hResult;

		ID3D11RasterizerState* rasterizerState = DX11::GetRasterizerState();

		D3D11_RASTERIZER_DESC rd{ };
		dxcall_v(rasterizerState->GetDesc(&rd));

		rd.FillMode = drawMode == EPolygonDrawMode::Lines ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;

		COMReset(DX11::s_RasterizerState);

		dxcall(DX11::GetDevice()->CreateRasterizerState(&rd, &DX11::s_RasterizerState));
		dxcall_v(DX11::GetContext()->RSSetState(DX11::s_RasterizerState));
	}

	EPolygonDrawMode DX11Renderer::GetPolygonDrawMode() const
	{
		ID3D11RasterizerState* rasterizerState = DX11::GetRasterizerState();

		D3D11_RASTERIZER_DESC rd { };
		dxcall_v(rasterizerState->GetDesc(&rd));

		return rd.FillMode == D3D11_FILL_WIREFRAME ? EPolygonDrawMode::Lines : EPolygonDrawMode::Fill;
	}

	void DX11Renderer::SetRenderTarget(const TShared<Texture>& targetTexture)
	{
		ionassert(!targetTexture || targetTexture->GetDescription().bUseAsRenderTarget);

		if (targetTexture)
		{
			DX11Texture* dx11Texture = (DX11Texture*)targetTexture.get();

			if (!targetTexture->HasColorAttachment() &&
				!targetTexture->HasDepthStencilAttachment())
			{
				LOG_ERROR("Cannot set render target. The texture needs at least one attachment.");
				return;
			}

			m_CurrentRTV = targetTexture->HasColorAttachment() ?
				dx11Texture->m_RTV : nullptr;
			m_CurrentDSV = targetTexture->HasDepthStencilAttachment() ?
				dx11Texture->m_DSV : nullptr;
		}
		else
		{
			m_CurrentRTV = DX11::GetRenderTargetView();
			m_CurrentDSV = DX11::GetDepthStencilView();
		}

		dxcall_v(DX11::GetContext()->OMSetRenderTargets(1, &m_CurrentRTV, m_CurrentDSV));

		D3D11_VIEWPORT viewport { };
		
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		if (targetTexture)
		{
			TextureDimensions dimensions = targetTexture->GetDimensions();
			viewport.TopLeftX = 0.0f;
			viewport.TopLeftY = 0.0f;
			viewport.Width = (float)dimensions.Width;
			viewport.Height = (float)dimensions.Height;
		}
		else
		{
			viewport.TopLeftX = (float)m_ViewportDimensions.X;
			viewport.TopLeftY = (float)m_ViewportDimensions.Y;
			viewport.Width = (float)m_ViewportDimensions.Width;
			viewport.Height = (float)m_ViewportDimensions.Height;
		}
		dxcall_v(DX11::s_Context->RSSetViewports(1, &viewport));
	}
}
