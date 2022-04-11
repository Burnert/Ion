#include "EditorPCH.h"

#include "EditorApplication.h"
#include "EditorLayer.h"

#include "Editor/Viewport/EditorViewport.h"
#include "Editor/UI/ViewportUI.h"

#include "Renderer/Renderer.h"

#include "Engine/Engine.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/DirectionalLightComponent.h"
#include "Engine/Components/BehaviorComponent.h"
#include "Engine/Entity/MeshEntity.h"

#include "ExampleModels.h"

namespace Ion::Editor
{
	// Constructed at the entry point
	EditorApplication::EditorApplication() :
		m_EventDispatcher(this),
		m_EditorMainWorld(nullptr),
		m_SelectedEntity(nullptr),
		m_SelectedComponent(nullptr)
	{
		ionassert(!s_Instance);
		s_Instance = this;
	}

	EditorApplication::~EditorApplication()
	{
	}

	constexpr int32 g_nHarnasSqrt = 20;
	static TArray<Entity*> g_HarnasArray;

	void EditorApplication::OnInit()
	{
		TRACE_FUNCTION();

		SetApplicationTitle(L"Ion Editor");
		GetWindow()->Maximize();

		m_EditorLayer = GetLayerStack()->PushLayer<EditorLayer>("EditorLayer");

		m_MainViewport = AddViewport();
		
		TShared<EditorViewport> additionalViewport = AddViewport();
		additionalViewport->GetUI()->SetOpen(true);
		additionalViewport->GetUI()->SetWindowName("Viewport 2");

		TShared<EditorViewport> mainViewport = m_MainViewport.lock();
		m_EditorLayer->SetMainViewportOpenFlagPtr(&mainViewport->GetUI()->GetWindowOpenFlagRef());
		mainViewport->GetUI()->SetOpen(true);
		mainViewport->GetUI()->SetWindowName("Main Viewport");

		Renderer::Get()->SetVSyncEnabled(true);

		WorldInitializer worldInitializer { };
		m_EditorMainWorld = g_Engine->CreateWorld(worldInitializer);

		InitExample(nullptr);

		ComponentRegistry& registry = m_EditorMainWorld->GetComponentRegistry();

		MeshEntity* entity = m_EditorMainWorld->SpawnEntityOfClass<MeshEntity>();
		entity->SetTransform(Transform(Vector3(0.0f, -1.0f, 0.0f), Rotator(Vector3(-90.0f, 0.0f, 0.0f))));
		entity->SetName("Mesh");
		GetModelDeferred(g_ExampleModels[0], [entity](ExampleModelData& model)
		{
			entity->SetMesh(model.Mesh);
		});

		MeshComponent* meshComp = registry.CreateComponent<MeshComponent>();
		Entity* emptyEntity = m_EditorMainWorld->SpawnEntityOfClass<Entity>();
		emptyEntity->SetName("Empty Entity");
		emptyEntity->SetRootComponent(meshComp);
		emptyEntity->SetLocation(Vector3(2.0f, 0.0f, 0.0f));
		GetModelDeferred(g_ExampleModels[3], [meshComp](ExampleModelData& model)
		{
			meshComp->SetMesh(model.Mesh);
		});

		if (0)
		{
			for (int32 i = 0; i < g_nHarnasSqrt * g_nHarnasSqrt; ++i)
			{
				MeshEntity* ent = m_EditorMainWorld->SpawnEntityOfClass<MeshEntity>();
				GetModelDeferred(g_ExampleModels[1], [ent](ExampleModelData& model)
				{
					ent->SetMesh(model.Mesh);
				});
				float x = ((i % g_nHarnasSqrt)) * 0.2f - g_nHarnasSqrt * 0.1f;
				float z = ((i / g_nHarnasSqrt)) * 0.2f - g_nHarnasSqrt * 0.1f;
				ent->SetLocation(Vector3(x, -4.0f, z));
				ent->SetRotation(Rotator(Vector3(-90.0f, 0.0f, 0.0f)));
				ent->SetName(String("Harnas_") + ToString(i));
				g_HarnasArray.push_back(ent);
			}
		}
	}

