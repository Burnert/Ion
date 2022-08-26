#include "IonPCH.h"

#include "RHI/RHICore.h"

#if RHI_BUILD_DX11

#include "DX11Texture.h"

namespace Ion
{
	DX11Texture::~DX11Texture()
	{
		TRACE_FUNCTION();

		ReleaseResources();
		DX11Logger.Info("DX11Texture \"{}\" has been destroyed.", m_Description.DebugName);
	}

	Result<void, RHIError> DX11Texture::SetDimensions(TextureDimensions dimensions)
	{
		//ionassert(dimensions.Width <= 16384 && dimensions.Height <= 16384, "Textures larger than 16K are unsupported.");
		// @TODO: This probably should not be here (recreate the texture instead)

		return Ok();
	}

	Result<void, RHIError> DX11Texture::UpdateSubresource(Image* image)
	{
		TRACE_FUNCTION();

		ionassert(m_Description.bUseAsRenderTarget,
			"Cannot update subresource if the texture has not been created as a render target.");

		ionassert(image->IsLoaded(), "Cannot Update Subresource of Texture. Image has not been loaded.");

		ionassert(
			image->GetWidth() == m_Description.Dimensions.Width &&
			image->GetHeight() == m_Description.Dimensions.Height,
			"Image dimensions do not match texture dimensions.");

		ID3D11DeviceContext* context = DX11::GetContext();

		uint32 lineSize = image->GetWidth() * 4;

		// Update Subresource
		dxcall(context->UpdateSubresource(m_Texture, 0, nullptr, image->GetPixelData(), lineSize, 0));

		if (m_Description.bGenerateMips)
		{
			// Generate mipmaps
			dxcall(context->GenerateMips(m_SRV));
		}

		return Ok();
	}

	Result<void, RHIError> DX11Texture::Bind(uint32 slot) const
	{
		ionassert(m_SRV);
		ionassert(m_SamplerState);

		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall(context->PSSetShaderResources(slot, 1, &m_SRV));
		dxcall(context->PSSetSamplers(slot, 1, &m_SamplerState));

		return Ok();
	}

	Result<void, RHIError> DX11Texture::Unbind() const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall(context->PSSetShaderResources(0, 0, nullptr));
		dxcall(context->PSSetSamplers(0, 0, nullptr));

