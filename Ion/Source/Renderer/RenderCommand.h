#pragma once

namespace Ion
{
	enum class ERenderCommandType
	{
		Null = 0,
		Clear,
		RenderScene,
		SetVSyncEnabled,
		SetViewportDimensions,
		SetPolygonDrawMode,
	};

	class Scene;
	struct ViewportDimensions;
	enum class EPolygonDrawMode : uint8;

	struct ION_API RenderCommand
	{
		static void Clear(const Vector4& color);
		static void RenderScene(Scene* scene);
		static void SetVSyncEnabled(bool bEnabled);
		static void SetViewportDimensions(const ViewportDimensions& dimensions);
		static void SetPolygonDrawMode(EPolygonDrawMode drawMode);

		ERenderCommandType Type;
		void* Arguments;
	};

	struct RenderCommandEx
	{
		using CommandFunctionT = TFunction<void()>;

		RenderCommandEx(CommandFunctionT function)
			: CommandFunction(function)
		{ }

		CommandFunctionT CommandFunction;
	};
}
