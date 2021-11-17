#include "IonPCH.h"

#include "DX11.h"
#include "Application/Platform/Windows/WindowsWindow.h"
#include "Core/Platform/Windows/WindowsUtility.h"

#include "UserInterface/ImGui.h"

namespace Ion
{
	void DX11::Init(GenericWindow* window)
	{
#pragma warning(disable:6001)
#pragma warning(disable:6387)

		WindowsWindow& windowsWindow = (WindowsWindow&)window;
		HWND hwnd = (HWND)window->GetNativeHandle();

		HRESULT hResult = S_OK;

#if ION_DEBUG
		// Init Debug Layer

		HMODULE hDxgiDebug = LoadLibrary(L"Dxgidebug.dll");
		win_check(hDxgiDebug, "Could not load module Dxgidebug.dll.");

		DXGIGetDebugInterface = (DXGIGetDebugInterfaceProc)GetProcAddress(hDxgiDebug, "DXGIGetDebugInterface");
		win_check(DXGIGetDebugInterface, "Cannot load DXGIGetDebugInterface from Dxgidebug.dll.");

		dxcall(DXGIGetDebugInterface(IID_PPV_ARGS(&s_DebugInfoQueue)), "Cannot get the Debug Interface.");
#endif

		DXGI_SWAP_CHAIN_DESC scd { };
		scd.BufferCount = 2;
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
		scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		D3D_FEATURE_LEVEL targetFeatureLevel[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		uint32 flags = 0;
#if ION_DEBUG
		flags |= D3D11_CREATE_DEVICE_DEBUG /*| D3D11_CREATE_DEVICE_DEBUGGABLE*/;
#endif

		dxcall(
			D3D11CreateDeviceAndSwapChain(nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				flags,
				targetFeatureLevel,
				2,
				D3D11_SDK_VERSION,
				&scd,
				&s_SwapChain,
				&s_Device,
				&s_FeatureLevel,
				&s_Context),
			"Cannot create D3D Device and Swap Chain.");

		SetDisplayVersion(D3DFeatureLevelToString(s_FeatureLevel));

		CreateRenderTarget();

		WindowDimensions dimensions = window->GetDimensions();

		D3D11_VIEWPORT viewport { };
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = (float)dimensions.Width;
		viewport.Height = (float)dimensions.Height;
		s_Context->RSSetViewports(1, &viewport);

		// @TODO: Add DirectX version logging.
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

		if (s_DebugInfoQueue)
			s_DebugInfoQueue->Release();
	}

	void DX11::BeginFrame()
	{
		dxcall_v(s_Context->OMSetRenderTargets(1, &s_RenderTarget, nullptr), "Cannot set render target.");
	}

	void DX11::EndFrame()
	{
		HRESULT hResult;
		dxcall(s_SwapChain->Present(s_SwapInterval, 0), "Cannot present frame.");
	}

	DX11::MessageArray DX11::GetDebugMessages()
	{
		if (!s_DebugInfoQueue)
		{
			debugbreak();
			return { };
		}

		HRESULT hResult;
		MessageArray messageArray;
		uint64 nMessages = s_DebugInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
		for (uint64 i = 0; i < nMessages; ++i)
		{
			uint64 messageLength = 0;
			hResult = s_DebugInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength);
			win_check_hresult_c(hResult, { break; }, "Could not get message length.");

			DXGI_INFO_QUEUE_MESSAGE* message = (DXGI_INFO_QUEUE_MESSAGE*)malloc(messageLength);

			hResult = s_DebugInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, message, &messageLength);
			if (message)
			{
				if (hResult == S_OK)
				{
					messageArray.emplace_back(DXGIDebugMessage { message->Severity, message->pDescription });
				}
				free(message);
			}

			win_check_hresult_c(hResult, { break; }, "Could not retrieve message.");
		}

		s_DebugInfoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);

		return messageArray;
	}

	void DX11::PrintDebugMessages()
	{
		if (!s_DebugInfoQueue || !s_DebugInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL))
			return;

		for (const DXGIDebugMessage& message : GetDebugMessages())
		{
			switch (message.Severity)
			{
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION:
			{
				LOG_CRITICAL(message.Message);
				break;
			}
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR:
			{
				LOG_ERROR(message.Message);
				break;
			}
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING:
			{
				LOG_WARN(message.Message);
				break;
			}
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO:
			{
				LOG_INFO(message.Message);
				break;
			}
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE:
			{
				LOG_TRACE(message.Message);
				break;
			}
			default:
				break;
			}
		}
	}

	void DX11::PrepareDebugMessageQueue()
	{
		if (!s_DebugInfoQueue)
			return;

		s_DebugInfoQueue->ClearStoredMessages(DXGI_DEBUG_ALL);
	}

	void DX11::SetDisplayVersion(const char* version)
	{
		static char* directX = "DirectX ";
		static size_t length = strlen(directX);
		strcpy_s((s_DisplayName + length), 120 - length, version);
	}

	void DX11::CreateRenderTarget()
	{
		HRESULT hResult = S_OK;

		ID3D11Texture2D* backBuffer = nullptr;

		dxcall(s_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)),
			"Cannot get buffer.");

		dxcall(s_Device->CreateRenderTargetView(backBuffer, nullptr, &s_RenderTarget),
			"Cannot create Render Target from back buffer.");

		backBuffer->Release();
	}

	void DX11::InitImGuiBackend()
	{
		ImGui_ImplDX11_Init(s_Device, s_Context);
	}

	void DX11::ImGuiNewFrame()
	{
		ImGui_ImplDX11_NewFrame();
	}

	void DX11::ImGuiRender(ImDrawData* drawData)
	{
		ImGui_ImplDX11_RenderDrawData(drawData);
	}

	void DX11::ImGuiShutdown()
	{
		ImGui_ImplDX11_Shutdown();
	}

	bool DX11::s_Initialized = false;
	D3D_FEATURE_LEVEL DX11::s_FeatureLevel = D3D_FEATURE_LEVEL_1_0_CORE;

	DX11::DXGIGetDebugInterfaceProc DX11::DXGIGetDebugInterface = nullptr;

	IDXGIInfoQueue* DX11::s_DebugInfoQueue = nullptr;

	char DX11::s_DisplayName[120] = "DirectX ";

	ID3D11Device* DX11::s_Device = nullptr;
	ID3D11DeviceContext* DX11::s_Context = nullptr;
	IDXGISwapChain* DX11::s_SwapChain = nullptr;
	ID3D11RenderTargetView* DX11::s_RenderTarget = nullptr;

	uint32 DX11::s_SwapInterval = 0;
}
