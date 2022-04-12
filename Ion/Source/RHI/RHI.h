#pragma once

#include "Renderer/RendererCore.h"

struct ImDrawData;

namespace Ion
{
	// @TODO: Implement other Render APIs in the future
	enum class ERHI
	{
		None = 0,
		OpenGL,
		DX11,
		DX12,
		Vulkan,
	};

	class GenericWindow;

	class ION_API RHI
	{
	public:
		static bool Init(ERHI rhi, GenericWindow* window);
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame(GenericWindow& window);

		static void ChangeDisplayMode(GenericWindow& window, EDisplayMode mode, uint32 width, uint32 height);
		static void ResizeBuffers(GenericWindow& window, const TextureDimensions& size);

		static FORCEINLINE ERHI GetCurrent() { return s_CurrentRHI; }

		static const char* GetCurrentDisplayName();

		static void InitImGuiBackend();
		static void ImGuiNewFrame();
		static void ImGuiRender(ImDrawData* drawData);
		static void ImGuiShutdown();

	protected:
		static void SetCurrent(ERHI rhi);

	private:
		static ERHI s_CurrentRHI;
	};
}
