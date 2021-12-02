#pragma once

#include "DX11.h"
#include "Renderer/Texture.h"

namespace Ion
{
	class ION_API DX11Texture : public Texture
	{
		friend class Texture;
	public:
		virtual ~DX11Texture() override;

		virtual void Bind(uint32 slot = 0) const override;
		virtual void Unbind() const override;

	protected:
		DX11Texture(Image* image);
		DX11Texture(AssetHandle asset);

	private:
		void CreateTexture(const void* const pixelData, uint32 width, uint32 height);

	private:
		uint32 m_ID;
		ID3D11Texture2D* m_Texture;
		ID3D11ShaderResourceView* m_SRV;
		ID3D11SamplerState* m_SamplerState;
	};
}
