#include "IonPCH.h"

#include "DX11Texture.h"

namespace Ion
{
	DX11Texture::~DX11Texture()
	{
		TRACE_FUNCTION();

		if (m_Texture)
			m_Texture->Release();

		if (m_SRV)
			m_SRV->Release();

		if (m_SamplerState)
			m_SamplerState->Release();
	}

	void DX11Texture::Bind(uint32 slot) const
	{
		TRACE_FUNCTION();

		dxcall_v(DX11::GetContext()->PSSetShaderResources(0, 1, &m_SRV));
		dxcall_v(DX11::GetContext()->PSSetSamplers(0, 1, &m_SamplerState));
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

		CreateTexture();
	}

	void DX11Texture::CreateTexture()
	{
		TRACE_FUNCTION();

		ionassert(m_TextureImage->IsLoaded(), "Image has not been initialized yet.");

		HRESULT hResult;

		ID3D11Device* device = DX11::GetDevice();

		// Create Texture2D

		D3D11_TEXTURE2D_DESC tex2DDesc { };
		tex2DDesc.Width = m_TextureImage->GetWidth();
		tex2DDesc.Height = m_TextureImage->GetHeight();
		tex2DDesc.MipLevels = 1;
		tex2DDesc.ArraySize = 1;
		tex2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		tex2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tex2DDesc.Usage = D3D11_USAGE_DEFAULT;
		tex2DDesc.CPUAccessFlags = 0;
		tex2DDesc.SampleDesc.Count = 1;
		tex2DDesc.MiscFlags = 0;/*D3D11_RESOURCE_MISC_GENERATE_MIPS*/

		uint32 lineSize = m_TextureImage->GetWidth() * 4;

		D3D11_SUBRESOURCE_DATA subData { };
		subData.pSysMem = m_TextureImage->GetPixelData();
		subData.SysMemPitch = lineSize;

		dxcall(device->CreateTexture2D(&tex2DDesc, &subData, &m_Texture));

		// Create SRV (Shader Resource View)

		D3D11_TEX2D_SRV tex2DSRV { };
		tex2DSRV.MipLevels = -1;
		tex2DSRV.MostDetailedMip = 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc { };
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D = tex2DSRV;

		dxcall(device->CreateShaderResourceView(m_Texture, &srvDesc, &m_SRV));

		// Create Sampler State

		D3D11_SAMPLER_DESC samplerDesc { };
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		dxcall(device->CreateSamplerState(&samplerDesc, &m_SamplerState));
	}
}