		return Ok();
	}

	Result<void, RHIError> DX11Texture::CopyTo(const TRef<RHITexture>& destination) const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		TRef<DX11Texture> destTexture = RefCast<DX11Texture>(destination);
		dxcall(context->CopyResource(destTexture->m_Texture, m_Texture));

		return Ok();
	}

	Result<void, RHIError> DX11Texture::Map(void*& outBuffer, int32& outLineSize, ETextureMapType mapType)
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		D3D11_MAPPED_SUBRESOURCE msr { };

		dxcall(context->Map(m_Texture, 0, MapTypeToDX11Map(mapType), 0, &msr));

		outBuffer = msr.pData;
		outLineSize = msr.RowPitch;

		return Ok();
	}

	Result<void, RHIError> DX11Texture::Unmap()
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall(context->Unmap(m_Texture, 0));

		return Ok();
	}

	void* DX11Texture::GetNativeID() const
	{
		return m_SRV;
	}

	DX11Texture::DX11Texture(const TextureDescription& desc) :
		RHITexture(desc),
		m_Texture(nullptr),
		m_RTV(nullptr),
		m_DSV(nullptr),
		m_SRV(nullptr),
		m_SamplerState(nullptr)
	{
		DX11Logger.Info("DX11Texture \"{}\" has been created.", desc.DebugName);

		CreateTexture(desc)
			.Err([&](Error& error) { DX11Logger.Critical("{}: Cannot create a Texture.\n{}", desc.DebugName, error.Message); })
			.Unwrap();
		// Can't do any of this if this is a staging texture
		if (desc.Usage != ETextureUsage::Staging)
		{
			CreateViews(desc)
				.Err([&](Error& error) { DX11Logger.Critical("{}: Cannot create Views.\n{}", desc.DebugName, error.Message); })
				.Unwrap();
			CreateSampler(desc)
				.Err([&](Error& error) { DX11Logger.Critical("{}: Cannot create a Sampler.\n{}", desc.DebugName, error.Message); })
				.Unwrap();
		}
	}

	DX11Texture::DX11Texture(const TextureDescription& desc, ID3D11Texture2D* existingResource) :
		RHITexture(desc),
		m_Texture(existingResource),
		m_RTV(nullptr),
		m_DSV(nullptr),
		m_SRV(nullptr),
		m_SamplerState(nullptr)
	{
		DX11Logger.Info("DX11Texture \"{}\" has been created.", desc.DebugName);

		// Can't do any of this if this is a staging texture
		if (desc.Usage != ETextureUsage::Staging)
		{
			CreateViews(desc)
				.Err([&](Error& error) { DX11Logger.Critical("{}: Cannot create Views.\n{}", desc.DebugName, error.Message); })
				.Unwrap();
			CreateSampler(desc)
				.Err([&](Error& error) { DX11Logger.Critical("{}: Cannot create a Sampler.\n{}", desc.DebugName, error.Message); })
				.Unwrap();
		}
	}

	static bool IsMultiSampled(const TextureDescription& desc)
	{
		return desc.MultiSampling != ETextureMSMode::Default && desc.MultiSampling != ETextureMSMode::X1;
	}

	Result<void, RHIError> DX11Texture::CreateTexture(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(!IsMultiSampled(desc) || !desc.InitialData,
			"Multisampled textures have to be initialized using UpdateSubresource function");

		ID3D11Device* device = DX11::GetDevice();
		ID3D11DeviceContext* context = DX11::GetContext();

		uint32 lineSize = desc.Dimensions.Width * 4;

		// Create Texture2D

		DXGI_FORMAT resourceFormat = DXCommon::TextureFormatToDXGIFormat(desc.Format, EDXTextureFormatUsage::Resource);

		int32 mipLevels = desc.bGenerateMips ? 0 : 1;
		int32 sampleCount = (bool)desc.MultiSampling ? (int32)desc.MultiSampling : 1;

		uint32 availQuality;
		dxcall(device->CheckMultisampleQualityLevels(resourceFormat, sampleCount, &availQuality));

		D3D11_TEXTURE2D_DESC tex2DDesc { };
		tex2DDesc.Width = desc.Dimensions.Width;
		tex2DDesc.Height = desc.Dimensions.Height;
		tex2DDesc.MipLevels = mipLevels;
		tex2DDesc.ArraySize = 1;
		tex2DDesc.Format = resourceFormat;
		tex2DDesc.Usage = UsageToDX11Usage(desc.Usage);
		tex2DDesc.SampleDesc.Count = sampleCount;

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

		DX11Logger.Debug("DX11Texture Texture2D object has been created.");

		return Ok();
	}

	Result<void, RHIError> DX11Texture::CreateViews(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(m_Texture);

		ID3D11Device* device = DX11::GetDevice();

		if (desc.bUseAsRenderTarget)
		{
			// Create RTV (Render Target View)

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc { };
			rtvDesc.Format = DXCommon::TextureFormatToDXGIFormat(desc.Format, EDXTextureFormatUsage::RTV);
			rtvDesc.ViewDimension = IsMultiSampled(desc) ?
				D3D11_RTV_DIMENSION_TEXTURE2DMS :
				D3D11_RTV_DIMENSION_TEXTURE2D;
			if (!IsMultiSampled(desc))
			{
				rtvDesc.Texture2D.MipSlice = 0;
			}

			dxcall(device->CreateRenderTargetView(m_Texture, &rtvDesc, &m_RTV));
			DX11::SetDebugName(m_RTV, desc.DebugName, "RTV_");

			DX11Logger.Debug("DX11Texture Render Target View object has been created.");
		}

		if (desc.bUseAsDepthStencil)
		{
			// Create DSV (Depth Stencil View)

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc { };
			dsvDesc.Format = DXCommon::TextureFormatToDXGIFormat(desc.Format, EDXTextureFormatUsage::DSV);
			dsvDesc.ViewDimension = IsMultiSampled(desc) ?
				D3D11_DSV_DIMENSION_TEXTURE2DMS :
				D3D11_DSV_DIMENSION_TEXTURE2D;
			if (!IsMultiSampled(desc))
			{
				dsvDesc.Texture2D.MipSlice = 0;
			}

			dxcall(device->CreateDepthStencilView(m_Texture, &dsvDesc, &m_DSV));
			DX11::SetDebugName(m_DSV, desc.DebugName, "DSV_");

			DX11Logger.Debug("DX11Texture Depth Stencil View object has been created.");
		}

		return Ok();
	}

	Result<void, RHIError> DX11Texture::CreateSampler(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(m_Texture);

		ID3D11Device* device = DX11::GetDevice();

		if (desc.bCreateSampler)
		{
			// Create SRV (Shader Resource View)

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc { };
			srvDesc.Format = DXCommon::TextureFormatToDXGIFormat(desc.Format, EDXTextureFormatUsage::SRV);
			srvDesc.ViewDimension = IsMultiSampled(desc) ?
				D3D11_SRV_DIMENSION_TEXTURE2DMS :
				D3D11_SRV_DIMENSION_TEXTURE2D;
			if (!IsMultiSampled(desc))
			{
				srvDesc.Texture2D.MipLevels = -1;
			}

			dxcall(device->CreateShaderResourceView(m_Texture, &srvDesc, &m_SRV));
			DX11::SetDebugName(m_SRV, desc.DebugName, "SRV_");

			DX11Logger.Debug("DX11Texture Shader Resource View object has been created.");

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

			DX11Logger.Debug("DX11Texture Sampler State object has been created.");
		}

		return Ok();
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

#endif // RHI_BUILD_DX11
