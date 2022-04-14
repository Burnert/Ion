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
		dxcall_v(context->UpdateSubresource(m_Texture, 0, nullptr, image->GetPixelData(), lineSize, 0));

		if (m_Description.bGenerateMips)
		{
			// Generate mipmaps
			dxcall_v(context->GenerateMips(m_SRV));
		}
	}

	void DX11Texture::Bind(uint32 slot) const
	{
		ionassert(m_SRV);
		ionassert(m_SamplerState);
		dxcall_v(DX11::GetContext()->PSSetShaderResources(slot, 1, &m_SRV));
		dxcall_v(DX11::GetContext()->PSSetSamplers(slot, 1, &m_SamplerState));
	}

	void DX11Texture::Unbind() const
	{
		dxcall_v(DX11::GetContext()->PSSetShaderResources(0, 0, nullptr));
		dxcall_v(DX11::GetContext()->PSSetSamplers(0, 0, nullptr));
	}

	void DX11Texture::CopyTo(const TShared<Texture>& destination) const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		TShared<DX11Texture> destTexture = TStaticCast<DX11Texture>(destination);
		dxcall_v(context->CopyResource(destTexture->m_Texture, m_Texture));
	}

	void DX11Texture::Map(void*& outBuffer, int32& outLineSize, ETextureMapType mapType)
	{
		HRESULT hResult;

		ID3D11DeviceContext* context = DX11::GetContext();

		D3D11_MAPPED_SUBRESOURCE msr { };

		dxcall(context->Map(m_Texture, 0, MapTypeToDX11Map(mapType), 0, &msr));

		outBuffer = msr.pData;
		outLineSize = msr.RowPitch;
	}

	void DX11Texture::Unmap()
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall_v(context->Unmap(m_Texture, 0));
	}

	void* DX11Texture::GetNativeID() const
	{
		return m_SRV;
	}

	DX11Texture::DX11Texture(const TextureDescription& desc) :
		Texture(desc),
		m_Texture(nullptr),
		m_RTV(nullptr),
		m_DSV(nullptr),
		m_SRV(nullptr),
		m_SamplerState(nullptr)
	{
		CreateTexture(desc);
		// Can't do any of this if this is a staging texture
		if (desc.Usage != ETextureUsage::Staging)
		{
			CreateViews(desc);
			CreateSampler(desc);
		}
	}

	DX11Texture::DX11Texture(const TextureDescription& desc, ID3D11Texture2D* existingResource) :
		Texture(desc),
		m_Texture(existingResource),
		m_RTV(nullptr),
		m_DSV(nullptr),
		m_SRV(nullptr),
		m_SamplerState(nullptr)
	{
		// Can't do any of this if this is a staging texture
		if (desc.Usage != ETextureUsage::Staging)
		{
			CreateViews(desc);
			CreateSampler(desc);
		}
	}

	void DX11Texture::CreateTexture(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		HRESULT hResult;

		ID3D11Device* device = DX11::GetDevice();
		ID3D11DeviceContext* context = DX11::GetContext();

		uint32 lineSize = desc.Dimensions.Width * 4;

		// Create Texture2D

		D3D11_TEXTURE2D_DESC tex2DDesc { };
		tex2DDesc.Width = desc.Dimensions.Width;
		tex2DDesc.Height = desc.Dimensions.Height;
		tex2DDesc.MipLevels = 1; // @TODO: fix the missing mipmaps thing
		tex2DDesc.ArraySize = 1;
		tex2DDesc.Format = FormatToDX11Format(desc.Format, EDX11FormatUsage::Resource);
		tex2DDesc.Usage = UsageToDX11Usage(desc.Usage);
		tex2DDesc.SampleDesc.Count = 1;

		tex2DDesc.BindFlags =
			FlagsIf(desc.bCreateSampler &&
				desc.Usage != ETextureUsage::Staging, D3D11_BIND_SHADER_RESOURCE) |
			FlagsIf(desc.bUseAsRenderTarget, D3D11_BIND_RENDER_TARGET) |
			FlagsIf(desc.bUseAsDepthStencil, D3D11_BIND_DEPTH_STENCIL);

		tex2DDesc.CPUAccessFlags =
			FlagsIf(desc.bAllowCPUReadAccess, D3D11_CPU_ACCESS_READ) |
			FlagsIf(desc.bAllowCPUWriteAccess, D3D11_CPU_ACCESS_WRITE);

		tex2DDesc.MiscFlags =
			FlagsIf(desc.bGenerateMips, D3D11_RESOURCE_MISC_GENERATE_MIPS);

		D3D11_SUBRESOURCE_DATA sd { };

		ionassert(desc.Usage != ETextureUsage::Immutable || desc.InitialData);
		if (desc.InitialData)
		{
			sd.pSysMem = desc.InitialData;
			sd.SysMemPitch = desc.Dimensions.Width * 4;
		}

		dxcall(device->CreateTexture2D(&tex2DDesc, desc.InitialData ? &sd : nullptr, &m_Texture));
		DX11::SetDebugName(m_Texture, desc.DebugName, "Texture2D_");
	}

	void DX11Texture::CreateViews(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(m_Texture);

		HRESULT hResult;

		ID3D11Device* device = DX11::GetDevice();

		if (desc.bUseAsRenderTarget)
		{
			// Create RTV (Render Target View)

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc { };
			rtvDesc.Format = FormatToDX11Format(desc.Format, EDX11FormatUsage::RTV);
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;

			dxcall(device->CreateRenderTargetView(m_Texture, &rtvDesc, &m_RTV));
			DX11::SetDebugName(m_RTV, desc.DebugName, "RTV_");
		}

		if (desc.bUseAsDepthStencil)
		{
			// Create DSV (Depth Stencil View)

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc { };
			dsvDesc.Format = FormatToDX11Format(desc.Format, EDX11FormatUsage::DSV);
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;

			dxcall(device->CreateDepthStencilView(m_Texture, &dsvDesc, &m_DSV));
			DX11::SetDebugName(m_DSV, desc.DebugName, "DSV_");
		}
	}

	void DX11Texture::CreateSampler(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(m_Texture);

		HRESULT hResult;

		ID3D11Device* device = DX11::GetDevice();

		if (desc.bCreateSampler)
		{
			// Create SRV (Shader Resource View)

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc { };
			srvDesc.Format = FormatToDX11Format(desc.Format, EDX11FormatUsage::SRV);
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = -1;

			dxcall(device->CreateShaderResourceView(m_Texture, &srvDesc, &m_SRV));
			DX11::SetDebugName(m_SRV, desc.DebugName, "SRV_");

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

			dxcall(device->CreateSamplerState(&samplerDesc, &m_SamplerState));
			DX11::SetDebugName(m_SamplerState, desc.DebugName, "SamplerState_");
		}
	}

	void DX11Texture::ReleaseResources()
	{
		TRACE_FUNCTION();

		COMRelease(m_Texture);
		COMRelease(m_RTV);
		COMRelease(m_DSV);
		COMRelease(m_SRV);
		COMRelease(m_SamplerState);
	}
}
