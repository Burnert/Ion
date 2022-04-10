#include "EditorPCH.h"

#include "EditorApplication.h"
#include "EditorLayer.h"

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
		m_bViewportCaptured(false),
		m_EditorCameraMoveSpeed(5.0f),
		m_EditorMainWorld(nullptr),
		m_SelectedEntity(nullptr),
		m_SelectedComponent(nullptr),
		m_ClickedViewportPoint({ -1, -1 })
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

		Renderer::Get()->SetVSyncEnabled(true);

		m_EditorCamera = Camera::Create();

		WorldInitializer worldInitializer { };
		m_EditorMainWorld = g_Engine->CreateWorld(worldInitializer);
		m_EditorMainWorld->GetScene()->SetActiveCamera(m_EditorCamera);

		m_EditorCameraTransform.SetLocation(Vector3(0.0f, 0.0f, 2.0f));
		m_EditorCamera->SetFOV(Math::Radians(90.0f));
		m_EditorCamera->SetNearClip(0.1f);
		m_EditorCamera->SetFarClip(100.0f);

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

		SelectClickedObject();

		UpdateEditorCamera(deltaTime);

		static float c_Time = 0.0f;
		c_Time += deltaTime;

		int32 nHarnas = 0;
		for (Entity*& harnas : g_HarnasArray)
		{
			if (harnas && m_EditorMainWorld->DoesOwnEntity(harnas))
			{
				int32 x = nHarnas % g_nHarnasSqrt;
				int32 y = nHarnas / g_nHarnasSqrt;

				float wave = 0.0f;
				wave += sinf(((float)x + c_Time * 10.0f) / g_nHarnasSqrt * (float)Math::TWO_PI * 2.0f);
				wave += sinf(((float)y + c_Time * 10.0f) / g_nHarnasSqrt * (float)Math::TWO_PI * 2.0f);

				Vector3 location = harnas->GetLocation();
				location.y = wave;
				harnas->SetLocation(location);
			}
			else
			{
				harnas = nullptr;
			}
			nHarnas++;
		}
	}

	void EditorApplication::PostUpdate()
	{
		PrepareEditorPass();
	}

	void EditorApplication::OnRender()
	{
		TRACE_FUNCTION();

		RenderEditorScene();
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

	void EditorApplication::ClickViewport(const IVector2& position)
	{
		m_ClickedViewportPoint = position;
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
		if (m_bViewportCaptured)
		{
			DriveEditorCameraRotation(event.GetX(), event.GetY());
		}
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

		if (!m_EditorDataObjectID || !m_EditorDataSelected)
			return;

		m_EditorPassData->RTObjectID = m_EditorDataObjectID;
		m_EditorPassData->RTSelection = m_EditorDataSelected;

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
					m_EditorPassData->Primitives.push_back(CreateEditorPassPrimitive((SceneComponent*)component));
				});
			}
		}

		if (m_SelectedEntity)
		{
			//if (m_SelectedEntity->GetRootComponent()->IsOfType<MeshComponent>())
			//{
			//	MeshComponent* meshComponent = (MeshComponent*)m_SelectedEntity->GetRootComponent();
			//	m_EditorPassData->SelectedPrimitives.push_back(CreateEditorPassPrimitive(meshComponent));
			//}

			// If there's at least one scene component selected,
			// don't render the whole entity, but only the selected components
			if (m_SelectedComponent && m_SelectedComponent->IsSceneComponent())
			{
				if (m_SelectedComponent->IsOfType<MeshComponent>())
				{
					MeshComponent* meshComponent = (MeshComponent*)m_SelectedComponent;
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
						m_EditorPassData->SelectedPrimitives.push_back(CreateEditorPassPrimitive(meshComponent));
					}
				}
			}
		}
	}

	void EditorApplication::RenderEditorScene()
	{
		if (m_ViewportFramebuffer)
		{
			// Render the scene

			Renderer::Get()->SetRenderTarget(m_FinalSceneFramebuffer);
			Renderer::Get()->Clear(Vector4(0.1f, 0.1f, 0.1f, 1.0f));
			Renderer::Get()->RenderScene(GetEditorScene());

			// Render editor pass

			Renderer::Get()->RenderEditorPass(GetEditorScene(), *m_EditorPassData);

			// Render to viewport framebuffer

			Renderer::Get()->SetRenderTarget(m_ViewportFramebuffer);
			Renderer::Get()->Clear(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			Renderer::Get()->DrawEditorViewport(m_FinalSceneFramebuffer, m_EditorDataSelected);
		}
	}

	void EditorApplication::SelectClickedObject()
	{
		if (!m_EditorDataSelected ||
			m_ClickedViewportPoint.x == -1 || m_ClickedViewportPoint.y == -1)
			return;

		// Copy the object ID to the staging texture
		m_EditorDataObjectID->CopyTo(m_EditorDataObjectIDStaging);

		auto [width, height] = m_EditorDataObjectIDStaging->GetDimensions();

		// Get the data on the CPU
		void* buffer = nullptr;
		int32 lineSize = 0;
		m_EditorDataObjectIDStaging->Map(buffer, lineSize, ETextureMapType::Read);
		ionassert(lineSize / sizeof(GUID) >= width);

		// Get the pixel as GUID bytes
		GUID guid = GUID::Zero;
		memcpy(&guid, (GUID*)buffer + ((int32)m_ClickedViewportPoint.y * (lineSize / sizeof(GUID)) + (int32)m_ClickedViewportPoint.x), sizeof(GUID));

		LOG_DEBUG("Clicked component [{0}]", guid.ToString());

		// Deselect if Zero GUID (the clear space was clicked)
		if (guid.IsZero())
		{
			DeselectCurrentEntity();
		}
		else
		{
			Component* selectedComponent = GetEditorWorld()->GetComponentRegistry().FindComponentByGUID(guid);
			ionassert(selectedComponent);
			ionassert(selectedComponent->GetOwner());
			SelectObject(selectedComponent->GetOwner());
		}

		m_EditorDataObjectIDStaging->Unmap();

		m_ClickedViewportPoint = { -1, -1 };
	}

	void EditorApplication::CreateViewportFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc { };
		desc.DebugName = "Viewport";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateColorAttachment = true;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;

		m_ViewportFramebuffer = Texture::Create(desc);

		CreateFinalSceneFramebuffer(size);
		CreateEditorDataFramebuffer(size);
	}

	void EditorApplication::ResizeViewportFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc = m_ViewportFramebuffer->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_ViewportFramebuffer = Texture::Create(desc);

		ResizeFinalSceneFramebuffer(size);
		ResizeEditorDataFramebuffer(size);
	}

	void EditorApplication::TryResizeViewportFramebuffer(const UVector2& size)
	{
		if (size.x && size.y)
		{
			if (!m_ViewportFramebuffer)
			{
				CreateViewportFramebuffer(size);
			}

			TextureDimensions viewportDimensions = m_ViewportFramebuffer->GetDimensions();
			if (size != UVector2(viewportDimensions.Width, viewportDimensions.Height))
			{
				ResizeViewportFramebuffer(size);

				EditorApplication::Get()->GetEditorCamera()->SetAspectRatio((float)size.x / (float)size.y);
			}
		}
	}

	void EditorApplication::CreateFinalSceneFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc { };
		desc.DebugName = "FinalScene";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateColorAttachment = true;
		desc.bCreateDepthStencilAttachment = true;
		desc.bCreateDepthSampler = true;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;

		m_FinalSceneFramebuffer = Texture::Create(desc);
	}

	void EditorApplication::ResizeFinalSceneFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc = m_FinalSceneFramebuffer->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_FinalSceneFramebuffer = Texture::Create(desc);
	}

	void EditorApplication::CreateEditorDataFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc { };
		desc.DebugName = "EditorSelected";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateColorAttachment = true;
		desc.bCreateDepthStencilAttachment = true;
		desc.bCreateDepthSampler = true;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;

		m_EditorDataSelected = Texture::Create(desc);

		desc = { };
		desc.DebugName = "EditorObjectID";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateColorAttachment = true;
		desc.bCreateDepthStencilAttachment = true;
		desc.Format = ETextureFormat::UInt128GUID;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;

		m_EditorDataObjectID = Texture::Create(desc);

		desc = { };
		desc.DebugName = "EditorObjectIDStaging";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bCreateColorAttachment = true;
		desc.bAllowCPUReadAccess = true;
		desc.Usage = ETextureUsage::Staging;
		desc.Format = ETextureFormat::UInt128GUID;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;

		m_EditorDataObjectIDStaging = Texture::Create(desc);
	}

	void EditorApplication::ResizeEditorDataFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc = m_EditorDataSelected->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_EditorDataSelected = Texture::Create(desc);

		desc = m_EditorDataObjectID->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_EditorDataObjectID = Texture::Create(desc);

		desc = m_EditorDataObjectIDStaging->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_EditorDataObjectIDStaging = Texture::Create(desc);
	}

	EditorApplication* EditorApplication::s_Instance;
}
