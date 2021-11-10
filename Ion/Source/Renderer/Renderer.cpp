#include "IonPCH.h"

#include "Renderer.h"
#include "RenderCommand.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLRenderer.h"

namespace Ion
{
	// RenderCommandQueue: --------------------------------------

	void RenderCommandQueue::PushCommand(RenderCommand& command)
	{
		LockGuard lock(m_QueueMutex);

		m_RenderCommands.push_back(command);
	}

	void RenderCommandQueue::PushCommand(RenderCommandEx& command)
	{
		LockGuard lock(m_QueueMutex);

		m_RenderCommandsEx.push_back(command);
	}

	void RenderCommandQueue::Flush()
	{
		LockGuard lock(m_QueueMutex);

		ExecuteCommands();

		m_RenderCommands.clear();
		m_RenderCommandsEx.clear();
	}

	void RenderCommandQueue::ExecuteCommands()
	{
		for (RenderCommand& command : m_RenderCommands)
		{
			Execute(command);
			FreeCommand(command);
		}

		for (RenderCommandEx& command : m_RenderCommandsEx)
		{
			command.CommandFunction();
		}
	}

	void RenderCommandQueue::Execute(RenderCommand& command)
	{
		Renderer* renderer = Renderer::Get();
		switch (command.Type)
		{
			case ERenderCommandType::Clear:
			{
				renderer->Clear(*(Vector4*)command.Arguments);
				break;
			}
			case ERenderCommandType::RenderScene:
			{
				renderer->RenderScene(*(RSceneProxy*)command.Arguments);
				break;
			}
			case ERenderCommandType::SetVSyncEnabled:
			{
				renderer->SetVSyncEnabled(*(bool*)command.Arguments);
				renderer->m_bVSyncEnabled = renderer->IsVSyncEnabled();
				break;
			}
			case ERenderCommandType::SetViewportDimensions:
			{
				renderer->SetViewportDimensions(*(ViewportDimensions*)command.Arguments);
				renderer->m_ViewportDimensions = renderer->GetViewportDimensions();
				break;
			}
			case ERenderCommandType::SetPolygonDrawMode:
			{
				renderer->SetPolygonDrawMode(*(EPolygonDrawMode*)command.Arguments);
				renderer->m_PolygonDrawMode = renderer->GetPolygonDrawMode();
				break;
			}
		}
	}

	void RenderCommandQueue::FreeCommand(RenderCommand& command)
	{
		delete command.Arguments;
	}

	// Renderer: --------------------------------------------------

	Renderer* Renderer::Create()
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return s_Instance = new OpenGLRenderer();
		default:
			return nullptr;
		}
	}

	void Renderer::RenderFrame()
	{
		m_RenderCommandQueue.Flush();
	}

	void Renderer::PushRenderCommand(RenderCommand& command)
	{
		if (s_Instance)
		{
			s_Instance->m_RenderCommandQueue.PushCommand(command);
		}
	}

	void Renderer::PushRenderCommand(RenderCommandEx& command)
	{
		if (s_Instance)
		{
			s_Instance->m_RenderCommandQueue.PushCommand(command);
		}
	}

	void Renderer::UpdateAtomicVariables()
	{
		m_bVSyncEnabled = IsVSyncEnabled();
		m_ViewportDimensions = GetViewportDimensions();
		m_PolygonDrawMode = GetPolygonDrawMode();
	}

	Renderer* Renderer::s_Instance = nullptr;
}
