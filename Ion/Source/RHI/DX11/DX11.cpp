#include "IonPCH.h"

#include "RHI/RHICore.h"

#if RHI_BUILD_DX11

#include "Core/Platform/Windows.h"

#include "DX11.h"
#include "RHI/DX11/DX11Texture.h"

#include "Renderer/Renderer.h"

#include "UserInterface/ImGui.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

#pragma warning(disable:6001)
#pragma warning(disable:6387)

namespace Ion
{
	void DX11DebugMessageQueue::PrepareQueue()
	{
		DX11::PrepareDebugMessageQueue();
	}

	void DX11DebugMessageQueue::PrintMessages()
	{
		DX11::PrintDebugMessages();
	}

	Result<void, RHIError> DX11::Init(RHIWindowData& mainWindow)
	{
		TRACE_FUNCTION();

		// No delete, persistent object
		g_DXDebugMessageQueue = new DX11DebugMessageQueue;

		// Init Debug Layer
		InitDebugLayer()
			.Err([](Error& error) { DX11Logger.Error(error.Message); });

		fwdthrowall(InitWindow(mainWindow));

		SetDisplayVersion(DXCommon::D3DFeatureLevelToString(s_FeatureLevel));
		DX11Logger.Info("Renderer: DirectX {}", GetFeatureLevelString());
		DX11Logger.Info("Shader Model {}", DXCommon::GetShaderModelString(s_FeatureLevel));

		return Ok();
	}

	Result<void, RHIError> DX11::InitWindow(RHIWindowData& window)
	{
		TRACE_FUNCTION();

		HWND hwnd = reinterpret_cast<HWND>(window.NativeHandle);

		// Create Device and Swap Chain
		DXGI_SWAP_CHAIN_DESC scd { };
		{
			TRACE_SCOPE("DX11::Init - Create Device and Swap Chain");

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

			D3D_FEATURE_LEVEL targetFeatureLevel[] = {
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
			};

			uint32 flags = 0;
#if ION_DEBUG
			flags |= D3D11_CREATE_DEVICE_DEBUG /*| D3D11_CREATE_DEVICE_DEBUGGABLE*/;
#endif

			ID3D11Device* device;
			ID3D11DeviceContext* context;
			dxcall(
				D3D11CreateDeviceAndSwapChain(
					nullptr,
					D3D_DRIVER_TYPE_HARDWARE,
					NULL,
					flags,
					targetFeatureLevel,
					2,
					D3D11_SDK_VERSION,
					&scd,
					&s_SwapChain,
					&device,
					&s_FeatureLevel,
					&context),
				"Cannot create D3D11 Device and Swap Chain.");

			dxcall(device->QueryInterface(IID_PPV_ARGS(&s_Device)));
			dxcall(context->QueryInterface(IID_PPV_ARGS(&s_Context)));
		}
		// Create Render Target

		safe_unwrap(window.ColorTexture, CreateRenderTarget());

		// Create Rasterizer State
		{
			TRACE_SCOPE("DX11::Init - Create Rasterizer State");

			D3D11_RASTERIZER_DESC rd { };
			rd.FillMode = D3D11_FILL_SOLID;
			rd.CullMode = D3D11_CULL_BACK;
			rd.FrontCounterClockwise = true;
			rd.DepthClipEnable = true;
			rd.MultisampleEnable = true;

			dxcall(s_Device->CreateRasterizerState(&rd, &s_RasterizerState));
			dxcall(s_Context->RSSetState(s_RasterizerState));
		}
		// Create Blend State
		{
			TRACE_SCOPE("DX11::Init - Create Blend State");

			D3D11_BLEND_DESC1 blendDesc { };
			blendDesc.RenderTarget[0].BlendEnable = true;
			// Enable alpha blending
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].LogicOp = D3D11_LOGIC_OP_NOOP;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			dxcall(s_Device->CreateBlendState1(&blendDesc, &s_BlendStateTransparent));
			dxcall(s_Context->OMSetBlendState(s_BlendState, nullptr, 0xFFFFFFFF));
		}
		// Create Depth / Stencil Buffer
		{
			TRACE_SCOPE("DX11::Init - Create Depth / Stencil Buffer");

			D3D11_DEPTH_STENCIL_DESC dsd { };
			dsd.DepthEnable = true;
			dsd.DepthFunc = D3D11_COMPARISON_LESS;
			dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsd.StencilEnable = true;
			dsd.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
			dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
			dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

			dxcall(s_Device->CreateDepthStencilState(&dsd, &s_DepthStencilState));
			dxcall(s_Context->OMSetDepthStencilState(s_DepthStencilState, 1));

			dxcall(s_SwapChain->GetDesc(&scd));
			safe_unwrap(window.DepthStencilTexture, CreateDepthStencil(scd.BufferDesc.Width, scd.BufferDesc.Height));
		}
		// Set Viewports
		{
			TRACE_SCOPE("DX11::Init - Set Viewports");

			TextureDimensions dimensions = window.ColorTexture->GetDimensions();

			D3D11_VIEWPORT viewport { };
			viewport.TopLeftX = 0.0f;
			viewport.TopLeftY = 0.0f;
			viewport.Width = (float)dimensions.Width;
			viewport.Height = (float)dimensions.Height;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			dxcall(s_Context->RSSetViewports(1, &viewport));
		}
		// Disable Alt+Enter Fullscreen