	void EditorApplication::OnUpdate(float deltaTime)
	{
		TRACE_FUNCTION();

		DriveCameraUpdate(deltaTime);

		UpdateViewports(deltaTime);

		//static float c_Time = 0.0f;
		//c_Time += deltaTime;

		//int32 nHarnas = 0;
		//for (Entity*& harnas : g_HarnasArray)
		//{
		//	if (harnas && m_EditorMainWorld->DoesOwnEntity(harnas))
		//	{
		//		int32 x = nHarnas % g_nHarnasSqrt;
		//		int32 y = nHarnas / g_nHarnasSqrt;

		//		float wave = 0.0f;
		//		wave += sinf(((float)x + c_Time * 10.0f) / g_nHarnasSqrt * (float)Math::TWO_PI * 2.0f);
		//		wave += sinf(((float)y + c_Time * 10.0f) / g_nHarnasSqrt * (float)Math::TWO_PI * 2.0f);

		//		Vector3 location = harnas->GetLocation();
		//		location.y = wave;
		//		harnas->SetLocation(location);
		//	}
		//	else
		//	{
		//		harnas = nullptr;
		//	}
		//	nHarnas++;
		//}
	}

	void EditorApplication::PostUpdate()
	{
		PrepareEditorPass();
	}

	void EditorApplication::OnRender()
	{
		TRACE_FUNCTION();

		RenderViewports();
	}

	void EditorApplication::OnShutdown()
	{
		TRACE_FUNCTION();

		if (m_EditorMainWorld)
		{
			g_Engine->DestroyWorld(m_EditorMainWorld);
			m_EditorMainWorld = nullptr;
		}
	}

	void EditorApplication::OnEvent(const Event& event)
	{
		TRACE_FUNCTION();

		m_EventDispatcher.Dispatch(event);
	}

	TShared<EditorViewport>& EditorApplication::AddViewport()
	{
		UVector2 initialSize = { 1, 1 };
		TShared<EditorViewport> viewport = MakeShared<EditorViewport>(initialSize, (int32)m_Viewports.size());

		viewport->SetOnClicked([this](const GUID& clickedGuid)
		{
			SelectClickedObject(clickedGuid);
		});

		viewport->SetOnCaptured([this](EditorViewport& viewport)
		{
			ionassert(m_Viewports.find(viewport.GetGuid()) != m_Viewports.end());
			m_CapturedViewport = m_Viewports.at(viewport.GetGuid());
		});

		viewport->SetOnReleased([this](EditorViewport& viewport)
		{
			ionassert(m_Viewports.find(viewport.GetGuid()) != m_Viewports.end());
			m_CapturedViewport.reset();
		});

		auto& [id, ref] = *m_Viewports.insert({ viewport->GetGuid(), viewport }).first;

		return ref;
	}

	void EditorApplication::RemoveViewport(const GUID& viewportID)
	{
		auto it = m_Viewports.find(viewportID);
		if (it != m_Viewports.end())
		{
			if (m_CapturedViewport == it->second)
				m_CapturedViewport = nullptr;
			if (m_MainViewport.lock() == it->second)
				m_MainViewport.reset();
			m_Viewports.erase(viewportID);
		}
	}

	TShared<EditorViewport> EditorApplication::GetViewport(const GUID& viewportID)
	{
		auto it = m_Viewports.find(viewportID);
		if (it != m_Viewports.end())
			return it->second;
		return nullptr;
	}

	void EditorApplication::UpdateViewports(float deltaTime)
	{
		for (auto& [id, viewport] : m_Viewports)
		{
			viewport->Update(deltaTime);
		}
	}

	void EditorApplication::RenderViewports()
	{
		for (auto& [id, viewport] : m_Viewports)
		{
			viewport->Render();
		}
	}

