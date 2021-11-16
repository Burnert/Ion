#include "IonPCH.h"

#include "DX11.h"
#include "Application/Platform/Windows/WindowsWindow.h"

#include "Core/Platform/Windows/WindowsUtility.h"

namespace Ion
{
	void DX11::Init(GenericWindow* window)
	{
#pragma warning(disable:6001)
#pragma warning(disable:6387)

		WindowsWindow& windowsWindow = (WindowsWindow&)window;
		HWND hwnd = (HWND)window->GetNativeHandle();

		HRESULT hResult = S_OK;

		DXGI_SWAP_CHAIN_DESC scd { };
		scd.BufferCount = 1;
		scd.BufferDesc.Width = 0;
		scd.BufferDesc.Height = 0;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.RefreshRate.Numerator = 0;
		scd.BufferDesc.RefreshRate.Denominator = 0;
		scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = hwnd;
		scd.Windowed = true;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scd.Flags = 0;

		D3D_FEATURE_LEVEL targetFeatureLevel[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		hResult = D3D11CreateDeviceAndSwapChain(nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			0,
			targetFeatureLevel,
			2,
			D3D11_SDK_VERSION,
			&scd,
			&s_SwapChain,
			&s_Device,
			&s_FeatureLevel,
			&s_Context);

		SetDisplayVersion(D3DFeatureLevelToString(s_FeatureLevel));

		if (FAILED(hResult))
		{
			Windows::PrintHResultError(hResult);
			ionassertnd(false, "Cannot create D3D Device and Swap Chain.");
			return;
		}

		ID3D11Resource* backBuffer = nullptr;

		hResult = s_SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&backBuffer);
		if (FAILED(hResult))
		{
			Windows::PrintHResultError(hResult);
			ionassertnd(false, "Cannot get buffer.");
			return;
		}

		hResult = s_Device->CreateRenderTargetView(backBuffer, nullptr, &s_RenderTarget);
		if (FAILED(hResult))
		{
			Windows::PrintHResultError(hResult);
			ionassertnd(false, "Cannot create Render Target from back buffer.");
			return;
		}

		backBuffer->Release();
	}

	void DX11::Shutdown()
	{
		// Free the D3D objects

		if (s_Device)
			s_Device->Release();

		if (s_Context)
			s_Context->Release();

		if (s_SwapChain)
			s_SwapChain->Release();

		if (s_RenderTarget)
			s_RenderTarget->Release();
	}

	void DX11::EndFrame()
	{
		HRESULT hResult;

		hResult = s_SwapChain->Present(s_SwapInterval, 0);
		if (FAILED(hResult))
		{
			Windows::PrintHResultError(hResult);
		}
	}

	void DX11::SetDisplayVersion(const char* version)
	{
		static char* directX = "DirectX ";
		static size_t length = strlen(directX);
		strcpy_s((s_DisplayName + length), 120 - length, version);
	}

	void DX11::InitImGuiBackend()
	{
	}

	void DX11::ImGuiNewFrame()
	{
	}

	void DX11::ImGuiRender(ImDrawData* drawData)
	{
	}

	void DX11::ImGuiShutdown()
	{
	}

	bool DX11::s_Initialized = false;
	D3D_FEATURE_LEVEL DX11::s_FeatureLevel = D3D_FEATURE_LEVEL_1_0_CORE;

	char DX11::s_DisplayName[120] = "DirectX ";

	ID3D11Device* DX11::s_Device = nullptr;
	ID3D11DeviceContext* DX11::s_Context = nullptr;
	IDXGISwapChain* DX11::s_SwapChain = nullptr;
	ID3D11RenderTargetView* DX11::s_RenderTarget = nullptr;

	uint32 DX11::s_SwapInterval = 1;
}
