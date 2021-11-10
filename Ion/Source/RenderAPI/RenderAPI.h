#pragma once

struct ImDrawData;

namespace Ion
{
	class GenericWindow;

	// @TODO: Implement other Render APIs in the future
	enum class ERenderAPI
	{
		None,
		OpenGL,
		DirectX,
		DX12,
		Vulkan,
	};

	class ION_API RenderAPI
	{
	public:
		static bool Init(ERenderAPI api);

		static FORCEINLINE ERenderAPI GetCurrent() { return m_CurrentRenderAPI; }

		static const char* GetCurrentDisplayName();

		// OpenGL: --------------------------------------------------

		static void CreateContext(GenericWindow& window);
		static void MakeContextCurrent(GenericWindow& window);
		static void UseShareContext(GenericWindow& window);

		// ImGui: --------------------------------------------------

		static void InitImGuiBackend();
		static void ImGuiNewFrame();
		static void ImGuiRender(ImDrawData* drawData);
		static void ImGuiShutdown();

	protected:
		static void SetCurrent(ERenderAPI api);

	private:
		static ERenderAPI m_CurrentRenderAPI;
	};
}
