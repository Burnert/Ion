#include "EditorPCH.h"

#include "EditorApplication.h"
#include "Renderer/Renderer.h"

#include "EditorLayer.h"

#include "ExampleModels.h"

#include "Engine/Engine.h"
#include "Engine/WorldTree.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/DirectionalLightComponent.h"
#include "Engine/Components/BehaviorComponent.h"

#include "Core/Container/OldTree.h"
#include "Core/Container/Tree.h"
#include "Core/Memory/PoolAllocator.h"

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
		m_SelectedEntity(nullptr),
		m_SelectedComponent(nullptr)
	{
		ionassert(!s_Instance);
		s_Instance = this;
	}

	EditorApplication::~EditorApplication()
	{
	}

	constexpr int32 g_nHarnasSqrt = 100;
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
		GetModelDeferred(g_ExampleModels[0], [this](ExampleModelData& model)
		{
			m_TestMeshComponent->SetMesh(model.Mesh);
		});

		m_TestLightComponent = registry.CreateComponent<LightComponent>();
		m_TestLightComponent->GetLightDataRef().LightColor = Vector3(1.0f, 1.0f, 0.0f);
		m_TestLightComponent->GetLightDataRef().Intensity = 1.0f;
		m_TestLightComponent->GetLightDataRef().Falloff = 5.0f;

		m_TestDirLightComponent = registry.CreateComponent<DirectionalLightComponent>();
		m_TestDirLightComponent->GetDirectionalLightDataRef().LightColor = Vector3(1.0f, 1.0f, 1.0f);
		m_TestDirLightComponent->GetDirectionalLightDataRef().Intensity = 1.0f;

		Entity* entity = m_EditorMainWorld->SpawnEntityOfClass<Entity>();
		entity->SetRootComponent(m_TestMeshComponent);
		entity->SetTransform(Transform(Vector3(0.0f, -1.0f, 0.0f), Rotator(Vector3(-90.0f, 0.0f, 0.0f))));
		entity->SetName("Mesh");

		Entity* lightEntity = m_EditorMainWorld->SpawnAndAttachEntityOfClass<Entity>(entity);
		lightEntity->SetRootComponent(m_TestLightComponent);
		lightEntity->SetLocation(Vector3(0.0f, 1.0f, 0.0f));
		lightEntity->SetName("Light");

		Entity* dirLightEntity = m_EditorMainWorld->SpawnEntityOfClass<Entity>();
		dirLightEntity->SetRootComponent(m_TestDirLightComponent);
		dirLightEntity->SetRotation(Rotator({ -60.0f, 0.0f, 0.0f }));
		dirLightEntity->SetName("DirectionalLight");

		for (int32 i = 0; i < 6; i++)
		{
			Entity* childEntity = m_EditorMainWorld->SpawnEntityOfClass<Entity>();
			childEntity->SetName(String("Test") + ToString(i));
			childEntity->AttachTo(lightEntity);
		}

		if (0)
		{
			Entity* lastEntity = nullptr;
			for (int32 i = 0; i < 6; ++i)
			{
				MeshComponent* meshComp1 = registry.CreateComponent<MeshComponent>();
				GetModelDeferred(g_ExampleModels[i], [meshComp1, this](ExampleModelData& model)
				{
					meshComp1->SetMesh(model.Mesh);
				});
				Entity* entity1 = m_EditorMainWorld->SpawnEntityOfClass<Entity>();
				entity1->SetRootComponent(meshComp1);
				entity1->SetName("Mesh_" + ToString(i));
				if (lastEntity)
				{
					entity1->AttachTo(lastEntity);
				}
				lastEntity = entity1;
			}
		}
		
		if (1)
		{
			Entity* entity2 = m_EditorMainWorld->SpawnEntityOfClass<Entity>();
			entity2->SetName("MultipleMeshes");
			SceneComponent* lastComp = entity2->GetRootComponent();
			for (int32 i = 0; i < 6; ++i)
			{
				MeshComponent* meshComp1 = registry.CreateComponent<MeshComponent>();
				GetModelDeferred(g_ExampleModels[i], [meshComp1, this](ExampleModelData& model)
				{
					meshComp1->SetMesh(model.Mesh);
				});
				meshComp1->SetName("Mesh_" + ToString(i));
				meshComp1->AttachTo(lastComp);
				lastComp = meshComp1;
			}
			BehaviorComponent* behaviorComp = registry.CreateComponent<BehaviorComponent>();
			behaviorComp->SetName("Nice Behavior");
			entity2->AddComponent(behaviorComp);
			behaviorComp = registry.CreateComponent<BehaviorComponent>();
			behaviorComp->SetName("Nice Behavior 2");
			entity2->AddComponent(behaviorComp);
		}

		//const WorldTree& worldTree = m_EditorMainWorld->GetWorldTree();
		//worldTree.LogTree();

		if (0)
		{
			for (int32 i = 0; i < g_nHarnasSqrt * g_nHarnasSqrt; ++i)
			{
				MeshComponent* mesh = registry.CreateComponent<MeshComponent>();
				GetModelDeferred(g_ExampleModels[1], [mesh](ExampleModelData& model)
				{
					mesh->SetMesh(model.Mesh);
				});
				Entity* ent = m_EditorMainWorld->SpawnEntityOfClass<Entity>();
				ent->SetRootComponent(mesh);
				float x = ((i % g_nHarnasSqrt)) * 0.2f - g_nHarnasSqrt * 0.1f;
				float z = ((i / g_nHarnasSqrt)) * 0.2f - g_nHarnasSqrt * 0.1f;
				ent->SetLocation(Vector3(x, -4.0f, z));
				ent->SetRotation(Rotator(Vector3(-90.0f, 0.0f, 0.0f)));
				ent->SetName(String("Harnas_") + ToString(i));
				g_HarnasArray.push_back(ent);
			}
		}

		if (0)
		{
			using StringTree = TTree<String>;
			StringTree tree;
			StringTree::NodeRef root   = tree.GetRootNode();
			StringTree::NodeRef node0  = tree.Insert("Node0");
			StringTree::NodeRef node1  = tree.Insert("Node1");
			StringTree::NodeRef node2  = tree.Insert("Node2", root);
			StringTree::NodeRef node00 = tree.Insert("Node00", node0);
			StringTree::NodeRef node01 = tree.Insert("Node01", node0);
			StringTree::NodeRef node10 = tree.Insert("Node10", node1);
			//node1.Element() = nullptr;
			tree.LogTree();
			tree.Remove(node0, true);
			tree.LogTree();
			tree.Remove(node00, true);
			tree.LogTree();
			tree.Remove(node1);
			tree.LogTree();
			StringTree::NodeRef node3  = tree.Insert("Node3");
			StringTree::NodeRef node30 = tree.Insert("Node30", node3);
			StringTree::NodeRef node31 = tree.Insert("Node31", node3);
			StringTree::NodeRef node32 = tree.Insert(node3);
			tree.LogTree();
			String& n3Str = *node30.Element();
			LOG_INFO(n3Str);
			LOG_INFO(BoolStr(node32.IsElementNode()));
			LOG_INFO(BoolStr(node31.IsElementNode()));
		}

		if (1)
		{
			TTreeNodeFactory<String> nodeFactory;
			TTreeNode<String>& root = nodeFactory.Create("Root");
			TTreeNode<String>& child0 = root.Insert(nodeFactory.Create("Child0"));
			TTreeNode<String>& child1 = root.Insert(nodeFactory.Create("Child1"));
			TTreeNode<String>& child2 = root.Insert(nodeFactory.Create("Child2"));
			TTreeNode<String>& child3 = root.Insert(nodeFactory.Create("Child3"));
			TTreeNode<String>& child4 = root.Insert(nodeFactory.Create("Child4"));
			TTreeNode<String>& child5 = root.Insert(nodeFactory.Create("Child5"), 2);
			nodeFactory.Destroy(root.Remove(child3));
			child0.Insert(root.Remove(child5));
			root.SwapNodes(0, 2);
			root.SortNodes(std::less<String>());
		}
		
		if (0)
		{
			TestPoolAllocator();
		}
	}

	void EditorApplication::OnUpdate(float deltaTime)
	{
		TRACE_FUNCTION();

		UpdateEditorCamera(deltaTime);

		static float c_Time = 0.0f;
		c_Time += deltaTime;

		int32 nHarnas = 0;
		for (Entity* harnas : g_HarnasArray)
		{
			int32 x = nHarnas % g_nHarnasSqrt;
			int32 y = nHarnas / g_nHarnasSqrt;

			float wave = 0.0f;
			wave += sinf(((float)x + c_Time * 10.0f) / g_nHarnasSqrt * (float)Math::TWO_PI * 2.0f);
			wave += sinf(((float)y + c_Time * 10.0f) / g_nHarnasSqrt * (float)Math::TWO_PI * 2.0f);

			Vector3 location = harnas->GetLocation();
			location.y = wave;
			harnas->SetLocation(location);
			nHarnas++;
		}

		//m_Scene->UpdateRenderData();
	}

	void EditorApplication::OnRender()
	{
		TRACE_FUNCTION();
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
		if (GetSelectedComponent() && entity != GetSelectedComponent()->GetOwner())
		{
			SetSelectedComponent(nullptr);
		}
	}

	void EditorApplication::SetSelectedComponent(Component* component)
	{
		m_SelectedComponent = component;
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

	void EditorApplication::OnKeyPressedEvent(const KeyPressedEvent& event)
	{
		switch (event.GetActualKeyCode())
		{
			case Key::Escape:
			{
				SetSelectedEntity(nullptr);
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

	EditorApplication* EditorApplication::s_Instance;
}
}
