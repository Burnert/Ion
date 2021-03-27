#pragma once

namespace Ion
{
	// @TODO: Implement other Render APIs in the future
	enum class ERenderAPI
	{
		None,
		OpenGL
	};

	class ION_API RenderAPI
	{
	public:
		static bool Init(ERenderAPI api);

		static FORCEINLINE ERenderAPI GetCurrent() { return m_CurrentRenderAPI; }

		static const char* GetCurrentDisplayName();

	protected:
		static void SetCurrent(ERenderAPI api);

	private:
		static ERenderAPI m_CurrentRenderAPI;
	};
}
