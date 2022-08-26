#include "IonPCH.h"

#include "RHI/RHICore.h"

#if RHI_BUILD_DX10

#include "DX10Texture.h"

namespace Ion
{
	DX10Texture::~DX10Texture()
	{
		TRACE_FUNCTION();

		ReleaseResources();
		DX10Logger.Info("DX10Texture \"{}\" has been destroyed.", m_Description.DebugName);
	}

	Result<void, RHIError> DX10Texture::SetDimensions(TextureDimensions dimensions)
	{
		//ionassert(dimensions.Width <= 16384 && dimensions.Height <= 16384, "Textures larger than 16K are unsupported.");
		// @TODO: This probably should not be here (recreate the texture instead)

		return Ok();
	}

	Result<void, RHIError> DX10Texture::UpdateSubresource(Image* image)
	{
		TRACE_FUNCTION();

		ionassert(m_Description.bUseAsRenderTarget,
			"Cannot update subresource if the texture has not been created as a render target.");

		ionassert(image->IsLoaded(), "Cannot Update Subresource of Texture. Image has not been loaded.");

		ionassert(
			image->GetWidth() == m_Description.Dimensions.Width &&
			image->GetHeight() == m_Description.Dimensions.Height,
			"Image dimensions do not match texture dimensions.");

		ID3D10Device* device = DX10::GetDevice();

		uint32 lineSize = image->GetWidth() * 4;

		// Update Subresource
		dxcall(device->UpdateSubresource(m_Texture, 0, nullptr, image->GetPixelData(), lineSize, 0));

		if (m_Description.bGenerateMips)
		{
			// Generate mipmaps
			dxcall(device->GenerateMips(m_SRV));
		}

		return Ok();
	}

	Result<void, RHIError> DX10Texture::Bind(uint32 slot) const
	{
		ionassert(m_SRV);
		ionassert(m_SamplerState);

		ID3D10Device* device = DX10::GetDevice();

		dxcall(device->PSSetShaderResources(slot, 1, &m_SRV));
		dxcall(device->PSSetSamplers(slot, 1, &m_SamplerState));

		return Ok();
	}

	Result<void, RHIError> DX10Texture::Unbind() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall(device->PSSetShaderResources(0, 0, nullptr));
		dxcall(device->PSSetSamplers(0, 0, nullptr));

