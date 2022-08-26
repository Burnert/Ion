#pragma once

#include "RHICore.h"
#include "Renderer/RendererCore.h"

struct ImDrawData;

namespace Ion
{
	// @TODO: Implement other Render APIs in the future
	enum class ERHI
	{
		None = 0,
		OpenGL,
		DX10,
		DX11,
		DX12,
		Vulkan,
	};

	inline String ERHIAsString(ERHI rhi)
	{
		switch (rhi)
		{
		case ERHI::None:   return "None";
		case ERHI::OpenGL: return "OpenGL";
		case ERHI::DX10:   return "DX10";
		case ERHI::DX11:   return "DX11";
		case ERHI::DX12:   return "DX12";
		case ERHI::Vulkan: return "Vulkan";
		}
		return "";
	}

	struct RHIWindowData
	{
		TRef<RHITexture>& ColorTexture;
		TRef<RHITexture>& DepthStencilTexture;
		void* NativeHandle;

		RHIWindowData(TRef<RHITexture>& color, TRef<RHITexture>& depthStencil, void* handle) :
			ColorTexture(color),
			DepthStencilTexture(depthStencil),
			NativeHandle(handle)
		{
		}
	};

	enum class EWindowDisplayMode : uint8
	{
		Windowed,
		BorderlessWindow,
		FullScreen,
	};

	enum class EDisplayMode : uint8;

	class ION_API RHI
	{
	public:
		static RHI* Create(ERHI rhi);
		static RHI* Get();

		static void SetEngineShadersPath(const FilePath& path);
		static const FilePath& GetEngineShadersPath();

		virtual Result<void, RHIError> Init(RHIWindowData& mainWindow) = 0;
		virtual Result<void, RHIError> InitWindow(RHIWindowData& window) = 0;
		virtual void Shutdown() = 0;
		virtual void ShutdownWindow(RHIWindowData& window) = 0;

		virtual Result<void, RHIError> BeginFrame() = 0;
		virtual Result<void, RHIError> EndFrame(RHIWindowData& window) = 0;

		virtual Result<void, RHIError> ChangeDisplayMode(RHIWindowData& window, EWindowDisplayMode mode, uint32 width, uint32 height) = 0;
		virtual Result<void, RHIError> ResizeBuffers(RHIWindowData& window, const TextureDimensions& size) = 0;

		virtual String GetCurrentDisplayName() = 0;

		virtual void InitImGuiBackend() = 0;
		virtual void ImGuiNewFrame() = 0;
		virtual void ImGuiRender(ImDrawData* drawData) = 0;
		virtual void ImGuiShutdown() = 0;

		static ERHI GetCurrent();

	private:
		static ERHI s_CurrentRHI;
		static RHI* s_RHI;

		static FilePath s_EngineShadersPath;
	};

	FORCEINLINE RHI* RHI::Get()
	{
		ionassert(s_RHI);
		return s_RHI;
	}

	FORCEINLINE const FilePath& RHI::GetEngineShadersPath()
	{
		return s_EngineShadersPath;
	}

	FORCEINLINE ERHI RHI::GetCurrent()
	{
		return s_CurrentRHI;
	}
}
