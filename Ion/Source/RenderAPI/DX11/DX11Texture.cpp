#include "IonPCH.h"

#include "DX11Texture.h"

namespace Ion
{
	DX11Texture::~DX11Texture()
	{
		TRACE_FUNCTION();

		ReleaseResources();
	}

	void DX11Texture::SetDimensions(TextureDimensions dimensions)
	{
		//ionassert(dimensions.Width <= 16384 && dimensions.Height <= 16384, "Textures larger than 16K are unsupported.");
		// @TODO: This probably should not be here (recreate the texture instead)
	}

	void DX11Texture::UpdateSubresource(Image* image)
	{
		TRACE_FUNCTION();

		ionassert(m_Description.bUseAsRenderTarget,
			"Cannot update subresource if the texture has not been created as a render target.");

		if (!image->IsLoaded())
		{
			LOG_ERROR("Cannot Update Subresource of Texture. Image has not been loaded.");
			return;
		}

		ID3D11DeviceContext* context = DX11::GetContext();

		// Dimensions are different
		if (image->GetWidth() != m_Description.Dimensions.Width ||
			image->GetHeight() != m_Description.Dimensions.Height)
		{
			LOG_WARN("Image dimensions do not match texture dimensions.");
			return;
		}

		uint32 lineSize = image->GetWidth() * 4;

		// Update Subresource
		dxcall_v(context->UpdateSubresource(m_ColorTexture, 0, nullptr, image->GetPixelData(), lineSize, 0));

		if (m_Description.bGenerateMips)
		{
			// Generate mipmaps
			dxcall_v(context->GenerateMips(m_ColorSRV));
		}
	}

	void DX11Texture::Bind(uint32 slot) const
	{
		dxcall_v(DX11::GetContext()->PSSetShaderResources(slot, 1, &m_ColorSRV));
		dxcall_v(DX11::GetContext()->PSSetSamplers(slot, 1, &m_ColorSamplerState));
	}

	void DX11Texture::Unbind() const
	{
		dxcall_v(DX11::GetContext()->PSSetShaderResources(0, 0, nullptr));
		dxcall_v(DX11::GetContext()->PSSetSamplers(0, 0, nullptr));
	}

	void* DX11Texture::GetNativeID() const
	{
		return m_ColorSRV;
	}

	DX11Texture::DX11Texture(const TextureDescription& desc) :
		Texture(desc),
		m_ColorTexture(nullptr),
		m_RTV(nullptr),
		m_ColorSRV(nullptr),
		m_ColorSamplerState(nullptr),
		m_DepthStencilTexture(nullptr),
		m_DSV(nullptr),
		m_DepthSRV(nullptr),
		m_DepthSamplerState(nullptr)
	{
		CreateAttachments(desc);
	}

	void DX11Texture::CreateAttachments(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		if (desc.bCreateColorAttachment)
		{
			CreateColorAttachment(desc);
		}

		if (desc.bCreateDepthStencilAttachment)
		{
			CreateDepthStencilAttachment(desc);
		}
	}