		IDXGIFactory1* factory = nullptr;

		dxcall(s_SwapChain->GetParent(IID_PPV_ARGS(&factory)), "Cannot get the Swap Chain factory.");
		dxcall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

		factory->Release();

		// -------------------------------------------------------

		dxcall(s_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		return Ok();
	}

	void DX11::Shutdown()
	{
		TRACE_FUNCTION();

		if (s_SwapChain)
			dxcall_nocheck(s_SwapChain->SetFullscreenState(false, nullptr));

		// Free the D3D objects

		COMRelease(s_Device);
		COMRelease(s_Context);
		COMRelease(s_SwapChain);
		COMRelease(s_DepthStencilState);
		COMRelease(s_RasterizerState);
		COMRelease(s_DebugInfoQueue);
	}

	void DX11::ShutdownWindow(RHIWindowData& window)
	{

	}

	Result<void, RHIError> DX11::BeginFrame()
	{
		TRACE_FUNCTION();

		//dxcall(s_Context->OMSetRenderTargets(1, &s_RTV, s_DSV), "Cannot set render target.");

		return Ok();
	}

	Result<void, RHIError> DX11::EndFrame(RHIWindowData& window)
	{
		TRACE_FUNCTION();

		dxcall(s_SwapChain->Present(s_SwapInterval, 0), "Cannot present frame.");

		return Ok();
	}

	Result<void, RHIError> DX11::ChangeDisplayMode(RHIWindowData& window, EWindowDisplayMode mode, uint32 width, uint32 height)
	{
		TRACE_FUNCTION();

		dxcall(s_SwapChain->SetFullscreenState(mode == EWindowDisplayMode::FullScreen, nullptr));

		return ResizeBuffers(window, { width, height });
	}

	Result<void, RHIError> DX11::ResizeBuffers(RHIWindowData& window, const TextureDimensions& size)
	{
		TRACE_FUNCTION();

		window.ColorTexture = nullptr;
		window.DepthStencilTexture = nullptr;

		dxcall(s_SwapChain->ResizeBuffers(2, size.Width, size.Height, DXGI_FORMAT_UNKNOWN, 0),
			"Cannot resize buffers.");

		safe_unwrap(window.ColorTexture, CreateRenderTarget());
		safe_unwrap(window.DepthStencilTexture, CreateDepthStencil(size.Width, size.Height));

		return Ok();
	}

	String DX11::GetCurrentDisplayName()
	{
		return GetDisplayName();
	}

	DX11::MessageArray DX11::GetDebugMessages()
	{
		TRACE_FUNCTION();

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
			if (FAILED(hResult))
			{
				DX11Logger.Error("Could not get the message length.\n{}", Windows::FormatHResultMessage(hResult));
				continue;
			}

			DXGI_INFO_QUEUE_MESSAGE* message = (DXGI_INFO_QUEUE_MESSAGE*)malloc(messageLength);
			ionverify(message);

			hResult = s_DebugInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, message, &messageLength);
			if (FAILED(hResult))
			{
				DX11Logger.Error("Could not retrieve the message.\n{}", Windows::FormatHResultMessage(hResult));
				free(message);
				continue;
			}

			messageArray.emplace_back(DXGIDebugMessage { message->Severity, message->pDescription });
			free(message);
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
				DX11Logger.Critical(message.Message);
				break;
			}
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR:
			{
				DX11Logger.Error(message.Message);
				break;
			}
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING:
			{
				DX11Logger.Warn(message.Message);
				break;
			}
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO:
			{
				DX11Logger.Info(message.Message);
				break;
			}
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE:
			{
				DX11Logger.Trace(message.Message);
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

	Result<void, RHIError> DX11::SetDebugName(ID3D11DeviceChild* object, const String& name, const String& prefix)
	{
#if ION_DEBUG
		ionassert(object);

		if (name.empty())
			return Ok();

		String fullName = prefix + name;
		dxcall(object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)fullName.size(), fullName.c_str()));
