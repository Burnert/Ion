#include "IonPCH.h"

#include "DX10.h"
#include "RHI/DX10/DX10Texture.h"

#include "Application/Platform/Windows/WindowsWindow.h"
#include "Core/Platform/Windows/WindowsUtility.h"

#include "Renderer/Renderer.h"

#include "UserInterface/ImGui.h"

#pragma comment(lib, "D3D10_1.lib")
#pragma comment(lib, "DXGI.lib")

#pragma warning(disable:6001)
#pragma warning(disable:6387)

namespace Ion
{
	bool DX10::Init(GenericWindow* window)
	{
		TRACE_FUNCTION();

		ionassert(window);

		HRESULT hResult = S_OK;

		InitWindow(*window);

		const char* version = DXCommon::D3DFeatureLevelToString((D3D_FEATURE_LEVEL)s_FeatureLevel);
		SetDisplayVersion(version);
		DX10Logger.Info("Renderer: DirectX {}", version);
		DX10Logger.Info("Shader Model {}", GetShaderModel());

		return true;
	}

	bool DX10::InitWindow(GenericWindow& window)
	{
		TRACE_FUNCTION();

		HRESULT hResult = S_OK;

		WindowsWindow& windowsWindow = (WindowsWindow&)window;
		HWND hwnd = (HWND)window.GetNativeHandle();

		// Create Device and Swap Chain
		DXGI_SWAP_CHAIN_DESC scd { };
		{
			TRACE_SCOPE("DX10::Init - Create Device and Swap Chain");

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
			scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			scd.Flags = 0;

			D3D10_FEATURE_LEVEL1 targetFeatureLevel = D3D10_FEATURE_LEVEL_10_1;

			uint32 flags = 0;
#if ION_DEBUG
			flags |= D3D10_CREATE_DEVICE_DEBUG /*| D3D10_CREATE_DEVICE_DEBUGGABLE*/;
#endif

			IDXGIFactory* dxgiFactory = nullptr;
			win_check_hresult_r(CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory)),
				false, "Cannot create a DXGI Factory object.");

			UINT i = 0;
			IDXGIAdapter* adapter;
			TArray<IDXGIAdapter*> adapters;
			for (UINT i = 0; dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				//DXGI_ADAPTER_DESC desc;
				//adapter->GetDesc(&desc);
				adapters.push_back(adapter);
			}

			ionassert(!adapters.empty(), "Could not find any video adapters.");

			win_check_hresult_r(
				D3D10CreateDeviceAndSwapChain1(
					adapters[0],
					D3D10_DRIVER_TYPE_HARDWARE,
					NULL,
					flags,
					targetFeatureLevel,
					D3D10_1_SDK_VERSION,
					&scd,
					&s_SwapChain,
					&s_Device),
				false, "Cannot create D3D Device and Swap Chain.");

