#pragma once

#include "RHI/RHI.h"

#include "Core/Platform/Windows/WindowsCore.h"

#include "RHI/DirectX/DXCommon.h"
#include <d3d10_1.h>

namespace Ion
{
	REGISTER_LOGGER(DX10Logger, "RHI::DX10");

	class DX10DebugMessageQueue : public IDXDebugMessageQueue
	{
	public:
		virtual void PrepareQueue() override;
		virtual void PrintMessages() override;
	};

	struct DX10DebugMessage
	{
		D3D10_MESSAGE_SEVERITY Severity;
		String Message;
	};

	class ION_API DX10 : public RHI
	{
	public:
		virtual bool Init(GenericWindow* window) override;
		virtual bool InitWindow(GenericWindow& window) override;
		virtual void Shutdown() override;
		virtual void ShutdownWindow(GenericWindow& window) override;

		virtual void BeginFrame() override;
		virtual void EndFrame(GenericWindow& window) override;

		virtual void ChangeDisplayMode(GenericWindow& window, EDisplayMode mode, uint32 width, uint32 height) override;
		virtual void ResizeBuffers(GenericWindow& window, const TextureDimensions& size) override;

		virtual String GetCurrentDisplayName() override;

		static FORCEINLINE const char* GetFeatureLevelString();
		static FORCEINLINE D3D10_FEATURE_LEVEL1 GetFeatureLevel();

		static ID3D10Device* GetDevice();
		static IDXGISwapChain* GetSwapChain();
		static ID3D10RasterizerState* GetRasterizerState();
		static const String& GetDisplayName();

		static void SetSwapInterval(uint32 interval);
		static uint32 GetSwapInterval();

		using MessageArray = TArray<DX10DebugMessage>;
		static MessageArray GetDebugMessages();
		static void PrintDebugMessages();
		static void PrepareDebugMessageQueue();

		static void SetDebugName(ID3D10DeviceChild* object, const String& name);

	private:
		static void SetDisplayVersion(const char* version);
		static const char* GetShaderModel();

		static TShared<RHITexture> CreateRenderTarget();
		static TShared<RHITexture> CreateDepthStencil(uint32 width, uint32 height);

		virtual void InitImGuiBackend() override;
		virtual void ImGuiNewFrame() override;
		virtual void ImGuiRender(ImDrawData* drawData) override;
		virtual void ImGuiShutdown() override;

	private:
		static ID3D10Device1* s_Device;
		static IDXGISwapChain* s_SwapChain;
		static ID3D10DepthStencilState* s_DepthStencilState;
		static ID3D10RasterizerState* s_RasterizerState;
		/** Default blend state (nullptr) */
		static ID3D10BlendState1* s_BlendState;
		static ID3D10BlendState1* s_BlendStateTransparent;

		static ID3D10InfoQueue* s_DebugInfoQueue;

		static uint32 s_SwapInterval;

		static String s_DisplayName;

		static D3D10_FEATURE_LEVEL1 s_FeatureLevel;
		static bool s_Initialized;

		friend class DX10Renderer;
		friend class RHI;
	};

	inline const char* DX10::GetFeatureLevelString()
	{
		return DXCommon::D3DFeatureLevelToString((D3D_FEATURE_LEVEL)s_FeatureLevel);
	}

	inline D3D10_FEATURE_LEVEL1 DX10::GetFeatureLevel()
	{
		return s_FeatureLevel;
	}

	inline ID3D10Device* DX10::GetDevice()
	{
		return s_Device;
	}

	inline IDXGISwapChain* DX10::GetSwapChain()
	{
		return s_SwapChain;
	}

	inline ID3D10RasterizerState* DX10::GetRasterizerState()
	{
		return s_RasterizerState;
	}

	inline const String& DX10::GetDisplayName()
	{
		return s_DisplayName;
	}

	inline void DX10::SetSwapInterval(uint32 interval)
	{
		s_SwapInterval = interval;
	}

	inline uint32 DX10::GetSwapInterval()
	{
		return s_SwapInterval;
	}
}