		return Ok();
	}

	Result<void, RHIError> DX10Texture::CopyTo(const TRef<RHITexture>& destination) const
	{
		ID3D10Device* device = DX10::GetDevice();

		TRef<DX10Texture> destTexture = RefCast<DX10Texture>(destination);
		dxcall(device->CopyResource(destTexture->m_Texture, m_Texture));

		return Ok();
	}

	Result<void, RHIError> DX10Texture::Map(void*& outBuffer, int32& outLineSize, ETextureMapType mapType)
	{
		D3D10_MAPPED_TEXTURE2D mt2d { };

		dxcall(m_Texture->Map(0, MapTypeToDX10Map(mapType), 0, &mt2d));

		outBuffer = mt2d.pData;
		outLineSize = mt2d.RowPitch;

		return Ok();
	}

	Result<void, RHIError> DX10Texture::Unmap()
	{
		dxcall(m_Texture->Unmap(0));

		return Ok();
	}

	void* DX10Texture::GetNativeID() const
	{
		return m_SRV;
	}

	DX10Texture::DX10Texture(const TextureDescription& desc) :
		RHITexture(desc),
		m_Texture(nullptr),
		m_RTV(nullptr),
		m_DSV(nullptr),
		m_SRV(nullptr),
		m_SamplerState(nullptr)
	{
		DX10Logger.Info("DX10Texture \"{}\" has been created.", desc.DebugName);

		CreateTexture(desc)
			.Err([&](Error& error) { DX10Logger.Critical("{}: Cannot create a Texture.\n{}", desc.DebugName, error.Message); })
			.Unwrap();
		// Can't do any of this if this is a staging texture
		if (desc.Usage != ETextureUsage::Staging)
		{
			CreateViews(desc)
				.Err([&](Error& error) { DX10Logger.Critical("{}: Cannot create Views.\n{}", desc.DebugName, error.Message); })
				.Unwrap();
			CreateSampler(desc)
				.Err([&](Error& error) { DX10Logger.Critical("{}: Cannot create a Sampler.\n{}", desc.DebugName, error.Message); })
				.Unwrap();
		}
	}

	DX10Texture::DX10Texture(const TextureDescription& desc, ID3D10Texture2D* existingResource) :
		RHITexture(desc),
		m_Texture(existingResource),
		m_RTV(nullptr),
		m_DSV(nullptr),
		m_SRV(nullptr),
		m_SamplerState(nullptr)
	{
		DX10Logger.Info("DX10Texture \"{}\" has been created.", desc.DebugName);

		// Can't do any of this if this is a staging texture
		if (desc.Usage != ETextureUsage::Staging)
		{
			CreateViews(desc)
				.Err([&](Error& error) { DX10Logger.Critical("{}: Cannot create Views.\n{}", desc.DebugName, error.Message); })
				.Unwrap();
			CreateSampler(desc)
				.Err([&](Error& error) { DX10Logger.Critical("{}: Cannot create a Sampler.\n{}", desc.DebugName, error.Message); })
				.Unwrap();
		}
	}

	static bool IsMultiSampled(const TextureDescription& desc)
	{
		return desc.MultiSampling != ETextureMSMode::Default && desc.MultiSampling != ETextureMSMode::X1;
	}

	Result<void, RHIError> DX10Texture::CreateTexture(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(!IsMultiSampled(desc) || !desc.InitialData,
			"Multisampled textures have to be initialized using UpdateSubresource function");

		ID3D10Device* device = DX10::GetDevice();

		uint32 lineSize = desc.Dimensions.Width * 4;

		// Create Texture2D

		DXGI_FORMAT resourceFormat = DXCommon::TextureFormatToDXGIFormat(desc.Format, EDXTextureFormatUsage::Resource);

		int32 mipLevels = desc.bGenerateMips ? 0 : 1;
		int32 sampleCount = (bool)desc.MultiSampling ? (int32)desc.MultiSampling : 1;

		uint32 availQuality;
		dxcall(device->CheckMultisampleQualityLevels(resourceFormat, sampleCount, &availQuality));

		D3D10_TEXTURE2D_DESC tex2DDesc { };
		tex2DDesc.Width = desc.Dimensions.Width;
		tex2DDesc.Height = desc.Dimensions.Height;
		tex2DDesc.MipLevels = mipLevels;
		tex2DDesc.ArraySize = 1;
		tex2DDesc.Format = resourceFormat;
		tex2DDesc.Usage = UsageToDX10Usage(desc.Usage);
		tex2DDesc.SampleDesc.Count = sampleCount;

		tex2DDesc.BindFlags =
			FlagsIf(desc.bCreateSampler &&
				desc.Usage != ETextureUsage::Staging, D3D10_BIND_SHADER_RESOURCE) |
			FlagsIf(desc.bUseAsRenderTarget, D3D10_BIND_RENDER_TARGET) |
			FlagsIf(desc.bUseAsDepthStencil, D3D10_BIND_DEPTH_STENCIL);

		tex2DDesc.CPUAccessFlags =
			FlagsIf(desc.bAllowCPUReadAccess, D3D10_CPU_ACCESS_READ) |
			FlagsIf(desc.bAllowCPUWriteAccess, D3D10_CPU_ACCESS_WRITE);

		tex2DDesc.MiscFlags =
			FlagsIf(desc.bGenerateMips, D3D10_RESOURCE_MISC_GENERATE_MIPS);

		D3D10_SUBRESOURCE_DATA sd { };

		ionassert(desc.Usage != ETextureUsage::Immutable || desc.InitialData);
		if (desc.InitialData)
		{
			sd.pSysMem = desc.InitialData;
			sd.SysMemPitch = desc.Dimensions.Width * 4;
		}

		dxcall(device->CreateTexture2D(&tex2DDesc, desc.InitialData ? &sd : nullptr, &m_Texture));
		DX10::SetDebugName(m_Texture, "Texture2D_" + desc.DebugName);

		DX10Logger.Debug("DX10Texture Texture2D object has been created.");

		return Ok();
	}

	Result<void, RHIError> DX10Texture::CreateViews(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(m_Texture);

		ID3D10Device* device = DX10::GetDevice();

		if (desc.bUseAsRenderTarget)
		{
			// Create RTV (Render Target View)

			D3D10_RENDER_TARGET_VIEW_DESC rtvDesc { };
			rtvDesc.Format = DXCommon::TextureFormatToDXGIFormat(desc.Format, EDXTextureFormatUsage::RTV);
			rtvDesc.ViewDimension = IsMultiSampled(desc) ?
				D3D10_RTV_DIMENSION_TEXTURE2DMS :
				D3D10_RTV_DIMENSION_TEXTURE2D;
			if (!IsMultiSampled(desc))
			{
				rtvDesc.Texture2D.MipSlice = 0;
			}

			dxcall(device->CreateRenderTargetView(m_Texture, &rtvDesc, &m_RTV));
			DX10::SetDebugName(m_RTV, "RTV_"+ desc.DebugName);

			DX10Logger.Debug("DX10Texture Render Target View object has been created.");
		}

		if (desc.bUseAsDepthStencil)
		{
			// Create DSV (Depth Stencil View)

			D3D10_DEPTH_STENCIL_VIEW_DESC dsvDesc { };
			dsvDesc.Format = DXCommon::TextureFormatToDXGIFormat(desc.Format, EDXTextureFormatUsage::DSV);
			dsvDesc.ViewDimension = IsMultiSampled(desc) ?
				D3D10_DSV_DIMENSION_TEXTURE2DMS :
				D3D10_DSV_DIMENSION_TEXTURE2D;
			if (!IsMultiSampled(desc))
			{
				dsvDesc.Texture2D.MipSlice = 0;
			}

			dxcall(device->CreateDepthStencilView(m_Texture, &dsvDesc, &m_DSV));
			DX10::SetDebugName(m_DSV, "DSV_" + desc.DebugName);

			DX10Logger.Debug("DX10Texture Depth Stencil View object has been created.");
		}

		return Ok();
	}

	Result<void, RHIError> DX10Texture::CreateSampler(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(m_Texture);

		ID3D10Device* device = DX10::GetDevice();

		if (desc.bCreateSampler)
		{
			// Create SRV (Shader Resource View)

			D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc { };
			srvDesc.Format = DXCommon::TextureFormatToDXGIFormat(desc.Format, EDXTextureFormatUsage::SRV);
			srvDesc.ViewDimension = IsMultiSampled(desc) ?
				D3D10_SRV_DIMENSION_TEXTURE2DMS :
				D3D10_SRV_DIMENSION_TEXTURE2D;
			if (!IsMultiSampled(desc))
			{
				srvDesc.Texture2D.MipLevels = -1;
			}

			dxcall(device->CreateShaderResourceView(m_Texture, &srvDesc, &m_SRV));
			DX10::SetDebugName(m_SRV, "SRV_" + desc.DebugName);

			DX10Logger.Debug("DX10Texture Shader Resource View object has been created.");

			// Create Sampler State

			D3D10_SAMPLER_DESC samplerDesc { };
			samplerDesc.Filter = SelectDX10Filter(desc.MinFilter, desc.MagFilter, desc.MipFilter);
			samplerDesc.AddressU = WrapModeToDX10AddressMode(desc.UWrapMode);
			samplerDesc.AddressV = WrapModeToDX10AddressMode(desc.VWrapMode);
			samplerDesc.AddressW = WrapModeToDX10AddressMode(desc.WWrapMode);
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D10_COMPARISON_ALWAYS;
			samplerDesc.MipLODBias = desc.LODBias;
			samplerDesc.MinLOD = -D3D10_FLOAT32_MAX;
			samplerDesc.MaxLOD = D3D10_FLOAT32_MAX;

			dxcall(device->CreateSamplerState(&samplerDesc, &m_SamplerState));
			DX10::SetDebugName(m_SamplerState, "SamplerState_" + desc.DebugName);

			DX10Logger.Debug("DX10Texture Sampler State object has been created.");
		}

		return Ok();
	}

	void DX10Texture::ReleaseResources()
	{
		TRACE_FUNCTION();

		COMRelease(m_Texture);
		COMRelease(m_RTV);
		COMRelease(m_DSV);
		COMRelease(m_SRV);
		COMRelease(m_SamplerState);
	}
}

#endif // RHI_BUILD_DX10
