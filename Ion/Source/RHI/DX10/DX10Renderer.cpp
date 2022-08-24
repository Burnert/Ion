#include "IonPCH.h"

#include "DX10.h"
#include "DX10Renderer.h"
#include "DX10Buffer.h"
#include "DX10Shader.h"
#include "DX10Texture.h"

#include "Renderer/Scene.h"

#include "Application/EnginePath.h"

#include "Engine/Entity/Entity.h"
#include "Engine/Components/MeshComponent.h"

namespace Ion
{
	DX10Renderer::DX10Renderer() :
		m_CurrentRTV(nullptr),
		m_CurrentDSV(nullptr)
	{
	}

	DX10Renderer::~DX10Renderer()
	{
	}

	void DX10Renderer::Init()
	{
		TRACE_FUNCTION();

		Renderer::Init();
	}

	Result<void, RHIError> DX10Renderer::Clear(const RendererClearOptions& options) const
	{
		TRACE_FUNCTION();

		ID3D10Device* device = DX10::GetDevice();

		if (m_CurrentRTV)
		{
			if (options.bClearColor)
			{
				dxcall(device->ClearRenderTargetView(m_CurrentRTV, (float*)&options.ClearColorValue));
			}
		}
		if (m_CurrentDSV)
		{
			if (options.bClearDepth || options.bClearStencil)
			{
				dxcall(device->ClearDepthStencilView(m_CurrentDSV,
					FlagsIf(options.bClearDepth, D3D10_CLEAR_DEPTH) |
					FlagsIf(options.bClearStencil, D3D10_CLEAR_STENCIL),
					options.ClearDepthValue, options.ClearStencilValue));
			}
		}

		return Ok();
	}

	Result<void, RHIError> DX10Renderer::DrawIndexed(uint32 indexCount) const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall(device->DrawIndexed(indexCount, 0, 0));

		return Ok();
	}

	Result<void, RHIError> DX10Renderer::UnbindResources() const
	{
		ID3D10Device* device = DX10::GetDevice();

		static ID3D10ShaderResourceView* const c_nullViews[D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = { };
		
		dxcall(device->PSSetShaderResources(0, D3D10_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, c_nullViews));

		return Ok();
	}

	Result<void, RHIError> DX10Renderer::SetBlendingEnabled(bool bEnable) const
	{
		ID3D10Device* device = DX10::GetDevice();

		ID3D10BlendState* blendState = bEnable ?
			DX10::s_BlendStateTransparent :
			DX10::s_BlendState;

		dxcall(device->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF));

		return Ok();
	}

	Result<void, RHIError> DX10Renderer::SetVSyncEnabled(bool bEnabled) const
	{
		dxcall(DX10::SetSwapInterval((uint32)bEnabled));

		return Ok();
	}

	bool DX10Renderer::IsVSyncEnabled() const
	{
		return DX10::GetSwapInterval();
	}

	Result<void, RHIError> DX10Renderer::SetViewport(const ViewportDescription& viewport)
	{
		TRACE_FUNCTION();

		HRESULT hResult = S_OK;

		ID3D10Device* device = DX10::GetDevice();

		D3D10_VIEWPORT dxViewport { };
		dxViewport.TopLeftX = viewport.X;
		dxViewport.TopLeftY = viewport.Y;
		dxViewport.Width = viewport.Width;
		dxViewport.Height = viewport.Height;
		dxViewport.MinDepth = viewport.MinDepth;
		dxViewport.MaxDepth = viewport.MaxDepth;

		dxcall(device->RSSetViewports(1, &dxViewport));

		m_CurrentViewport = viewport;

		return Ok();
	}

	Result<ViewportDescription, RHIError> DX10Renderer::GetViewport() const
	{
		TRACE_FUNCTION();

		ID3D10Device* device = DX10::GetDevice();

		D3D10_VIEWPORT dxViewport { };
		uint32 nViewports = 0;

		dxcall(device->RSGetViewports(&nViewports, &dxViewport));

		ViewportDescription viewport { };
		viewport.X = (int32)dxViewport.TopLeftX;
		viewport.Y = (int32)dxViewport.TopLeftY;
		viewport.Width = (uint32)dxViewport.Width;
		viewport.Height = (uint32)dxViewport.Height;
		viewport.MinDepth = dxViewport.MinDepth;
		viewport.MaxDepth = dxViewport.MaxDepth;

		return viewport;
	}

	Result<void, RHIError> DX10Renderer::SetPolygonDrawMode(EPolygonDrawMode drawMode) const
	{
		ID3D10Device* device = DX10::GetDevice();
		ID3D10RasterizerState* rasterizerState = DX10::GetRasterizerState();

		D3D10_RASTERIZER_DESC rd{ };
		dxcall(rasterizerState->GetDesc(&rd));

		rd.FillMode = drawMode == EPolygonDrawMode::Lines ? D3D10_FILL_WIREFRAME : D3D10_FILL_SOLID;

		COMReset(DX10::s_RasterizerState);

		dxcall(device->CreateRasterizerState(&rd, &DX10::s_RasterizerState));
		dxcall(device->RSSetState(DX10::s_RasterizerState));

		return Ok();
	}

	Result<EPolygonDrawMode, RHIError> DX10Renderer::GetPolygonDrawMode() const
	{
		ID3D10RasterizerState* rasterizerState = DX10::GetRasterizerState();

		D3D10_RASTERIZER_DESC rd { };
		dxcall(rasterizerState->GetDesc(&rd));

		return rd.FillMode == D3D10_FILL_WIREFRAME ? EPolygonDrawMode::Lines : EPolygonDrawMode::Fill;
	}

	Result<void, RHIError> DX10Renderer::SetRenderTarget(const TRef<RHITexture>& targetTexture)
	{
		ionassert(!targetTexture || targetTexture->GetDescription().bUseAsRenderTarget);
		ionassert(!targetTexture || UVector2(targetTexture->GetDimensions()) == m_CurrentViewport.GetSize());

		ID3D10Device* device = DX10::GetDevice();

		if (targetTexture)
		{
			DX10Texture* dx11Texture = (DX10Texture*)targetTexture.Raw();
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
		// @TODO: This shouldn't work like this,
		// it should be the user's responsibility to unset the DSV.
		m_CurrentDSV = nullptr;

		dxcall(device->OMSetRenderTargets(1, &m_CurrentRTV, m_CurrentDSV));

		return Ok();
	}

	Result<void, RHIError> DX10Renderer::SetDepthStencil(const TRef<RHITexture>& targetTexture)
	{
		ionassert(!targetTexture || targetTexture->GetDescription().bUseAsDepthStencil);
		ionassert(!targetTexture || UVector2(targetTexture->GetDimensions()) == m_CurrentViewport.GetSize());

		ID3D10Device* device = DX10::GetDevice();

		if (targetTexture)
		{
			DX10Texture* dx11Texture = (DX10Texture*)targetTexture.Raw();
			m_CurrentDSV = dx11Texture->m_DSV;
		}
		else
		{
			m_CurrentDSV = nullptr;
		}

		dxcall(device->OMSetRenderTargets(1, &m_CurrentRTV, m_CurrentDSV));

		return Ok();
	}
}
