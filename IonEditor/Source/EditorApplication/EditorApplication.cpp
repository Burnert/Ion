#include "EditorPCH.h"

#include "EditorApplication.h"
#include "EditorLayer.h"

#include "Editor/Viewport/EditorViewport.h"
#include "Editor/ContentBrowser/ContentBrowser.h"
#include "Editor/LogSettings.h"

#include "Editor/EditorAssets.h"

#include "Renderer/Renderer.h"

#include "Engine/Engine.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/DirectionalLightComponent.h"
#include "Engine/Components/BehaviorComponent.h"
#include "Engine/Entity/MeshEntity.h"

#include "Material/Material.h"

#include "ExampleModels.h"

#include "Core/Logging/LogManager.h"

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

		EditorIcons::LoadTextures();
		EditorBillboards::LoadTextures();
		EditorMeshes::Init();

		m_EditorLayer = GetLayerStack()->PushLayer<EditorLayer>("EditorLayer");

		m_MainViewport = AddViewport();
		
		std::shared_ptr<EditorViewport> additionalViewport = AddViewport();
		additionalViewport->GetUI()->SetOpen(true);
		additionalViewport->GetUI()->SetWindowName("Viewport 2");

		std::shared_ptr<EditorViewport> mainViewport = m_MainViewport.lock();
		m_EditorLayer->SetMainViewportOpenFlagPtr(&mainViewport->GetUI()->GetWindowOpenFlagRef());
		mainViewport->GetUI()->SetOpen(true);
		mainViewport->GetUI()->SetWindowName("Main Viewport");

		m_ContentBrowser = std::make_shared<ContentBrowser>();
		m_ContentBrowser->AddUI();

		m_LogSettings = std::make_shared<LogSettings>();

		Asset::Resolve("[Engine]/Materials/DefaultMaterial").Unwrap();
		Asset::Resolve("[Engine]/Textures/White").UnwrapOr(Asset::None);
		Asset::Resolve("[Engine]/Lol/Something")
			.Ok([](const Asset& asset)
			{
				EditorLogger.Info(asset->GetDefinitionPath().ToString());
			})
			.Err<FileNotFoundError>([](auto& error)
			{
				EditorLogger.Error("Cannot find asset file.");
			});

		Renderer::Get()->SetVSyncEnabled(true);

		WorldInitializer worldInitializer { };
		m_EditorMainWorld = g_Engine->CreateWorld(worldInitializer);

		AssetRegistry::RegisterVirtualRoot("[Example]", EnginePath::GetEnginePath() + "../IonExample/Assets");
		AssetRegistry::RegisterAssetsInVirtualRoot("[Example]");

		// Unicode test
		{
			EditorLogger.Info("UTF8: {}", u8"żółw");

			const char mbTestU1[] = u8"öÄÖÅäåèé";
			const wchar wcTestU1[] = L"öÄÖÅäåèé";
			const char mbTestU2[] = u8"żęąłóćź";
			const wchar wcTestU2[] = L"żęąłóćź";
			const char mbTestU3[] = u8"中文";
			const wchar wcTestU3[] = L"中文";

			EditorLogger.Trace("mbTestU1 = {}", mbTestU1);
			EditorLogger.Trace(L"wcTestU1 = {}", wcTestU1);
			EditorLogger.Trace("mbTestU2 = {}", mbTestU2);
			EditorLogger.Trace(L"wcTestU2 = {}", wcTestU2);
			EditorLogger.Trace("mbTestU3 = {}", mbTestU3);
			EditorLogger.Trace(L"wcTestU3 = {}", wcTestU3);

			char mbTestU1Out[50] = { };
			StringConverter::W2MB(wcTestU1, mbTestU1Out);
			wchar wcTestU1Out[50] = { };
			StringConverter::MB2W(mbTestU1, wcTestU1Out);

			String mbTest1 = StringConverter::WStringToString(wcTestU1);
			ionassert(mbTest1 == mbTestU1);
			WString wcTest1 = StringConverter::StringToWString(mbTestU1);
			ionassert(wcTest1 == wcTestU1);

			String mbTest2 = StringConverter::WStringToString(wcTestU2);
			ionassert(mbTest2 == mbTestU2);
			WString wcTest2 = StringConverter::StringToWString(mbTestU2);
			ionassert(wcTest2 == wcTestU2);

			String mbTest3 = StringConverter::WStringToString(wcTestU3);
			ionassert(mbTest3 == mbTestU3);
			WString wcTest3 = StringConverter::StringToWString(mbTestU3);
			ionassert(wcTest3 == wcTestU3);

			EditorLogger.Trace("mbTest1 = {}", mbTest1);
			EditorLogger.Trace(L"wcTest1 = {}", wcTest1);
			EditorLogger.Trace("mbTest2 = {}", mbTest2);
			EditorLogger.Trace(L"wcTest2 = {}", wcTest2);
			EditorLogger.Trace("mbTest3 = {}", mbTest3);
			EditorLogger.Trace(L"wcTest3 = {}", wcTest3);
		}

		//InitExample(nullptr);

		auto& hierarchy = LogManager::GetLoggerHierarchy();

		ComponentRegistry& registry = m_EditorMainWorld->GetComponentRegistry();

		//Asset materialAsset = AssetFinder(EnginePath::GetEngineContentPath() + L"Materials/DefaultMaterial.iasset").Resolve();

		//std::shared_ptr<Material> material = MaterialRegistry::QueryMaterial(materialAsset);
		//material->AddUsage(EShaderUsage::StaticMesh);
		//material->CompileShaders();

		//std::this_thread::sleep_for(std::chrono::milliseconds(60));

		//Asset materialInstanceAsset = AssetFinder(FilePath(L"../IonExample/Assets/Materials/ciupaga.iasset")).Resolve();

		//std::shared_ptr<MaterialInstance> materialInstance = MaterialRegistry::QueryMaterialInstance(materialInstanceAsset);
		//MaterialParameterInstanceScalar* paramBrightness = materialInstance->GetMaterialParameterInstanceTyped<MaterialParameterInstanceScalar>("Brightness");
		//MaterialParameterInstanceVector* paramColor = materialInstance->GetMaterialParameterInstanceTyped<MaterialParameterInstanceVector>("Color");

		//ionassert(paramBrightness->GetValue() == paramBrightness->GetParameterScalar()->GetDefaultValue());
		//ionassert(paramColor->GetValue() == paramColor->GetParameterVector()->GetDefaultValue());

		MeshEntity* meshEntity = m_EditorMainWorld->SpawnEntityOfClass<MeshEntity>();

		TResourcePtr<MeshResource> meshResource = MeshResource::Query(Asset::Resolve("[Example]/models/4pak").UnwrapOr(Asset::None));
		std::shared_ptr<Mesh> mesh = Mesh::CreateFromResource(meshResource);
		meshEntity->SetMesh(mesh);
		meshEntity->SetName("MaterialExampleMesh");

		//mesh->AssignMaterialToSlot(0, materialInstance);

		//if (0)
		//{
		//	for (int32 i = 0; i < g_nHarnasSqrt * g_nHarnasSqrt; ++i)
		//	{
		//		MeshEntity* ent = m_EditorMainWorld->SpawnEntityOfClass<MeshEntity>();
		//		GetModelDeferred(g_ExampleModels[1], [ent](ExampleModelData& model)
		//		{
		//			ent->SetMesh(model.Mesh);
		//		});
		//		float x = ((i % g_nHarnasSqrt)) * 0.2f - g_nHarnasSqrt * 0.1f;
		//		float z = ((i / g_nHarnasSqrt)) * 0.2f - g_nHarnasSqrt * 0.1f;
		//		ent->SetLocation(Vector3(x, -4.0f, z));
		//		ent->SetRotation(Rotator(Vector3(-90.0f, 0.0f, 0.0f)));
		//		ent->SetName(String("Harnas_") + ToString(i));
		//		g_HarnasArray.push_back(ent);
		//	}
		//}
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

		Renderer::Get()->UnbindResources();

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

	std::shared_ptr<EditorViewport>& EditorApplication::AddViewport()
	{
		UVector2 initialSize = { 1, 1 };
		std::shared_ptr<EditorViewport> viewport = std::make_shared<EditorViewport>(initialSize, (int32)m_Viewports.size());

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

	std::shared_ptr<EditorViewport> EditorApplication::GetViewport(const GUID& viewportID)
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

	void EditorApplication::DuplicateObject(Entity* entity)
	{
		ionassert(entity);
		Entity* newEntity = GetEditorWorld()->DuplicateEntity(entity);
		SetSelectedEntity(newEntity);
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
		REditorPassPrimitive prim { };
		prim.Guid = component->GetGUID();

		// @TODO: Implement the rest
		if (component->IsOfType<MeshComponent>())
		{
			MeshComponent* meshComponent = (MeshComponent*)component;
			RPrimitiveRenderProxy meshProxy = meshComponent->AsRenderProxy();

			prim.VertexBuffer = meshProxy.VertexBuffer;
			prim.IndexBuffer = meshProxy.IndexBuffer;
			prim.UniformBuffer = meshProxy.UniformBuffer;
			prim.Transform = meshProxy.Transform;
		}

		return prim;
	}

	REditorPassBillboardPrimitive EditorApplication::CreateEditorPassBillboard(SceneComponent* component)
	{
		REditorPassBillboardPrimitive prim;
		prim.Guid = component->GetGUID();
		prim.BillboardRenderProxy.Texture = EditorBillboards::GetComponentBillboardTexture(component->GetFinalTypeID()).get();
		prim.BillboardRenderProxy.LocationWS = component->GetWorldTransform().GetLocation();
		prim.BillboardRenderProxy.Scale = 0.2f;

		return prim;
	}

	void EditorApplication::PrepareEditorPass()
	{
		if (!m_EditorPassData)
		{
			m_EditorPassData = std::make_shared<EditorPassData>();
		}

		// Prepare the Editor Render Pass

		m_EditorPassData->Primitives.clear();
		m_EditorPassData->SelectedPrimitives.clear();
		m_EditorPassData->Billboards.clear();
		m_EditorPassData->SelectedBillboards.clear();

		// Prepare ObjectID data (meshes and billboards will render with component GUIDs as their colors)
		
		ComponentRegistry& registry = GetEditorWorld()->GetComponentRegistry();
		registry.ForEachSceneComponent([this](SceneComponent* component)
		{
			// @TODO: this is here only because AssetManager doesn't do its job...
			if (component->IsOfType<MeshComponent>())
			{
				MeshComponent* meshComponent = (MeshComponent*)component;
				// Don't draw the billboard if the component has a mesh
				// @TODO: Make some kind of a function to check if the mesh can be rendered
				if (meshComponent->GetMesh() && meshComponent->GetMesh()->GetVertexBufferRaw())
				{
					m_EditorPassData->Primitives.push_back(CreateEditorPassPrimitive(component));
					return;
				}
			}
			m_EditorPassData->Billboards.push_back(CreateEditorPassBillboard(component));
		});

		// Prepare selected components data (for outline drawing)

		if (m_SelectedEntity)
		{
			TArray<SceneComponent*> selectedComponents;

			// If there's at least one scene component selected,
			// don't render the whole entity, but only the selected components
			if (m_SelectedComponent && m_SelectedComponent->IsSceneComponent())
			{
				selectedComponents.push_back((SceneComponent*)m_SelectedComponent);
			}
			else
			{
				TArray<SceneComponent*> entityComponents = m_SelectedEntity->GetAllOwnedSceneComponents();
				selectedComponents.reserve(entityComponents.size());
				for (SceneComponent* comp : entityComponents)
				{
					selectedComponents.push_back(comp);
				}
			}

			// Add the selected components to the Editor Render Pass Data structure
			for (SceneComponent* comp : selectedComponents)
			{
				if (comp->IsOfType<MeshComponent>())
				{
					MeshComponent* meshComponent = (MeshComponent*)comp;
					// Don't draw the billboard if the mesh component has a mesh
					if (meshComponent->GetMesh() && meshComponent->GetMesh()->GetVertexBufferRaw())
					{
						m_EditorPassData->SelectedPrimitives.push_back(CreateEditorPassPrimitive(comp));
						continue;
					}
				}
				m_EditorPassData->SelectedBillboards.push_back(CreateEditorPassBillboard(comp));
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
