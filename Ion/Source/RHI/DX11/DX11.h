#pragma once

#include "RHI/RHI.h"

#include "Core/Platform/Windows/WindowsCore.h"

#include <d3d11_1.h>
#include <dxgidebug.h>

#if ION_LOG_ENABLED
/// Requires
/// HRESULT hResult;
/// in function scope.
/// Returns a custom value on error.
#define dxcall_r(call, ret, ...) \
{ \
	Ion::DX11::PrepareDebugMessageQueue(); \
	hResult = call; \
	win_check_hresult_c(hResult, { Ion::DX11::PrintDebugMessages(); debugbreak(); return ret; }, __VA_ARGS__) \
	Ion::DX11::PrintDebugMessages(); \
}
#define dxcall_v(call, ...) \
{ \
	Ion::DX11::PrepareDebugMessageQueue(); \
	call; \
	Ion::DX11::PrintDebugMessages(); \
}
#else
#define dxcall_r(call, ret, ...) hResult = call
#define dxcall_v(call, ...) call
#endif

/// Requires
/// HRESULT hResult;
/// in function scope.
#define dxcall(call, ...) dxcall_r(call, , __VA_ARGS__)
/// Requires
/// HRESULT hResult;
/// in function scope.
/// Returns false on error.
#define dxcall_f(call, ...) dxcall_r(call, false, __VA_ARGS__)

struct ImDrawData;
struct ImGuiViewport;

namespace Ion
{
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
			return D3DFeatureLevelToString(s_FeatureLevel);
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

		static constexpr const char* D3DFeatureLevelToString(D3D_FEATURE_LEVEL level)
		{
			switch (level)
			{
			case D3D_FEATURE_LEVEL_1_0_CORE: return "1.0 Core";
			case D3D_FEATURE_LEVEL_9_1:      return "9.1";
			case D3D_FEATURE_LEVEL_9_2:      return "9.2";
			case D3D_FEATURE_LEVEL_9_3:      return "9.3";
			case D3D_FEATURE_LEVEL_10_0:     return "10.0";
			case D3D_FEATURE_LEVEL_10_1:     return "10.1";
			case D3D_FEATURE_LEVEL_11_0:     return "11.0";
			case D3D_FEATURE_LEVEL_11_1:     return "11.1";
			case D3D_FEATURE_LEVEL_12_0:     return "12.0";
			case D3D_FEATURE_LEVEL_12_1:     return "12.1";
			default:                         return "UNKNOWNVERSION";
			}
		}

		static constexpr const char* GetShaderModelString()
		{
			switch (s_FeatureLevel)
			{
			case D3D_FEATURE_LEVEL_1_0_CORE: return "";
			case D3D_FEATURE_LEVEL_9_1:      return "3.0";
			case D3D_FEATURE_LEVEL_9_2:      return "3.0";
			case D3D_FEATURE_LEVEL_9_3:      return "3.0";
			case D3D_FEATURE_LEVEL_10_0:     return "4.0";
			case D3D_FEATURE_LEVEL_10_1:     return "4.1";
			case D3D_FEATURE_LEVEL_11_0:     return "5.0";
			case D3D_FEATURE_LEVEL_11_1:     return "5.0";
			case D3D_FEATURE_LEVEL_12_0:     return "5.1";
			case D3D_FEATURE_LEVEL_12_1:     return "5.1";
			default:                         return "UNKNOWNVERSION";
			}
		}

	protected:
		static void SetDisplayVersion(const char* version);

		static void CreateRenderTarget(TShared<RHITexture>& texture);
		static void CreateDepthStencil(TShared<RHITexture>& texture, uint32 width, uint32 height);

	private:
		virtual void InitImGuiBackend() override;
		virtual void ImGuiNewFrame() override;
		virtual void ImGuiRender(ImDrawData* drawData) override;
		virtual void ImGuiShutdown() override;

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

		// Debug
#if ION_DEBUG
		static HMODULE s_hDxgiDebugModule;

		using DXGIGetDebugInterfaceProc = HRESULT(*)(REFIID, void**);
		static DXGIGetDebugInterfaceProc DXGIGetDebugInterface;

		static IDXGIInfoQueue* s_DebugInfoQueue;
#endif
		friend class DX11Renderer;
		friend class RHI;
	};
}
