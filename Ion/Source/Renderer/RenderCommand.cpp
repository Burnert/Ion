#include "IonPCH.h"

#include "RenderCommand.h"
#include "Renderer.h"

#define PUSH_COMMAND(type, args) Renderer::PushRenderCommand(RenderCommand { type, args });

namespace Ion
{
	void RenderCommand::Clear(const Vector4& color)
	{
		PUSH_COMMAND(ERenderCommandType::Clear, new Vector4(color));
	}

	void RenderCommand::RenderScene(Scene* scene)
	{
		RSceneProxy* sceneProxy = new RSceneProxy;
		scene->CopySceneData(*sceneProxy);
		PUSH_COMMAND(ERenderCommandType::RenderScene, sceneProxy);
	}

	void RenderCommand::SetVSyncEnabled(bool bEnabled)
	{
		PUSH_COMMAND(ERenderCommandType::SetVSyncEnabled, new bool(bEnabled));
	}

	void RenderCommand::SetViewportDimensions(const ViewportDimensions& dimensions)
	{
		PUSH_COMMAND(ERenderCommandType::SetViewportDimensions, new ViewportDimensions(dimensions));
	}

	void RenderCommand::SetPolygonDrawMode(EPolygonDrawMode drawMode)
	{
		PUSH_COMMAND(ERenderCommandType::SetPolygonDrawMode, new EPolygonDrawMode(drawMode));
	}
}