			s_FeatureLevel = s_Device->GetFeatureLevel();
		}

		// Create Debug Info Queue
		{
			win_check_hresult_r(s_Device->QueryInterface(IID_PPV_ARGS(&s_DebugInfoQueue)), false);
			ionassert(s_DebugInfoQueue);
		}

		// Create Render Target

		window.m_WindowColorTexture = CreateRenderTarget();

		// Create Rasterizer State
		{
			TRACE_SCOPE("DX10::Init - Create Rasterizer State");

			D3D10_RASTERIZER_DESC rd { };
			rd.FillMode = D3D10_FILL_SOLID;
			rd.CullMode = D3D10_CULL_BACK;
			rd.FrontCounterClockwise = true;
			rd.DepthClipEnable = true;
			rd.MultisampleEnable = true;

			dxcall_f(s_Device->CreateRasterizerState(&rd, &s_RasterizerState));
			dxcall_v(s_Device->RSSetState(s_RasterizerState));
		}
		// Create Blend State
		{
			TRACE_SCOPE("DX10::Init - Create Blend State");

			D3D10_BLEND_DESC1 blendDesc { };
			blendDesc.RenderTarget[0].BlendEnable = true;
			// Enable alpha blending
			blendDesc.RenderTarget[0].SrcBlend = D3D10_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D10_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D10_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D10_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D10_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

			dxcall_f(s_Device->CreateBlendState1(&blendDesc, &s_BlendStateTransparent));

			s_Device->OMSetBlendState(s_BlendState, nullptr, 0xFFFFFFFF);
		}
		// Create Depth / Stencil Buffer
		{
			TRACE_SCOPE("DX10::Init - Create Depth / Stencil Buffer");

			D3D10_DEPTH_STENCIL_DESC dsd { };
			dsd.DepthEnable = true;
			dsd.DepthFunc = D3D10_COMPARISON_LESS;
			dsd.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
			dsd.StencilEnable = true;
			dsd.StencilReadMask = D3D10_DEFAULT_STENCIL_READ_MASK;
			dsd.StencilWriteMask = D3D10_DEFAULT_STENCIL_WRITE_MASK;
			dsd.FrontFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
			dsd.FrontFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
			dsd.FrontFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
			dsd.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;
			dsd.BackFace.StencilFunc = D3D10_COMPARISON_ALWAYS;
			dsd.BackFace.StencilFailOp = D3D10_STENCIL_OP_KEEP;
			dsd.BackFace.StencilPassOp = D3D10_STENCIL_OP_KEEP;
			dsd.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;

			dxcall_f(s_Device->CreateDepthStencilState(&dsd, &s_DepthStencilState));
			dxcall_v(s_Device->OMSetDepthStencilState(s_DepthStencilState, 1));

			s_SwapChain->GetDesc(&scd);
			window.m_WindowDepthStencilTexture = CreateDepthStencil(scd.BufferDesc.Width, scd.BufferDesc.Height);
		}
		// Set Viewports
		{
			TRACE_SCOPE("DX10::Init - Set Viewports");

			WindowDimensions dimensions = window.GetDimensions();

			D3D10_VIEWPORT viewport { };
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = (UINT)dimensions.Width;
			viewport.Height = (UINT)dimensions.Height;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			dxcall_v(s_Device->RSSetViewports(1, &viewport));
		}
		// Disable Alt+Enter Fullscreen

		IDXGIFactory* factory = nullptr;

		dxcall_f(s_SwapChain->GetParent(IID_PPV_ARGS(&factory)), "Cannot get the Swap Chain factory.");
		dxcall_f(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

		factory->Release();

		// -------------------------------------------------------

		dxcall_v(s_Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		return true;
	}

	void DX10::Shutdown()
	{
		TRACE_FUNCTION();

		if (s_SwapChain)
			s_SwapChain->SetFullscreenState(false, nullptr);

		// Free the D3D objects

		COMRelease(s_Device);
		COMRelease(s_SwapChain);
		COMRelease(s_DepthStencilState);
		COMRelease(s_RasterizerState);
		COMRelease(s_DebugInfoQueue);
	}

	void DX10::ShutdownWindow(GenericWindow& window)
	{
	}

	void DX10::BeginFrame()
	{
	}

	void DX10::EndFrame(GenericWindow& window)
	{
		TRACE_FUNCTION();

		HRESULT hResult;
		dxcall(s_SwapChain->Present(s_SwapInterval, 0), "Cannot present frame.");
	}

	void DX10::ChangeDisplayMode(GenericWindow& window, EDisplayMode mode, uint32 width, uint32 height)
	{
		TRACE_FUNCTION();

		HRESULT hResult;

		dxcall(s_SwapChain->SetFullscreenState(mode == EDisplayMode::FullScreen, nullptr));

		ResizeBuffers(window, { width, height });
	}

	void DX10::ResizeBuffers(GenericWindow& window, const TextureDimensions& size)
	{
		TRACE_FUNCTION();

		HRESULT hResult = S_OK;

		window.m_WindowColorTexture = nullptr;
		window.m_WindowDepthStencilTexture = nullptr;

		dxcall(s_SwapChain->ResizeBuffers(2, size.Width, size.Height, DXGI_FORMAT_UNKNOWN, 0),
			"Cannot resize buffers.");

		window.m_WindowColorTexture = CreateRenderTarget();
		window.m_WindowDepthStencilTexture = CreateDepthStencil(size.Width, size.Height);
	}

	String DX10::GetCurrentDisplayName()
	{
		return GetDisplayName();
	}

	DX10::MessageArray DX10::GetDebugMessages()
	{
		TRACE_FUNCTION();

		if (!s_DebugInfoQueue)
		{
			ionbreak("Debug Info Queue has not been initialized.");
			return { };
		}

		HRESULT hResult;
		MessageArray messageArray;
		uint64 nMessages = s_DebugInfoQueue->GetNumStoredMessages();
		for (uint64 i = 0; i < nMessages; ++i)
		{
			uint64 messageLength = 0;
			hResult = s_DebugInfoQueue->GetMessage(i, nullptr, &messageLength);
			win_check_hresult_c(hResult, { break; }, "Could not get the message length.");

			D3D10_MESSAGE* message = (D3D10_MESSAGE*)malloc(messageLength);
			hResult = s_DebugInfoQueue->GetMessage(i, message, &messageLength);
			if (hResult == S_OK)
			{
				messageArray.emplace_back(DX10DebugMessage { message->Severity, message->pDescription });
			}
			free(message);

			win_check_hresult_c(hResult, { break; }, "Could not retrieve message.");
		}

		s_DebugInfoQueue->ClearStoredMessages();

		return messageArray;
	}

	void DX10::PrintDebugMessages()
	{
		if (!s_DebugInfoQueue || !s_DebugInfoQueue->GetNumStoredMessages())
			return;

		for (const DX10DebugMessage& message : GetDebugMessages())
		{
			switch (message.Severity)
			{
			case D3D10_MESSAGE_SEVERITY_CORRUPTION:
			{
				DX10Logger.Critical(message.Message);
				break;
			}
			case D3D10_MESSAGE_SEVERITY_ERROR:
			{
				DX10Logger.Error(message.Message);
				break;
			}
			case D3D10_MESSAGE_SEVERITY_WARNING:
			{
				DX10Logger.Warn(message.Message);
				break;
			}
			case D3D10_MESSAGE_SEVERITY_INFO:
			{
				DX10Logger.Info(message.Message);
				break;
			}
			case D3D10_MESSAGE_SEVERITY_MESSAGE:
			{
				DX10Logger.Trace(message.Message);
				break;
			}
			default:
				break;
			}
		}
	}

	void DX10::PrepareDebugMessageQueue()
	{
		if (!s_DebugInfoQueue)
			return;

		s_DebugInfoQueue->ClearStoredMessages();
	}

	void DX10::SetDebugName(ID3D10DeviceChild* object, const String& name)
	{
		ionassert(object);

		if (name.empty())
			return;

		HRESULT hResult;

		dxcall(object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str()));
	}

	void DX10::SetDisplayVersion(const char* version)
	{
		s_DisplayName = "DirectX "s + version;
	}

	const char* DX10::GetShaderModel()
	{
		return DXCommon::GetShaderModelString((D3D_FEATURE_LEVEL)s_FeatureLevel);
	}

	TShared<RHITexture> DX10::CreateRenderTarget()
	{
		TRACE_FUNCTION();

		HRESULT hResult;

		ID3D10Texture2D* backBuffer;

		dxcall_r(s_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)),
			nullptr, "Cannot get buffer.");

		D3D10_TEXTURE2D_DESC t2dDesc;
		backBuffer->GetDesc(&t2dDesc);

		TextureDescription desc { };
		desc.Format = ETextureFormat::RGBA8;
		desc.bUseAsRenderTarget = true;
		desc.Dimensions = { t2dDesc.Width, t2dDesc.Height };

		return MakeShareable(new DX10Texture(desc, backBuffer));
	}

	TShared<RHITexture> DX10::CreateDepthStencil(uint32 width, uint32 height)
	{
		TRACE_FUNCTION();

		TextureDescription desc { };
		desc.Format = ETextureFormat::D24S8;
		desc.bUseAsDepthStencil = true;
		desc.Dimensions = { width, height };

		return RHITexture::CreateShared(desc);
	}

	void DX10::InitImGuiBackend()
	{
		TRACE_FUNCTION();

		ImGui_ImplDX10_Init(s_Device);
	}

	void DX10::ImGuiNewFrame()
	{
		TRACE_FUNCTION();

		ImGui_ImplDX10_NewFrame();
	}

	void DX10::ImGuiRender(ImDrawData* drawData)
	{
		TRACE_FUNCTION();

		ImGui_ImplDX10_RenderDrawData(drawData);
	}

	void DX10::ImGuiShutdown()
	{
		TRACE_FUNCTION();

		ImGui_ImplDX10_Shutdown();
	}

	ID3D10Device1* DX10::s_Device = nullptr;
	IDXGISwapChain* DX10::s_SwapChain = nullptr;
	ID3D10DepthStencilState* DX10::s_DepthStencilState = nullptr;
	ID3D10RasterizerState* DX10::s_RasterizerState = nullptr;
	ID3D10BlendState1* DX10::s_BlendState = nullptr;
	ID3D10BlendState1* DX10::s_BlendStateTransparent = nullptr;

	ID3D10InfoQueue* DX10::s_DebugInfoQueue = nullptr;

	uint32 DX10::s_SwapInterval = 0;

	String DX10::s_DisplayName;

	bool DX10::s_Initialized = false;
	D3D10_FEATURE_LEVEL1 DX10::s_FeatureLevel = D3D10_FEATURE_LEVEL_9_1;
}
