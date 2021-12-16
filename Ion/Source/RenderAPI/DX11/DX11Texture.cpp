#include "IonPCH.h"

#include "DX11Texture.h"

namespace Ion
{
	DX11Texture::~DX11Texture()
	{
		TRACE_FUNCTION();

		COMRelease(m_Texture);
		COMRelease(m_SRV);
		COMRelease(m_SamplerState);
	}

	void DX11Texture::Bind(uint32 slot) const
	{
		TRACE_FUNCTION();

		dxcall_v(DX11::GetContext()->PSSetShaderResources(slot, 1, &m_SRV));
		dxcall_v(DX11::GetContext()->PSSetSamplers(slot, 1, &m_SamplerState));
	}

	void DX11Texture::Unbind() const
	{
		TRACE_FUNCTION();

		dxcall_v(DX11::GetContext()->PSSetShaderResources(0, 0, nullptr));
		dxcall_v(DX11::GetContext()->PSSetSamplers(0, 0, nullptr));
	}

	DX11Texture::DX11Texture(Image* image) :
		Texture(image)
	{
		TRACE_FUNCTION();

		CreateTexture(image->GetPixelData(), image->GetWidth(), image->GetHeight());
	}

	DX11Texture::DX11Texture(AssetHandle asset) :
		Texture(asset)
	{
		const AssetDescription::Texture* desc = asset->GetDescription<EAssetType::Texture>();
		CreateTexture(asset->Data(), desc->Width, desc->Height);
	}

	void DX11Texture::CreateTexture(const void* const pixelData, uint32 width, uint32 height)
	{
		TRACE_FUNCTION();

		ionassert(m_TextureAsset->IsLoaded(), "Texture has not been loaded yet.");

		HRESULT hResult;

		ID3D11Device* device = DX11::GetDevice();
		ID3D11DeviceContext* context = DX11::GetContext();

		uint32 lineSize = width * 4;

		// Create Texture2D

		D3D11_TEXTURE2D_DESC tex2DDesc { };
		tex2DDesc.Width = width;
		tex2DDesc.Height = height;
		tex2DDesc.MipLevels = 0;
		tex2DDesc.ArraySize = 1;
		tex2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		tex2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		tex2DDesc.Usage = D3D11_USAGE_DEFAULT;
		tex2DDesc.CPUAccessFlags = 0;
		tex2DDesc.SampleDesc.Count = 1;
		tex2DDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		dxcall(device->CreateTexture2D(&tex2DDesc, nullptr, &m_Texture));

		// Create SRV (Shader Resource View)

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc { };
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;

		dxcall(device->CreateShaderResourceView(m_Texture, &srvDesc, &m_SRV));

		// Update Subresource

		dxcall_v(context->UpdateSubresource(m_Texture, 0, nullptr, pixelData, lineSize, 0));

		// Generate mipmaps

		dxcall_v(context->GenerateMips(m_SRV));

		// Create Sampler State

		D3D11_SAMPLER_DESC samplerDesc { };
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.MipLODBias = 1; // For visualization
		samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		dxcall(device->CreateSamplerState(&samplerDesc, &m_SamplerState));
	}
}