	void EditorApplication::DrawViewports()
	{
		for (auto& [id, viewport] : m_Viewports)
		{
			viewport->GetUI()->Draw();
		}
	}

	void EditorApplication::SetSelectedEntity(Entity* entity)
	{
		m_SelectedEntity = entity;
	}

	void EditorApplication::SetSelectedComponent(Component* component)
	{
		m_SelectedComponent = component;
	}

	void EditorApplication::SelectObject(Entity* entity)
	{
		if (!entity || m_SelectedComponent && entity != m_SelectedComponent->GetOwner())
		{
			DeselectCurrentComponent();
		}
		SetSelectedEntity(entity);
	}

	void EditorApplication::SelectObject(Component* component)
	{
		// If the component has a different owner than the selected entity,
		// select the owner entity first.
		if (component && component->GetOwner() != m_SelectedEntity)
		{
			SetSelectedEntity(component->GetOwner());
		}
		SetSelectedComponent(component);
	}

	void EditorApplication::DeselectCurrentEntity()
	{
		SetSelectedEntity(nullptr);
		SetSelectedComponent(nullptr);
	}

	void EditorApplication::DeselectCurrentComponent()
	{
		SetSelectedComponent(nullptr);
	}

	void EditorApplication::DeselectCurrentObject()
	{
		if (m_SelectedComponent)
		{
			DeselectCurrentComponent();
			return;
		}
		DeselectCurrentEntity();
	}

	void EditorApplication::DeleteObject(Entity* entity)
	{
		ionassert(entity);
		entity->Destroy();
		SetSelectedEntity(nullptr);
	}

	bool EditorApplication::DeleteObject(Component* component)
	{
		ionassert(component);
		Entity* owner = component->GetOwner();
		ionassert(owner);

		// Destroy the component only if it's not the root
		if (owner->GetRootComponent() != component)
		{
			component->Destroy();
			SetSelectedComponent(nullptr);

			return true;
		}
		return false;
	}

	void EditorApplication::DeleteSelectedObject()
	{
		if (m_SelectedEntity)
		{
			if (m_SelectedComponent)
			{
				Entity* owner = m_SelectedComponent->GetOwner();
				ionassert(owner == m_SelectedEntity);

				// If it's the root component, it won't get deleted,
				// just delete the entity (@TODO: Ask the user first!!)
				if (DeleteObject(m_SelectedComponent))
				{
					return;
				}
			}

			DeleteObject(m_SelectedEntity);
		}
	}

	Scene* EditorApplication::GetEditorScene() const
	{
		return m_EditorMainWorld->GetScene();
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
		DriveCapturedViewportCameraRotation({ event.GetY(), event.GetX() });
	}

	void EditorApplication::OnKeyPressedEvent(const KeyPressedEvent& event)
	{
		switch (event.GetActualKeyCode())
		{
			case Key::Escape:
			{
				DeselectCurrentObject();
				break;
			}
			case Key::Delete:
			{
				DeleteSelectedObject();
				break;
			}
		}
	}

#pragma warning(disable:6031)
	void EditorApplication::DriveCameraUpdate(float deltaTime)
	{
		if (m_CapturedViewport)
		{
			Vector3 axisValues { };

			if (GetInputManager()->IsKeyPressed(Key::W))
			{
				axisValues.z += 1.0f;
			}
			if (GetInputManager()->IsKeyPressed(Key::S))
			{
				axisValues.z += -1.0f;
			}
			if (GetInputManager()->IsKeyPressed(Key::A))
			{
				axisValues.x += -1.0f;
			}
			if (GetInputManager()->IsKeyPressed(Key::D))
			{
				axisValues.x += 1.0f;
			}
			if (GetInputManager()->IsKeyPressed(Key::Q))
			{
				axisValues.y += -1.0f;
			}
			if (GetInputManager()->IsKeyPressed(Key::E))
			{
				axisValues.y += 1.0f;
			}

			DriveCapturedViewportCamera(axisValues, deltaTime);
		}
	}

