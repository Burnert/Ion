#pragma once

#include "RHI/RHI.h"

#include "Core/Platform/Windows/WindowsCore.h"

#include "RHI/DirectX/DXCommon.h"
#include <d3d11_1.h>
#include <dxgidebug.h>

struct ImDrawData;
struct ImGuiViewport;

namespace Ion
{
	REGISTER_LOGGER(DX11Logger, "RHI::DX11");

	class DX11DebugMessageQueue : public IDXDebugMessageQueue
	{
	public:
		virtual void PrepareQueue() override;
		virtual void PrintMessages() override;
	};

	struct DXGIDebugMessage
	{
		DXGI_INFO_QUEUE_MESSAGE_SEVERITY Severity;
		String Message;
	};

	class GenericWindow;

	class ION_API DX11 : public RHI
	{
	public:
		/* Called by the Application class */
		virtual bool Init(GenericWindow* window) override;
		virtual bool InitWindow(GenericWindow& window) override;
		virtual void Shutdown() override;
		virtual void ShutdownWindow(GenericWindow& window) override;

		virtual void BeginFrame() override;
		virtual void EndFrame(GenericWindow& window) override;

		virtual void ChangeDisplayMode(GenericWindow& window, EDisplayMode mode, uint32 width, uint32 height);
		virtual void ResizeBuffers(GenericWindow& window, const TextureDimensions& size);

		virtual String GetCurrentDisplayName() override;

		static FORCEINLINE const char* GetFeatureLevelString()
		{
			return DXCommon::D3DFeatureLevelToString(s_FeatureLevel);
		}

		static FORCEINLINE D3D_FEATURE_LEVEL GetFeatureLevel()
		{
			return s_FeatureLevel;
		}

		static inline ID3D11Device* GetDevice()
		{
			return s_Device;
		}

		static inline ID3D11DeviceContext* GetContext()
		{
			return s_Context;
		}

		static inline IDXGISwapChain* GetSwapChain()
		{
			return s_SwapChain;
		}

		static inline ID3D11RasterizerState* GetRasterizerState()
		{
			return s_RasterizerState;
		}

		static const char* GetDisplayName()
		{
			return s_DisplayName;
		}

		static void SetSwapInterval(uint32 interval)
		{
			s_SwapInterval = interval;
		}

		static uint32 GetSwapInterval()
		{
			return s_SwapInterval;
		}

		using MessageArray = TArray<DXGIDebugMessage>;
		static MessageArray GetDebugMessages();
		static void PrintDebugMessages();
		static void PrepareDebugMessageQueue();

		static void SetDebugName(ID3D11DeviceChild* object, const String& name, const String& prefix);

	protected:
		static void SetDisplayVersion(const char* version);

		static void CreateRenderTarget(TShared<RHITexture>& texture);
		static void CreateDepthStencil(TShared<RHITexture>& texture, uint32 width, uint32 height);

	private:
		virtual void InitImGuiBackend() override;
		virtual void ImGuiNewFrame() override;
		virtual void ImGuiRender(ImDrawData* drawData) override;
		virtual void ImGuiShutdown() override;

		static Result<void, DXError, PlatformError> InitDebugLayer();

	protected:
		static bool s_Initialized;
		static D3D_FEATURE_LEVEL s_FeatureLevel;

	private:
		static char s_DisplayName[120];

		static ID3D11Device1* s_Device;
		static ID3D11DeviceContext1* s_Context;
		static IDXGISwapChain* s_SwapChain;
		static ID3D11DepthStencilState* s_DepthStencilState;
		static ID3D11RasterizerState* s_RasterizerState;
		/** Default blend state (nullptr) */
		static ID3D11BlendState1* s_BlendState;
		static ID3D11BlendState1* s_BlendStateTransparent;

		static uint32 s_SwapInterval;

		static HMODULE s_hDxgiDebugModule;

		using DXGIGetDebugInterfaceProc = HRESULT(*)(REFIID, void**);
		static DXGIGetDebugInterfaceProc DXGIGetDebugInterface;

		static IDXGIInfoQueue* s_DebugInfoQueue;

		friend class DX11Renderer;
		friend class RHI;
	};
}
