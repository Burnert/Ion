#pragma once

#include "DX11.h"
#include "RHI/Texture.h"

namespace Ion
{
	enum class EDX11FormatUsage
	{
		Resource,
		RTV,
		DSV,
		SRV,
		AuxSRV,
	};

	class ION_API DX11Texture : public Texture
	{
	public:
		virtual ~DX11Texture() override;

		virtual void SetDimensions(TextureDimensions dimensions) override;
		virtual void UpdateSubresource(Image* image) override;

		virtual void Bind(uint32 slot = 0) const override;
		virtual void Unbind() const override;

		virtual void CopyTo(const TShared<Texture>& destination) const override;
		virtual void Map(void*& outBuffer, int32& outLineSize, ETextureMapType mapType) override;
		virtual void Unmap() override;

		virtual void* GetNativeID() const override;

		inline static constexpr D3D11_USAGE UsageToDX11Usage(ETextureUsage usage)
		{
			switch (usage)
			{
				case ETextureUsage::Default:    return D3D11_USAGE_DEFAULT;
				case ETextureUsage::Immutable:  return D3D11_USAGE_IMMUTABLE;
				case ETextureUsage::Dynamic:    return D3D11_USAGE_DYNAMIC;
				case ETextureUsage::Staging:    return D3D11_USAGE_STAGING;
			}
			ionassert(false, "Invalid usage.");
			return D3D11_USAGE_DEFAULT;
		}

		inline static constexpr D3D11_TEXTURE_ADDRESS_MODE WrapModeToDX11AddressMode(ETextureWrapMode mode)
		{
			switch (mode)
			{
				case ETextureWrapMode::Wrap:   return D3D11_TEXTURE_ADDRESS_WRAP;
				case ETextureWrapMode::Clamp:  return D3D11_TEXTURE_ADDRESS_CLAMP;
				case ETextureWrapMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
			}
			ionassert(false, "Invalid wrap mode.");
			return D3D11_TEXTURE_ADDRESS_WRAP;
		}

		inline static constexpr DXGI_FORMAT FormatToDX11Format(ETextureFormat format, EDX11FormatUsage usage)
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
					case EDX11FormatUsage::Resource: return DXGI_FORMAT_R24G8_TYPELESS;
					case EDX11FormatUsage::RTV:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					case EDX11FormatUsage::DSV:      return DXGI_FORMAT_D24_UNORM_S8_UINT;
					case EDX11FormatUsage::SRV:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					case EDX11FormatUsage::AuxSRV:   return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
					}
					break;
				}
				case ETextureFormat::UInt128GUID: return DXGI_FORMAT_R32G32B32A32_UINT;
			}
			ionassert(false, "Invalid format.");
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
			ionassert(false, "Invalid format.");
			return 0;
		}

		inline static constexpr D3D11_MAP MapTypeToDX11Map(ETextureMapType mapType)
		{
			switch (mapType)
			{
				case ETextureMapType::Read:      return D3D11_MAP_READ;
				case ETextureMapType::Write:     return D3D11_MAP_WRITE;
				case ETextureMapType::ReadWrite: return D3D11_MAP_READ_WRITE;
			}
			ionassert(false, "Invalid map type.");
			return D3D11_MAP_READ;
		}

		inline static constexpr D3D11_FILTER SelectDX11Filter(ETextureFilteringMethod minFilter, ETextureFilteringMethod magFilter, ETextureFilteringMethod mipFilter)
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
								case ETextureFilteringMethod::Linear:  return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
								case ETextureFilteringMethod::Nearest: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
							}
							break;
						}
						case ETextureFilteringMethod::Nearest:
						{
							switch (mipFilter)
							{
								case ETextureFilteringMethod::Linear:  return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
								case ETextureFilteringMethod::Nearest: return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
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
								case ETextureFilteringMethod::Linear:  return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
								case ETextureFilteringMethod::Nearest: return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
							}
							break;
						}
						case ETextureFilteringMethod::Nearest:
						{
							switch (mipFilter)
							{
								case ETextureFilteringMethod::Linear:  return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
								case ETextureFilteringMethod::Nearest: return D3D11_FILTER_MIN_MAG_MIP_POINT;
							}
							break;
						}
					}
					break;
				}
			}
			ionassert(false, "Invalid filter.");
			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		}

	protected:
		DX11Texture(const TextureDescription& desc);
		DX11Texture(const TextureDescription& desc, ID3D11Texture2D* existingResource);

	private:
		void CreateTexture(const TextureDescription& desc);
		void CreateViews(const TextureDescription& desc);
		void CreateSampler(const TextureDescription& desc);
		void ReleaseResources();

	private:
		ID3D11Texture2D* m_Texture;
		ID3D11RenderTargetView* m_RTV;
		ID3D11DepthStencilView* m_DSV;
		ID3D11ShaderResourceView* m_SRV;
		ID3D11SamplerState* m_SamplerState;

		friend class Texture;
		friend class DX11Renderer;
		friend class DX11;
	};
}