	void EditorApplication::DriveCapturedViewportCamera(const Vector3& axisValues, float deltaTime)
	{
		if (m_CapturedViewport)
		{
			m_CapturedViewport->DriveCamera(axisValues, deltaTime);
		}
	}

	void EditorApplication::DriveCapturedViewportCameraRotation(const Vector2& axisValues)
	{
		if (m_CapturedViewport)
		{
			m_CapturedViewport->DriveCameraRotation(axisValues);
		}
	}

	REditorPassPrimitive EditorApplication::CreateEditorPassPrimitive(SceneComponent* component)
	{
		// @TODO: Implement the rest
		if (!component->IsOfType<MeshComponent>())
			return REditorPassPrimitive();

		MeshComponent* meshComponent = (MeshComponent*)component;
		RPrimitiveRenderProxy meshProxy = meshComponent->AsRenderProxy();

		REditorPassPrimitive prim { };
		prim.Guid = meshComponent->GetGUID();
		prim.VertexBuffer = meshProxy.VertexBuffer;
		prim.IndexBuffer = meshProxy.IndexBuffer;
		prim.UniformBuffer = meshProxy.UniformBuffer;
		prim.Transform = meshProxy.Transform;

		return prim;
	}

	void EditorApplication::PrepareEditorPass()
	{
		if (!m_EditorPassData)
		{
			m_EditorPassData = MakeShared<EditorPassData>();
		}

		// Prepare the Editor Render Pass

		m_EditorPassData->Primitives.clear();
		m_EditorPassData->SelectedPrimitives.clear();
		
		ComponentRegistry& registry = GetEditorWorld()->GetComponentRegistry();
		const ComponentDatabase* database = registry.GetComponentTypeDatabase();
		for (auto& [id, type] : database->RegisteredTypes)
		{
			// @TODO: For now only add mesh components, extend this in the future
			// if (type.bIsSceneComponent)
			if (type.Is<MeshComponent>())
			{
				auto& componentType = type;
				registry.ForEachComponentOfType(id, [this, &componentType](Component* component)
				{
					if (!((MeshComponent*)component)->GetMesh())
						return;

					m_EditorPassData->Primitives.push_back(CreateEditorPassPrimitive((SceneComponent*)component));
				});
			}
		}

		if (m_SelectedEntity)
		{
			// If there's at least one scene component selected,
			// don't render the whole entity, but only the selected components
			if (m_SelectedComponent && m_SelectedComponent->IsSceneComponent())
			{
				if (m_SelectedComponent->IsOfType<MeshComponent>())
				{
					MeshComponent* meshComponent = (MeshComponent*)m_SelectedComponent;
					if (!meshComponent->GetMesh())
						return;

					m_EditorPassData->SelectedPrimitives.push_back(CreateEditorPassPrimitive(meshComponent));
				}
			}
			else
			{
				for (SceneComponent* comp : m_SelectedEntity->GetAllOwnedSceneComponents())
				{
					if (comp->IsOfType<MeshComponent>())
					{
						MeshComponent* meshComponent = (MeshComponent*)comp;
						if (!meshComponent->GetMesh())
							return;

						m_EditorPassData->SelectedPrimitives.push_back(CreateEditorPassPrimitive(meshComponent));
					}
				}
			}
		}
	}

	void EditorApplication::SelectClickedObject(const GUID& clickedGuid)
	{
		// Deselect if Zero GUID (the clear space was clicked)
		if (clickedGuid.IsZero())
		{
			DeselectCurrentEntity();
		}
		else
		{
			DeselectCurrentComponent();
			Component* selectedComponent = GetEditorWorld()->GetComponentRegistry().FindComponentByGUID(clickedGuid);
			ionassert(selectedComponent);
			ionassert(selectedComponent->GetOwner());
			SelectObject(selectedComponent->GetOwner());
		}
	}

	EditorApplication* EditorApplication::s_Instance;
}