	void DX11Texture::CreateColorAttachment(const TextureDescription& desc)
	{
		HRESULT hResult;

		ID3D11Device* device = DX11::GetDevice();
		ID3D11DeviceContext* context = DX11::GetContext();

		uint32 lineSize = desc.Dimensions.Width * 4;

		// Create Texture2D

		D3D11_TEXTURE2D_DESC tex2DDesc { };
		tex2DDesc.Width = desc.Dimensions.Width;
		tex2DDesc.Height = desc.Dimensions.Height;
		tex2DDesc.MipLevels = 0;
		tex2DDesc.ArraySize = 1;
		tex2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		tex2DDesc.Usage = UsageToDX11Usage(desc.Usage);
		tex2DDesc.SampleDesc.Count = 1;

		tex2DDesc.BindFlags =
			D3D11_BIND_SHADER_RESOURCE |
			FlagsIf(desc.bUseAsRenderTarget, D3D11_BIND_RENDER_TARGET);

		tex2DDesc.CPUAccessFlags =
			FlagsIf(desc.bAllowCPUReadAccess, D3D11_CPU_ACCESS_READ) |
			FlagsIf(desc.bAllowCPUWriteAccess, D3D11_CPU_ACCESS_WRITE);

		tex2DDesc.MiscFlags =
			FlagsIf(desc.bGenerateMips, D3D11_RESOURCE_MISC_GENERATE_MIPS);

		dxcall(device->CreateTexture2D(&tex2DDesc, nullptr, &m_ColorTexture));

		// Create SRV (Shader Resource View)

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc { };
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;

		dxcall(device->CreateShaderResourceView(m_ColorTexture, &srvDesc, &m_ColorSRV));

		// Create Sampler State

		D3D11_SAMPLER_DESC samplerDesc { };
		samplerDesc.Filter = SelectDX11Filter(desc.MinFilter, desc.MagFilter, desc.MipFilter);
		samplerDesc.AddressU = WrapModeToDX11AddressMode(desc.UWrapMode);
		samplerDesc.AddressV = WrapModeToDX11AddressMode(desc.VWrapMode);
		samplerDesc.AddressW = WrapModeToDX11AddressMode(desc.WWrapMode);
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.MipLODBias = desc.LODBias;
		samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		dxcall(device->CreateSamplerState(&samplerDesc, &m_ColorSamplerState));

		// Create RTV (Render Target View)

		if (desc.bUseAsRenderTarget)
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc { };
			rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;

			dxcall(device->CreateRenderTargetView(m_ColorTexture, &rtvDesc, &m_RTV));
		}
	}

	void DX11Texture::CreateDepthStencilAttachment(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		HRESULT hResult;

		ID3D11Device* device = DX11::GetDevice();

		// Create Depth Stencil Texture2D

		D3D11_TEXTURE2D_DESC depthDesc { };
		depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthDesc.Width = desc.Dimensions.Width;
		depthDesc.Height = desc.Dimensions.Height;

		depthDesc.BindFlags =
			D3D11_BIND_DEPTH_STENCIL |
			FlagsIf(desc.bCreateDepthSampler, D3D11_BIND_SHADER_RESOURCE);

		depthDesc.Usage = UsageToDX11Usage(desc.Usage);
		depthDesc.SampleDesc.Count = 1;
		depthDesc.SampleDesc.Quality = 0;
		depthDesc.MiscFlags = 0;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;

		depthDesc.CPUAccessFlags =
			FlagsIf(desc.bAllowCPUReadAccess, D3D11_CPU_ACCESS_READ) |
			FlagsIf(desc.bAllowCPUWriteAccess, D3D11_CPU_ACCESS_WRITE);

		dxcall(device->CreateTexture2D(&depthDesc, nullptr, &m_DepthStencilTexture));

		// Create DSV (Depth Stencil View)

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd { };
		dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvd.Texture2D.MipSlice = 0;

		dxcall(device->CreateDepthStencilView(m_DepthStencilTexture, &dsvd, &m_DSV));

		if (desc.bCreateDepthSampler)
		{
			// Create SRV (Shader Resource View)

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc { };
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = -1;

			dxcall(device->CreateShaderResourceView(m_DepthStencilTexture, &srvDesc, &m_DepthSRV));

			// Create Sampler State

			D3D11_SAMPLER_DESC samplerDesc { };
			samplerDesc.Filter = SelectDX11Filter(desc.MinFilter, desc.MagFilter, desc.MipFilter);
			samplerDesc.AddressU = WrapModeToDX11AddressMode(desc.UWrapMode);
			samplerDesc.AddressV = WrapModeToDX11AddressMode(desc.VWrapMode);
			samplerDesc.AddressW = WrapModeToDX11AddressMode(desc.WWrapMode);
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.MipLODBias = desc.LODBias;
			samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			dxcall(device->CreateSamplerState(&samplerDesc, &m_DepthSamplerState));
		}
	}

	void DX11Texture::ReleaseResources()
	{
		TRACE_FUNCTION();

		COMRelease(m_ColorTexture);
		COMRelease(m_RTV);
		COMRelease(m_ColorSRV);
		COMRelease(m_ColorSamplerState);

		COMRelease(m_DepthStencilTexture);
		COMRelease(m_DSV);
		COMRelease(m_DepthSRV);
		COMRelease(m_DepthSamplerState);
	}
}
