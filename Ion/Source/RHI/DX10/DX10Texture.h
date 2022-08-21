#pragma once

#include "DX10.h"
#include "RHI/Texture.h"

namespace Ion
{
	enum class EDX10FormatUsage
	{
		Resource,
		RTV,
		DSV,
		SRV,
		AuxSRV,
	};

	class ION_API DX10Texture : public RHITexture
	{
	public:
		virtual ~DX10Texture() override;

		virtual Result<void, RHIError> SetDimensions(TextureDimensions dimensions) override;
		virtual Result<void, RHIError> UpdateSubresource(Image* image) override;

		virtual Result<void, RHIError> Bind(uint32 slot = 0) const override;
		virtual Result<void, RHIError> Unbind() const override;

		virtual Result<void, RHIError> CopyTo(const TShared<RHITexture>& destination) const override;
		virtual Result<void, RHIError> Map(void*& outBuffer, int32& outLineSize, ETextureMapType mapType) override;
		virtual Result<void, RHIError> Unmap() override;

		virtual void* GetNativeID() const override;

		inline static constexpr D3D10_USAGE UsageToDX10Usage(ETextureUsage usage)
		{
			switch (usage)
			{
				case ETextureUsage::Default:    return D3D10_USAGE_DEFAULT;
				case ETextureUsage::Immutable:  return D3D10_USAGE_IMMUTABLE;
				case ETextureUsage::Dynamic:    return D3D10_USAGE_DYNAMIC;
				case ETextureUsage::Staging:    return D3D10_USAGE_STAGING;
			}
			ionbreak("Invalid usage.");
			return D3D10_USAGE_DEFAULT;
		}

		inline static constexpr D3D10_TEXTURE_ADDRESS_MODE WrapModeToDX10AddressMode(ETextureWrapMode mode)
		{
			switch (mode)
			{
				case ETextureWrapMode::Wrap:   return D3D10_TEXTURE_ADDRESS_WRAP;
				case ETextureWrapMode::Clamp:  return D3D10_TEXTURE_ADDRESS_CLAMP;
				case ETextureWrapMode::Mirror: return D3D10_TEXTURE_ADDRESS_MIRROR;
			}
			ionbreak("Invalid wrap mode.");
			return D3D10_TEXTURE_ADDRESS_WRAP;
		}

		inline static constexpr DXGI_FORMAT FormatToDX10Format(ETextureFormat format, EDX10FormatUsage usage)
		{
			switch (format)
			{
				case ETextureFormat::RGBA8:       return DXGI_FORMAT_R8G8B8A8_UNORM;
				case ETextureFormat::RGBA10:      return DXGI_FORMAT_R10G10B10A2_UNORM;
				case ETextureFormat::RGBAFloat32: return DXGI_FORMAT_R32G32B32A32_FLOAT;
				case ETextureFormat::Float32:     return DXGI_FORMAT_R32_FLOAT;
				case ETextureFormat::UInt32:      return DXGI_FORMAT_R32_UINT;
				case ETextureFormat::D24S8:
				{
					switch (usage)
					{
					case EDX10FormatUsage::Resource: return DXGI_FORMAT_R24G8_TYPELESS;
					case EDX10FormatUsage::RTV:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					case EDX10FormatUsage::DSV:      return DXGI_FORMAT_D24_UNORM_S8_UINT;
					case EDX10FormatUsage::SRV:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					case EDX10FormatUsage::AuxSRV:   return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
					}
					break;
				}
				case ETextureFormat::UInt128GUID: return DXGI_FORMAT_R32G32B32A32_UINT;
			}
			ionbreak("Invalid format.");
			return DXGI_FORMAT_UNKNOWN;
		}

		/* Returns bytes per pixel for a format. */
		inline static constexpr int32 GetFormatPixelSize(ETextureFormat format)
		{
			switch (format)
			{
				case ETextureFormat::RGBA8:       return 4;
				case ETextureFormat::RGBAFloat32: return 16;
				case ETextureFormat::UInt32:      return 4;
				case ETextureFormat::UInt128GUID: return 16;
			}
			ionbreak("Invalid format.");
			return 0;
		}

		inline static constexpr D3D10_MAP MapTypeToDX10Map(ETextureMapType mapType)
		{
			switch (mapType)
			{
				case ETextureMapType::Read:      return D3D10_MAP_READ;
				case ETextureMapType::Write:     return D3D10_MAP_WRITE;
				case ETextureMapType::ReadWrite: return D3D10_MAP_READ_WRITE;
			}
			ionbreak("Invalid map type.");
			return D3D10_MAP_READ;
		}

		inline static constexpr D3D10_FILTER SelectDX10Filter(ETextureFilteringMethod minFilter, ETextureFilteringMethod magFilter, ETextureFilteringMethod mipFilter)
		{
			switch (minFilter)
			{
				case ETextureFilteringMethod::Linear:
				{
					switch (magFilter)
					{
						case ETextureFilteringMethod::Linear:
						{
							switch (mipFilter)
							{
								case ETextureFilteringMethod::Linear:  return D3D10_FILTER_MIN_MAG_MIP_LINEAR;
								case ETextureFilteringMethod::Nearest: return D3D10_FILTER_MIN_MAG_LINEAR_MIP_POINT;
							}
							break;
						}
						case ETextureFilteringMethod::Nearest:
						{
							switch (mipFilter)
							{
								case ETextureFilteringMethod::Linear:  return D3D10_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
								case ETextureFilteringMethod::Nearest: return D3D10_FILTER_MIN_LINEAR_MAG_MIP_POINT;
							}
							break;
						}
					}
					break;
				}
				case ETextureFilteringMethod::Nearest:
				{
					switch (magFilter)
					{
						case ETextureFilteringMethod::Linear:
						{
							switch (mipFilter)
							{
								case ETextureFilteringMethod::Linear:  return D3D10_FILTER_MIN_POINT_MAG_MIP_LINEAR;
								case ETextureFilteringMethod::Nearest: return D3D10_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
							}
							break;
						}
						case ETextureFilteringMethod::Nearest:
						{
							switch (mipFilter)
							{
								case ETextureFilteringMethod::Linear:  return D3D10_FILTER_MIN_MAG_POINT_MIP_LINEAR;
								case ETextureFilteringMethod::Nearest: return D3D10_FILTER_MIN_MAG_MIP_POINT;
							}
							break;
						}
					}
					break;
				}
			}
			ionbreak("Invalid filter.");
			return D3D10_FILTER_MIN_MAG_MIP_POINT;
		}

	protected:
		DX10Texture(const TextureDescription& desc);
		DX10Texture(const TextureDescription& desc, ID3D10Texture2D* existingResource);

	private:
		Result<void, RHIError> CreateTexture(const TextureDescription& desc);
		Result<void, RHIError> CreateViews(const TextureDescription& desc);
		Result<void, RHIError> CreateSampler(const TextureDescription& desc);
		void ReleaseResources();

	private:
		ID3D10Texture2D* m_Texture;
		ID3D10RenderTargetView* m_RTV;
		ID3D10DepthStencilView* m_DSV;
		ID3D10ShaderResourceView* m_SRV;
		ID3D10SamplerState* m_SamplerState;

		friend class RHITexture;
		friend class DX10Renderer;
		friend class DX10;
	};
}
