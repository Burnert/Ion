#include "EditorPCH.h"

#include "EditorApplication.h"
#include "Renderer/Renderer.h"

#include "EditorLayer.h"

#include "ExampleModels.h"

namespace Ion
{
namespace Editor
{
	EditorApplication::EditorApplication() :
		m_EventDispatcher(this),
		m_ViewportSize({ }),
		m_bViewportCaptured(false),
		m_EditorCameraMoveSpeed(5.0f)
	{
		s_Instance = this;
	}

	EditorApplication::~EditorApplication()
	{

	}

	void EditorApplication::OnInit()
	{
		SetApplicationTitle(L"Ion Editor");
		GetWindow()->Maximize();

		CreateViewportFramebuffer();

		GetLayerStack()->PushLayer<EditorLayer>("EditorLayer");

		Renderer::Get()->SetVSyncEnabled(true);

		m_Scene = Scene::Create();
		m_EditorCamera = Camera::Create();

		m_EditorCameraTransform.SetLocation(Vector3(0.0f, 0.0f, 2.0f));
		m_EditorCamera->SetFOV(Math::Radians(90.0f));
		m_EditorCamera->SetNearClip(0.1f);
		m_EditorCamera->SetFarClip(100.0f);

		m_Scene->SetActiveCamera(m_EditorCamera);

		InitExample(m_Scene);
	}

	void EditorApplication::OnUpdate(float deltaTime)
	{
		UpdateEditorCamera(deltaTime);

		m_Scene->UpdateRenderData();
		TryResizeViewportFramebuffer();
	}

	void EditorApplication::OnRender()
	{
		Renderer* renderer = GetRenderer();

		// Render to viewport

		renderer->SetRenderTarget(m_ViewportFramebuffer);

		renderer->Clear(Vector4(0.1f, 0.1f, 0.1f, 1.0f));
		renderer->RenderScene(m_Scene);
	}

	void EditorApplication::OnShutdown()
	{

	}

	void EditorApplication::OnEvent(const Event& event)
	{
		m_EventDispatcher.Dispatch(event);
	}

	void EditorApplication::CaptureViewport(bool bCapture)
	{
		if (m_bViewportCaptured != bCapture)
		{
			m_bViewportCaptured = bCapture;

			GetWindow()->ShowCursor(!bCapture);
			GetWindow()->LockCursor(bCapture);
		}
	}

	void EditorApplication::DriveEditorCameraRotation(float yawDelta, float pitchDelta)
	{
		if (m_bViewportCaptured)
		{
			yawDelta *= 0.2f;
			pitchDelta *= 0.2f;

			Rotator cameraRotation = m_EditorCameraTransform.GetRotation();
			cameraRotation.SetPitch(Math::Clamp(cameraRotation.Pitch() - pitchDelta, -89.99f, 89.99f));
			cameraRotation.SetYaw(cameraRotation.Yaw() - yawDelta);
			m_EditorCameraTransform.SetRotation(cameraRotation);
		}
	}

	void EditorApplication::ExitEditor()
	{
		s_Instance->Exit();
	}

	void EditorApplication::OnWindowResizeEvent(const WindowResizeEvent& event)
	{
	}

	void EditorApplication::OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event)
	{
	}

	void EditorApplication::OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event)
	{
	}

	void EditorApplication::OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event)
	{
	}

	void EditorApplication::CreateViewportFramebuffer()
	{
		WindowDimensions windowDimensions = GetWindow()->GetDimensions();
		
		TextureDescription desc { };
		desc.Dimensions.Width = windowDimensions.Width;
		desc.Dimensions.Height = windowDimensions.Height;
		desc.bUseAsRenderTarget = true;
		desc.bCreateDepthStencilAttachment = true;

		m_ViewportFramebuffer = Texture::Create(desc);
	}
	void EditorApplication::ResizeViewportFramebuffer(const UVector2& size)
	{
		if (!m_ViewportFramebuffer)
			return;

		TextureDescription desc = m_ViewportFramebuffer->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_ViewportFramebuffer = Texture::Create(desc);
	}

	void EditorApplication::TryResizeViewportFramebuffer()
	{
		if (m_ViewportSize.x && m_ViewportSize.y)
		{
			TextureDimensions viewportDimensions = m_ViewportFramebuffer->GetDimensions();
			if (m_ViewportSize != UVector2(viewportDimensions.Width, viewportDimensions.Height))
			{
				ResizeViewportFramebuffer(m_ViewportSize);

				m_EditorCamera->SetAspectRatio((float)m_ViewportSize.x / (float)m_ViewportSize.y);
			}
		}
	}

	void EditorApplication::UpdateEditorCamera(float deltaTime)
	{
		UpdateEditorCameraLocation(deltaTime);
		m_EditorCamera->SetTransform(m_EditorCameraTransform.GetMatrix());
	}

	void EditorApplication::UpdateEditorCameraLocation(float deltaTime)
	{
		if (m_bViewportCaptured)
		{
			if (GetInputManager()->IsKeyPressed(Key::W))
			{
				m_EditorCameraTransform += m_EditorCameraTransform.GetForwardVector() * deltaTime * m_EditorCameraMoveSpeed;
			}
			if (GetInputManager()->IsKeyPressed(Key::S))
			{
				m_EditorCameraTransform += -m_EditorCameraTransform.GetForwardVector() * deltaTime * m_EditorCameraMoveSpeed;
			}
			if (GetInputManager()->IsKeyPressed(Key::A))
			{
				m_EditorCameraTransform += -m_EditorCameraTransform.GetRightVector() * deltaTime * m_EditorCameraMoveSpeed;
			}
			if (GetInputManager()->IsKeyPressed(Key::D))
			{
				m_EditorCameraTransform += m_EditorCameraTransform.GetRightVector() * deltaTime * m_EditorCameraMoveSpeed;
			}
			if (GetInputManager()->IsKeyPressed(Key::Q))
			{
				m_EditorCameraTransform += Vector3(0.0f, -1.0f, 0.0f) * deltaTime * m_EditorCameraMoveSpeed;
			}
			if (GetInputManager()->IsKeyPressed(Key::E))
			{
				m_EditorCameraTransform += Vector3(0.0f, 1.0f, 0.0f) * deltaTime * m_EditorCameraMoveSpeed;
			}
		}
	}

	EditorApplication* EditorApplication::s_Instance;
}
}
