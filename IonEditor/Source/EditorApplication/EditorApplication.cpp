#include "EditorPCH.h"

#include "EditorApplication.h"
#include "Renderer/Renderer.h"

#include "UserInterface/ImGui.h"

#include "ExampleModels.h"

namespace Ion
{
namespace Editor
{
	EditorApplication::EditorApplication() :
		m_EventDispatcher(this),
		m_bEditorCameraCaptured(false),
		m_ViewportSize({ })
	{

	}

	EditorApplication::~EditorApplication()
	{

	}

	void EditorApplication::OnInit()
	{
		SetApplicationTitle(L"Ion Editor");

		CreateViewportFramebuffer();

		m_Scene = Scene::Create();
		m_EditorCamera = Camera::Create();

		m_EditorCamera->SetTransform(Math::Translate(Vector3(0.0f, 0.0f, 2.0f)));
		m_EditorCamera->SetFOV(Math::Radians(90.0f));
		m_EditorCamera->SetNearClip(0.1f);
		m_EditorCamera->SetFarClip(100.0f);

		m_Scene->SetActiveCamera(m_EditorCamera);

		CreateExampleModels();
		for (ExampleModelData& model : g_ExampleModels)
		{
			model.SetOnInit([&]
			{
				m_Scene->AddDrawableObject(model.Mesh.get());
			});
		}
		LoadExampleModels();
	}

	void EditorApplication::OnUpdate(float deltaTime)
	{
		m_Scene->UpdateRenderData();

		TryResizeViewportFramebuffer();
		DrawEditorUI();
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

	void EditorApplication::DrawEditorUI()
	{
		static bool c_bEditorDockspaceOpen = true;

		ImGuiWindowFlags editorWindowFlags =
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Editor", &c_bEditorDockspaceOpen, editorWindowFlags);

		ImGui::PopStyleVar(3);

		ImGuiID dockSpaceID = ImGui::GetID("EditorDockSpace");
		ImGui::DockSpace(dockSpaceID);

		ImGui::End();

		DrawViewportWindow();
	}

	void EditorApplication::DrawViewportWindow()
	{
		static bool c_bOpen = true;

		ImGuiWindowFlags windowFlags =
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		char windowName[16];
		sprintf_s(windowName, "Viewport##%i", /* ID */ 0);
		ImGui::Begin(windowName, &c_bOpen, windowFlags);

		m_ViewportSize = UVector2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		ImGui::PopStyleVar(3);

		TextureDimensions viewportDimensions = m_ViewportFramebuffer->GetDimensions();
		ImGui::Image(m_ViewportFramebuffer->GetNativeID(), ImVec2((float)viewportDimensions.Width, (float)viewportDimensions.Height));

		ImGui::End();
	}

	void EditorApplication::OnWindowResizeEvent(const WindowResizeEvent& event)
	{

	}

	void EditorApplication::OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event)
	{
		if (event.GetMouseButton() == Mouse::Right)
		{
			// Capture the camera
			GetWindow()->LockCursor();
			GetWindow()->ShowCursor(false);

			m_bEditorCameraCaptured = true;
		}
	}

	void EditorApplication::OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event)
	{
		if (event.GetMouseButton() == Mouse::Right)
		{
			// Release the camera
			GetWindow()->UnlockCursor();
			GetWindow()->ShowCursor(true);

			m_bEditorCameraCaptured = false;
		}
	}

	void EditorApplication::OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event)
	{
		if (m_bEditorCameraCaptured)
		{
			float yawDelta = event.GetX() * 0.2f;
			float pitchDelta = event.GetY() * 0.2f;

			Rotator cameraRotation = m_EditorCameraTransform.GetRotation();

			cameraRotation.SetPitch(Math::Clamp(cameraRotation.Pitch() - pitchDelta, -89.99f, 89.99f));
			cameraRotation.SetYaw(cameraRotation.Yaw() - yawDelta);

			m_EditorCameraTransform.SetRotation(cameraRotation);
		}
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
		if (m_ViewportSize == UVector2(0, 0))
			return;

		TextureDimensions viewportDimensions = m_ViewportFramebuffer->GetDimensions();
		if (m_ViewportSize != UVector2(viewportDimensions.Width, viewportDimensions.Height))
		{
			ResizeViewportFramebuffer(m_ViewportSize);

			m_EditorCamera->SetAspectRatio((float)m_ViewportSize.x / (float)m_ViewportSize.y);
		}
	}
}
}
