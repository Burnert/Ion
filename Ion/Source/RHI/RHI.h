#pragma once

#include "Renderer/RendererCore.h"

struct ImDrawData;

namespace Ion
{
	REGISTER_LOGGER(RHILogger, "RHI");

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

	class ION_API RHI
	{
	public:
		static RHI* Create(ERHI rhi);
		static RHI* Get();

		virtual bool Init(GenericWindow* window) = 0;
		virtual bool InitWindow(GenericWindow& window) = 0;
		virtual void Shutdown() = 0;
		virtual void ShutdownWindow(GenericWindow& window) = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame(GenericWindow& window) = 0;

		virtual void ChangeDisplayMode(GenericWindow& window, EDisplayMode mode, uint32 width, uint32 height) = 0;
		virtual void ResizeBuffers(GenericWindow& window, const TextureDimensions& size) = 0;

		virtual String GetCurrentDisplayName() = 0;

		virtual void InitImGuiBackend() = 0;
		virtual void ImGuiNewFrame() = 0;
		virtual void ImGuiRender(ImDrawData* drawData) = 0;
		virtual void ImGuiShutdown() = 0;

		static FORCEINLINE ERHI GetCurrent()
		{
			return s_CurrentRHI;
		}

	private:
		static ERHI s_CurrentRHI;
		static RHI* s_RHI;
	};

	FORCEINLINE RHI* RHI::Get()
	{
		ionassert(s_RHI);
		return s_RHI;
	}
}
