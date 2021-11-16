#pragma once

#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

struct ImDrawData;
struct ImGuiViewport;

namespace Ion
{
	class GenericWindow;

	class ION_API DX11
	{
		friend class RenderAPI;
	public:
		/* Called by the Application class */
		static void Init(GenericWindow* window);
		static void Shutdown();

		static void EndFrame();

		static FORCEINLINE const char* GetVendor() { return 0; }
		static FORCEINLINE const char* GetRendererName() { return 0; }
		static FORCEINLINE const char* GetVersion() { return 0; }
		static FORCEINLINE const char* GetLanguageVersion() { return 0; }
		static FORCEINLINE const char* GetExtensions() { return 0; }

		static FORCEINLINE const char* GetFeatureLevelString() { return D3DFeatureLevelToString(s_FeatureLevel); }
		static FORCEINLINE D3D_FEATURE_LEVEL GetFeatureLevel() { return s_FeatureLevel; }

		static const char* GetDisplayName() { return s_DisplayName; }

		//static void FilterDebugMessages();

		static void SetSwapInterval(uint32 interval) { s_SwapInterval = interval; }
		static uint32 GetSwapInterval() { return s_SwapInterval; }

		inline static char* D3DFeatureLevelToString(D3D_FEATURE_LEVEL level)
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

	protected:
		static void SetDisplayVersion(const char* version);

	private:
		static void InitImGuiBackend();
		static void ImGuiNewFrame();
		static void ImGuiRender(ImDrawData* drawData);
		static void ImGuiShutdown();

		static void ImGuiImplRendererCreateWindowPlatform(ImGuiViewport* viewport);
		static void ImGuiImplRendererRenderWindow(ImGuiViewport* viewport, void*);
		static void ImGuiImplRendererSwapBuffers(ImGuiViewport* viewport, void*);
		static void ImGuiImplRendererDestroyWindow(ImGuiViewport* viewport);

	protected:
		static bool s_Initialized;
		static D3D_FEATURE_LEVEL s_FeatureLevel;

	private:
		static char s_DisplayName[120];

		static ID3D11Device* s_Device;
		static ID3D11DeviceContext* s_Context;
		static IDXGISwapChain* s_SwapChain;
		static ID3D11RenderTargetView* s_RenderTarget;

		static uint32 s_SwapInterval;

		friend class DX11Renderer;
	};
}