#endif
		return Ok();
	}

	void DX11::SetDisplayVersion(const char* version)
	{
		TRACE_FUNCTION();

		static char* directX = "DirectX ";
		static size_t length = strlen(directX);
		strcpy_s((s_DisplayName + length), 120 - length, version);
	}

	Result<TRef<RHITexture>, RHIError> DX11::CreateRenderTarget()
	{
		TRACE_FUNCTION();

		ID3D11Texture2D* backBuffer;

		dxcall(s_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)),
			"Cannot get buffer.");

		D3D11_TEXTURE2D_DESC t2dDesc;
		backBuffer->GetDesc(&t2dDesc);

		TextureDescription desc { };
		desc.Format = ETextureFormat::RGBA8;
		desc.bUseAsRenderTarget = true;
		desc.Dimensions = { t2dDesc.Width, t2dDesc.Height };
		desc.DebugName = "Window_BackBuffer_RT";

		return TRef<RHITexture>(new DX11Texture(desc, backBuffer));
	}

	Result<TRef<RHITexture>, RHIError> DX11::CreateDepthStencil(uint32 width, uint32 height)
	{
		TRACE_FUNCTION();

		TextureDescription desc { };
		desc.Format = ETextureFormat::D24S8;
		desc.bUseAsDepthStencil = true;
		desc.Dimensions = { width, height };
		desc.DebugName = "Window_BackBuffer_DS";

		return RHITexture::Create(desc);
	}

	void DX11::InitImGuiBackend()
	{
		TRACE_FUNCTION();

		ImGui_ImplDX11_Init(s_Device, s_Context);
	}

	void DX11::ImGuiNewFrame()
	{
		TRACE_FUNCTION();

		ImGui_ImplDX11_NewFrame();
	}

	void DX11::ImGuiRender(ImDrawData* drawData)
	{
		TRACE_FUNCTION();

		ImGui_ImplDX11_RenderDrawData(drawData);
	}

	void DX11::ImGuiShutdown()
	{
		TRACE_FUNCTION();

		ImGui_ImplDX11_Shutdown();
	}

	Result<void, RHIError, PlatformError> DX11::InitDebugLayer()
	{
		TRACE_FUNCTION();

		s_hDxgiDebugModule = LoadLibrary(L"Dxgidebug.dll");
		if (!s_hDxgiDebugModule)
		{
			ionthrow(PlatformError, "Could not load module Dxgidebug.dll.\n{}", Windows::GetLastErrorMessage());
		}

		DXGIGetDebugInterface = (DXGIGetDebugInterfaceProc)GetProcAddress(s_hDxgiDebugModule, "DXGIGetDebugInterface");
		if (!DXGIGetDebugInterface)
		{
			ionthrow(PlatformError, "Cannot load DXGIGetDebugInterface from Dxgidebug.dll.");
		}

		HRESULT hResult = DXGIGetDebugInterface(IID_PPV_ARGS(&s_DebugInfoQueue));
		if (FAILED(hResult))
		{
			ionthrow(RHIError, "Cannot load DXGIGetDebugInterface from Dxgidebug.dll.\n{}", Windows::FormatHResultMessage(hResult));
		}

		return Ok();
	}

	bool DX11::s_Initialized = false;
	D3D_FEATURE_LEVEL DX11::s_FeatureLevel = D3D_FEATURE_LEVEL_1_0_CORE;

	char DX11::s_DisplayName[120] = "DirectX ";

	ID3D11Device1* DX11::s_Device = nullptr;
	ID3D11DeviceContext1* DX11::s_Context = nullptr;
	IDXGISwapChain* DX11::s_SwapChain = nullptr;
	ID3D11DepthStencilState* DX11::s_DepthStencilState = nullptr;
	ID3D11RasterizerState* DX11::s_RasterizerState = nullptr;
	ID3D11BlendState1* DX11::s_BlendState = nullptr;
	ID3D11BlendState1* DX11::s_BlendStateTransparent = nullptr;

	uint32 DX11::s_SwapInterval = 0;

	HMODULE DX11::s_hDxgiDebugModule = NULL;
	DX11::DXGIGetDebugInterfaceProc DX11::DXGIGetDebugInterface = nullptr;
	IDXGIInfoQueue* DX11::s_DebugInfoQueue = nullptr;
}

#endif // RHI_BUILD_DX11
