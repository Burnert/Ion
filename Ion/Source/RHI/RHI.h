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

	class GenericWindow;
	enum class EDisplayMode : uint8;

	class ION_API RHI
	{
	public:
		static RHI* Create(ERHI rhi);
		static RHI* Get();

		virtual Result<void, RHIError> Init(GenericWindow* window) = 0;
		virtual Result<void, RHIError> InitWindow(GenericWindow& window) = 0;
		virtual void Shutdown() = 0;
		virtual void ShutdownWindow(GenericWindow& window) = 0;

		virtual Result<void, RHIError> BeginFrame() = 0;
		virtual Result<void, RHIError> EndFrame(GenericWindow& window) = 0;

		virtual Result<void, RHIError> ChangeDisplayMode(GenericWindow& window, EDisplayMode mode, uint32 width, uint32 height) = 0;
		virtual Result<void, RHIError> ResizeBuffers(GenericWindow& window, const TextureDimensions& size) = 0;

		virtual String GetCurrentDisplayName() = 0;

		virtual void InitImGuiBackend() = 0;
		virtual void ImGuiNewFrame() = 0;
		virtual void ImGuiRender(ImDrawData* drawData) = 0;
		virtual void ImGuiShutdown() = 0;

		static ERHI GetCurrent();

	private:
		static ERHI s_CurrentRHI;
		static RHI* s_RHI;
	};

	FORCEINLINE RHI* RHI::Get()
	{
		ionassert(s_RHI);
		return s_RHI;
	}

	FORCEINLINE ERHI RHI::GetCurrent()
	{
		return s_CurrentRHI;
	}
}
