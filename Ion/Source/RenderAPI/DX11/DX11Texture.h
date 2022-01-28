#pragma once

#include "DX11.h"
#include "Renderer/Texture.h"

namespace Ion
{
	class ION_API DX11Texture : public Texture
	{
	public:
		virtual ~DX11Texture() override;

		virtual void SetDimensions(TextureDimensions dimensions) override;
		virtual void UpdateSubresource(Image* image) override;

		virtual void Bind(uint32 slot = 0) const override;
		virtual void Unbind() const override;

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
			return D3D11_TEXTURE_ADDRESS_WRAP;
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
			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		}

	protected:
		DX11Texture(const TextureDescription& desc);

	private:
		void CreateTexture(const TextureDescription& desc);
		void CreateDepthStencilTexture(const TextureDescription& desc);
		void ReleaseTexture();

	private:
		ID3D11Texture2D* m_Texture;
		ID3D11ShaderResourceView* m_SRV;
		ID3D11RenderTargetView* m_RTV;
		ID3D11SamplerState* m_SamplerState;

		ID3D11Texture2D* m_DepthStencilTexture;
		ID3D11DepthStencilView* m_DSV;

		friend class Texture;
		friend class DX11Renderer;
	};
}
