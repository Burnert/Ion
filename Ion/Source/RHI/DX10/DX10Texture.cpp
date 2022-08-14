#include "IonPCH.h"

#include "DX10Texture.h"

namespace Ion
{
	DX10Texture::~DX10Texture()
	{
		TRACE_FUNCTION();

		ReleaseResources();
	}

	void DX10Texture::SetDimensions(TextureDimensions dimensions)
	{
		//ionassert(dimensions.Width <= 16384 && dimensions.Height <= 16384, "Textures larger than 16K are unsupported.");
		// @TODO: This probably should not be here (recreate the texture instead)
	}

	void DX10Texture::UpdateSubresource(Image* image)
	{
		TRACE_FUNCTION();

		ionassert(m_Description.bUseAsRenderTarget,
			"Cannot update subresource if the texture has not been created as a render target.");

		if (!image->IsLoaded())
		{
			DX10Logger.Error("Cannot Update Subresource of Texture. Image has not been loaded.");
			return;
		}

		ID3D10Device* device = DX10::GetDevice();

		// Dimensions are different
		if (image->GetWidth() != m_Description.Dimensions.Width ||
			image->GetHeight() != m_Description.Dimensions.Height)
		{
			DX10Logger.Warn("Image dimensions do not match texture dimensions.");
			return;
		}

		uint32 lineSize = image->GetWidth() * 4;

		// Update Subresource
		dxcall_v(device->UpdateSubresource(m_Texture, 0, nullptr, image->GetPixelData(), lineSize, 0));

		if (m_Description.bGenerateMips)
		{
			// Generate mipmaps
			dxcall_v(device->GenerateMips(m_SRV));
		}
	}

	void DX10Texture::Bind(uint32 slot) const
	{
		ionassert(m_SRV);
		ionassert(m_SamplerState);

		ID3D10Device* device = DX10::GetDevice();

		dxcall_v(device->PSSetShaderResources(slot, 1, &m_SRV));
		dxcall_v(device->PSSetSamplers(slot, 1, &m_SamplerState));
	}

	void DX10Texture::Unbind() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall_v(device->PSSetShaderResources(0, 0, nullptr));
		dxcall_v(device->PSSetSamplers(0, 0, nullptr));
	}

	void DX10Texture::CopyTo(const TShared<RHITexture>& destination) const
	{
		ID3D10Device* device = DX10::GetDevice();

		TShared<DX10Texture> destTexture = TStaticCast<DX10Texture>(destination);
		dxcall_v(device->CopyResource(destTexture->m_Texture, m_Texture));
	}

	void DX10Texture::Map(void*& outBuffer, int32& outLineSize, ETextureMapType mapType)
	{
		HRESULT hResult;

		D3D10_MAPPED_TEXTURE2D mt2d { };

		dxcall(m_Texture->Map(0, MapTypeToDX10Map(mapType), 0, &mt2d));

		outBuffer = mt2d.pData;
		outLineSize = mt2d.RowPitch;
	}

	void DX10Texture::Unmap()
	{
		dxcall_v(m_Texture->Unmap(0));
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
		CreateTexture(desc);
		// Can't do any of this if this is a staging texture
		if (desc.Usage != ETextureUsage::Staging)
		{
			CreateViews(desc);
			CreateSampler(desc);
		}

		DX10Logger.Trace("Created DX10Texture object \"{}\".", desc.DebugName);
	}

	DX10Texture::DX10Texture(const TextureDescription& desc, ID3D10Texture2D* existingResource) :
		RHITexture(desc),
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

		DX10Logger.Trace("Created DX10Texture object \"{}\".", desc.DebugName);
	}

	static bool IsMultiSampled(const TextureDescription& desc)
	{
		return desc.MultiSampling != ETextureMSMode::Default && desc.MultiSampling != ETextureMSMode::X1;
	}

	void DX10Texture::CreateTexture(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(!IsMultiSampled(desc) || !desc.InitialData,
			"Multisampled textures have to be initialized using UpdateSubresource function");

		HRESULT hResult;

		ID3D10Device* device = DX10::GetDevice();

		uint32 lineSize = desc.Dimensions.Width * 4;

		// Create Texture2D

		DXGI_FORMAT resourceFormat = DXCommon::TextureFormatToDXGIFormat(desc.Format, EDX10FormatUsage::Resource);

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
	}

	void DX10Texture::CreateViews(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(m_Texture);

		HRESULT hResult;

		ID3D10Device* device = DX10::GetDevice();

		if (desc.bUseAsRenderTarget)
		{
			// Create RTV (Render Target View)

			D3D10_RENDER_TARGET_VIEW_DESC rtvDesc { };
			rtvDesc.Format = FormatToDX10Format(desc.Format, EDX10FormatUsage::RTV);
			rtvDesc.ViewDimension = IsMultiSampled(desc) ?
				D3D10_RTV_DIMENSION_TEXTURE2DMS :
				D3D10_RTV_DIMENSION_TEXTURE2D;
			if (!IsMultiSampled(desc))
			{
				rtvDesc.Texture2D.MipSlice = 0;
			}

			dxcall(device->CreateRenderTargetView(m_Texture, &rtvDesc, &m_RTV));
			DX10::SetDebugName(m_RTV, "RTV_"+ desc.DebugName);
		}

		if (desc.bUseAsDepthStencil)
		{
			// Create DSV (Depth Stencil View)

			D3D10_DEPTH_STENCIL_VIEW_DESC dsvDesc { };
			dsvDesc.Format = FormatToDX10Format(desc.Format, EDX10FormatUsage::DSV);
			dsvDesc.ViewDimension = IsMultiSampled(desc) ?
				D3D10_DSV_DIMENSION_TEXTURE2DMS :
				D3D10_DSV_DIMENSION_TEXTURE2D;
			if (!IsMultiSampled(desc))
			{
				dsvDesc.Texture2D.MipSlice = 0;
			}

			dxcall(device->CreateDepthStencilView(m_Texture, &dsvDesc, &m_DSV));
			DX10::SetDebugName(m_DSV, "DSV_" + desc.DebugName);
		}
	}

	void DX10Texture::CreateSampler(const TextureDescription& desc)
	{
		TRACE_FUNCTION();

		ionassert(m_Texture);

		HRESULT hResult;

		ID3D10Device* device = DX10::GetDevice();

		if (desc.bCreateSampler)
		{
			// Create SRV (Shader Resource View)

			D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc { };
			srvDesc.Format = FormatToDX10Format(desc.Format, EDX10FormatUsage::SRV);
			srvDesc.ViewDimension = IsMultiSampled(desc) ?
				D3D10_SRV_DIMENSION_TEXTURE2DMS :
				D3D10_SRV_DIMENSION_TEXTURE2D;
			if (!IsMultiSampled(desc))
			{
				srvDesc.Texture2D.MipLevels = -1;
			}

			dxcall(device->CreateShaderResourceView(m_Texture, &srvDesc, &m_SRV));
			DX10::SetDebugName(m_SRV, "SRV_" + desc.DebugName);

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
		}
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
