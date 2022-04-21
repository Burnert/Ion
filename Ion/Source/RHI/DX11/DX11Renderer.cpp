#include "IonPCH.h"

#include "DX11.h"
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
		m_CurrentRTV(nullptr),
		m_CurrentDSV(nullptr)
	{
	}

	DX11Renderer::~DX11Renderer()
	{
	}

	void DX11Renderer::Init()
	{
		TRACE_FUNCTION();

		Renderer::Init();
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

	void DX11Renderer::UnbindResources() const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		static ID3D11ShaderResourceView* const c_nullViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = { };
		
		dxcall_v(context->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, c_nullViews));
	}

	void DX11Renderer::SetBlendingEnabled(bool bEnable) const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		ID3D11BlendState* blendState = bEnable ?
			DX11::s_BlendStateTransparent :
			DX11::s_BlendState;

		dxcall_v(context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF));
	}

	void DX11Renderer::SetVSyncEnabled(bool bEnabled) const
	{
		dxcall_v(DX11::SetSwapInterval((uint32)bEnabled));
	}

	bool DX11Renderer::IsVSyncEnabled() const
	{
		return DX11::GetSwapInterval();
	}

	void DX11Renderer::SetViewport(const ViewportDescription& viewport)
	{
		TRACE_FUNCTION();

		HRESULT hResult = S_OK;

		ID3D11DeviceContext* context = DX11::GetContext();

		D3D11_VIEWPORT dxViewport { };
		dxViewport.TopLeftX = (float)viewport.X;
		dxViewport.TopLeftY = (float)viewport.Y;
		dxViewport.Width = (float)viewport.Width;
		dxViewport.Height = (float)viewport.Height;
		dxViewport.MinDepth = viewport.MinDepth;
		dxViewport.MaxDepth = viewport.MaxDepth;

		dxcall_v(context->RSSetViewports(1, &dxViewport));

		m_CurrentViewport = viewport;
	}

	ViewportDescription DX11Renderer::GetViewport() const
	{
		TRACE_FUNCTION();

		ID3D11DeviceContext* context = DX11::GetContext();

		D3D11_VIEWPORT dxViewport { };
		uint32 nViewports = 0;

		dxcall_v(context->RSGetViewports(&nViewports, &dxViewport));

		ViewportDescription viewport { };
		viewport.X = (int32)dxViewport.TopLeftX;
		viewport.Y = (int32)dxViewport.TopLeftY;
		viewport.Width = (uint32)dxViewport.Width;
		viewport.Height = (uint32)dxViewport.Height;
		viewport.MinDepth = dxViewport.MinDepth;
		viewport.MaxDepth = dxViewport.MaxDepth;

		return viewport;
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

	void DX11Renderer::SetRenderTarget(const TShared<RHITexture>& targetTexture)
	{
		ionassert(!targetTexture || targetTexture->GetDescription().bUseAsRenderTarget);
		ionassert(!targetTexture || UVector2(targetTexture->GetDimensions()) == m_CurrentViewport.GetSize());

		// ahhhhhhhh idk
		//if (targetTexture && UVector2(targetTexture->GetDimensions()) != m_CurrentViewport.GetSize())
		//{
		//	TextureDimensions size = targetTexture->GetDimensions();
		//	ViewportDescription viewport = m_CurrentViewport;
		//	viewport.X = 0;
		//	viewport.Y = 0;
		//	viewport.Width = size.Width;
		//	viewport.Height = size.Height;
		//	SetViewport(viewport);
		//}

		if (targetTexture)
		{
			DX11Texture* dx11Texture = (DX11Texture*)targetTexture.get();
			m_CurrentRTV = dx11Texture->m_RTV;
		}
		else
		{
			m_CurrentRTV = nullptr;
		}
		// Override the depth stencil too
		// The DSV has to be the same format as the RTV
		// so it has to be cleared here to make it possible
		// to set the RTV
		m_CurrentDSV = nullptr;

		dxcall_v(DX11::GetContext()->OMSetRenderTargets(1, &m_CurrentRTV, m_CurrentDSV));
	}

	void DX11Renderer::SetDepthStencil(const TShared<RHITexture>& targetTexture)
	{
		ionassert(!targetTexture || targetTexture->GetDescription().bUseAsDepthStencil);
		ionassert(!targetTexture || UVector2(targetTexture->GetDimensions()) == m_CurrentViewport.GetSize());

		if (targetTexture)
		{
			DX11Texture* dx11Texture = (DX11Texture*)targetTexture.get();
			m_CurrentDSV = dx11Texture->m_DSV;
		}
		else
		{
			m_CurrentDSV = nullptr;
		}

		dxcall_v(DX11::GetContext()->OMSetRenderTargets(1, &m_CurrentRTV, m_CurrentDSV));
	}
}
