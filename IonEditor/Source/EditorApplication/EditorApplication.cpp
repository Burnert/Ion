#include "EditorPCH.h"

#include "EditorApplication.h"
#include "Renderer/Renderer.h"

#include "EditorLayer.h"

#include "ExampleModels.h"

#include "Engine/Engine.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/DirectionalLightComponent.h"

namespace Ion
{
namespace Editor
{
	// Constructed at the entry point
	EditorApplication::EditorApplication() :
		m_EventDispatcher(this),
		m_bViewportCaptured(false),
		m_EditorCameraMoveSpeed(5.0f),
		m_EditorMainWorld(nullptr),
		m_SelectedEntity(nullptr)
	{
		ionassert(!s_Instance);
		s_Instance = this;
	}

	EditorApplication::~EditorApplication()
	{
	}

	void EditorApplication::OnInit()
	{
		SetApplicationTitle(L"Ion Editor");
		GetWindow()->Maximize();

		m_EditorLayer = GetLayerStack()->PushLayer<EditorLayer>("EditorLayer");

		Renderer::Get()->SetVSyncEnabled(true);

		m_EditorCamera = Camera::Create();

		WorldInitializer worldInitializer { };
		m_EditorMainWorld = g_Engine->CreateWorld(worldInitializer);
		m_EditorMainWorld->GetScene()->SetActiveCamera(m_EditorCamera);

		//m_Scene = new Scene;

		m_EditorCameraTransform.SetLocation(Vector3(0.0f, 0.0f, 2.0f));
		m_EditorCamera->SetFOV(Math::Radians(90.0f));
		m_EditorCamera->SetNearClip(0.1f);
		m_EditorCamera->SetFarClip(100.0f);

		//m_Scene->SetActiveCamera(m_EditorCamera);

		InitExample(nullptr);

		// Engine testing:

		ComponentRegistry& registry = m_EditorMainWorld->GetComponentRegistry();

		m_TestMeshComponent = registry.CreateComponent<MeshComponent>();
		GetModelDeferred(g_ExampleModels[0], [&, this]
		{
			m_TestMeshComponent->SetMesh(g_ExampleModels[0].Mesh);
		});

		m_TestLightComponent = registry.CreateComponent<LightComponent>();
		m_TestLightComponent->SetLocation(Vector3(0.0f, 3.0f, 0.0f));
		m_TestLightComponent->GetLightDataRef().LightColor = Vector3(1.0f, 1.0f, 0.0f);
		m_TestLightComponent->GetLightDataRef().Intensity = 1.0f;
		m_TestLightComponent->GetLightDataRef().Falloff = 5.0f;

		m_TestDirLightComponent = registry.CreateComponent<DirectionalLightComponent>();
		m_TestDirLightComponent->SetRotation(Rotator({ -60.0f, 0.0f, 0.0f }));
		m_TestDirLightComponent->GetDirectionalLightDataRef().LightColor = Vector3(0.0f, 1.0f, 1.0f);
		m_TestDirLightComponent->GetDirectionalLightDataRef().Intensity = 1.0f;

		Entity* entity = m_EditorMainWorld->SpawnEntityOfClass<Entity>();
		entity->AddComponent(m_TestMeshComponent);
		entity->SetTransform(Transform(Vector3(0.0f, 0.0f, 2.0f)));
		entity->SetName("Mesh");

		Entity* lightEntity = m_EditorMainWorld->SpawnEntityOfClass<Entity>();
		lightEntity->AddComponent(m_TestLightComponent);
		lightEntity->SetName("Light");

		Entity* dirLightEntity = m_EditorMainWorld->SpawnEntityOfClass<Entity>();
		dirLightEntity->AddComponent(m_TestDirLightComponent);
		dirLightEntity->SetName("Directional Light");
	}

	void EditorApplication::OnUpdate(float deltaTime)
	{
		UpdateEditorCamera(deltaTime);

		//m_Scene->UpdateRenderData();
	}

	void EditorApplication::OnRender()
	{
	}

	void EditorApplication::OnShutdown()
	{
		if (m_EditorMainWorld)
		{
			g_Engine->DestroyWorld(m_EditorMainWorld);
			m_EditorMainWorld = nullptr;
		}
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
		yawDelta *= 0.2f;
		pitchDelta *= 0.2f;

		Rotator cameraRotation = m_EditorCameraTransform.GetRotation();
		cameraRotation.SetPitch(Math::Clamp(cameraRotation.Pitch() - pitchDelta, -89.99f, 89.99f));
		cameraRotation.SetYaw(cameraRotation.Yaw() - yawDelta);
		m_EditorCameraTransform.SetRotation(cameraRotation);
	}

	void EditorApplication::SetSelectedEntity(Entity* entity)
	{
		m_SelectedEntity = entity;
	}

	Scene* EditorApplication::GetEditorScene() const
	{
		return m_EditorMainWorld->GetScene();
	}

	void EditorApplication::TestChangeMesh()
	{
		static int32 c_Index = 1;
		m_TestMeshComponent->SetMesh(g_ExampleModels[c_Index].Mesh);
		c_Index = (c_Index + 1) % g_ExampleModels.size();
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
		if (m_bViewportCaptured)
		{
			DriveEditorCameraRotation(event.GetX(), event.GetY());
		}
	}

	void EditorApplication::UpdateEditorCamera(float deltaTime)
	{
		UpdateEditorCameraLocation(deltaTime);
		m_EditorCamera->SetTransform(m_EditorCameraTransform.GetMatrix());
	}

#pragma warning(disable:6031)
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
