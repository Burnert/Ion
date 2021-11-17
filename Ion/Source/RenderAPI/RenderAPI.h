#pragma once

struct ImDrawData;

namespace Ion
{
	// @TODO: Implement other Render APIs in the future
	enum class ERenderAPI
	{
		None,
		OpenGL,
		DX11,
		DX12,
		Vulkan,
	};

	class GenericWindow;

	class ION_API RenderAPI
	{
	public:
		static bool Init(ERenderAPI api, GenericWindow* window);
		static void Shutdown();

		static void BeginFrame();
		static void EndFrame(GenericWindow& window);

		static FORCEINLINE ERenderAPI GetCurrent() { return s_CurrentRenderAPI; }

		static const char* GetCurrentDisplayName();

		static void InitImGuiBackend();
		static void ImGuiNewFrame();
		static void ImGuiRender(ImDrawData* drawData);
		static void ImGuiShutdown();

	protected:
		static void SetCurrent(ERenderAPI api);

	private:
		static ERenderAPI s_CurrentRenderAPI;
	};
}
