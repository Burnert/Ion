#include "EditorPCH.h"

#include "EditorLayer.h"
#include "EditorApplication.h"
#include "Editor/EditorAssets.h"
#include "Editor/ContentBrowser/ContentBrowser.h"
#include "Editor/LogSettings.h"

#include "Engine/World.h"
#include "Engine/Entity/EntityOld.h"
#include "Engine/Components/SceneComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/Entity/MeshEntity.h"

#include "Renderer/Renderer.h"

#include "UserInterface/ImGui.h"

#include "Resource/ResourceManager.h"

#include "ExampleModels.h"

namespace Ion::Editor
{
	EditorLayer::EditorLayer(const char* name) :
		Layer(name),
		m_EventDispatcher(this),
		m_bMainViewportOpenPtr(nullptr),
		m_bInsertPanelOpen(true),
		m_bResourcesPanelOpen(false),
		m_bWorldTreePanelOpen(true),
		m_bDetailsPanelOpen(true),
		m_bLoggingPanelOpen(true),
		m_bDiagnosticsPanelOpen(false),
		m_bImGuiMetricsOpen(false),
		m_bImGuiDemoOpen(false),
		m_DraggedWorldTreeNodeInfo(0),
		m_HoveredWorldTreeNodeDragTarget(nullptr)
	{
		m_EventDispatcher.RegisterEventFunction(&EditorLayer::OnMouseButtonPressedEvent);
		m_EventDispatcher.RegisterEventFunction(&EditorLayer::OnMouseButtonReleasedEvent);
		m_EventDispatcher.RegisterEventFunction(&EditorLayer::OnMouseDoubleClickEvent);
		m_EventDispatcher.RegisterEventFunction(&EditorLayer::OnRawInputMouseMovedEvent);
		m_EventDispatcher.RegisterEventFunction(&EditorLayer::OnRawInputMouseScrolledEvent);
		m_EventDispatcher.RegisterEventFunction(&EditorLayer::OnKeyPressedEvent);
		m_EventDispatcher.RegisterEventFunction(&EditorLayer::OnKeyReleasedEvent);
	}

	void EditorLayer::OnAttach()
	{
	}

	void EditorLayer::OnDetach()
	{
	}

	void EditorLayer::OnUpdate(float DeltaTime)
	{
		TRACE_FUNCTION();

		DrawEditorUI();

		for (EntityOld* entity : m_EntitiesToDestroy)
		{
			EditorApplication::Get()->DeleteObject(entity);
		}
		m_EntitiesToDestroy.clear();
		for (ComponentOld* component : m_ComponentsToDestroy)
		{
			EditorApplication::Get()->DeleteObject(component);
		}
		m_ComponentsToDestroy.clear();

		for (EntityOld* entity : m_EntitiesToDuplicate)
		{
			EditorApplication::Get()->DuplicateObject(entity);
		}
		m_EntitiesToDuplicate.clear();
	}

	void EditorLayer::OnRender()
	{
		TRACE_FUNCTION();
	}

	void EditorLayer::OnEvent(const Event& event)
	{
		m_EventDispatcher.Dispatch(event);
	}

	void EditorLayer::DrawEditorUI()
	{
		TRACE_FUNCTION();

		// Dockspace

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

		// The rest

		DrawMainMenuBar();

		DrawViewportWindows();
		EditorApplication::Get()->m_ContentBrowser->DrawUI();
		EditorApplication::Get()->m_LogSettings->DrawUI();
		
		DrawInsertPanel();
		DrawResourcesPanel();
		DrawWorldTreePanel();
		DrawDetailsPanel();
		DrawDiagnosticsPanel();

		if (m_bImGuiMetricsOpen)
			ImGui::ShowMetricsWindow(&m_bImGuiMetricsOpen);
		if (m_bImGuiDemoOpen)
			ImGui::ShowDemoWindow(&m_bImGuiDemoOpen);
	}

	void EditorLayer::DrawMainMenuBar()
	{
		TRACE_FUNCTION();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("New");
				ImGui::MenuItem("Open");
				ImGui::Separator();
				if (ImGui::MenuItem("Exit"))
				{
					EditorApplication::ExitEditor();
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				bool bSelected = EditorApplication::Get()->IsAnyObjectSelected();

				ImGui::MenuItem("Undo", nullptr, false, false);
				ImGui::MenuItem("Redo", nullptr, false, false);
				ImGui::Separator();
				ImGui::MenuItem("Cut", nullptr, false, false);
				ImGui::MenuItem("Copy", nullptr, false, false);
				ImGui::MenuItem("Paste", nullptr, false, false);
				if (ImGui::MenuItem("Delete", nullptr, false, bSelected))
				{
					EditorApplication::Get()->DeleteSelectedObject();
				}
				ImGui::SameLine(100); ImGui::TextDisabled("Del");
				ImGui::Separator();
				if (ImGui::MenuItem("Deselect", nullptr, false, bSelected))
				{
					EditorApplication::Get()->DeselectCurrentObject();
				}
				ImGui::SameLine(100); ImGui::TextDisabled("Esc");

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Main Viewport", nullptr, m_bMainViewportOpenPtr);
				ImGui::MenuItem("Insert", nullptr, &m_bInsertPanelOpen);
				ImGui::MenuItem("Content Browser", nullptr, &EditorApplication::Get()->m_ContentBrowser->GetUIWindowOpenFlag());
				ImGui::MenuItem("Resources", nullptr, &m_bResourcesPanelOpen);
				ImGui::MenuItem("World Tree", nullptr, &m_bWorldTreePanelOpen);
				ImGui::MenuItem("Details", nullptr, &m_bDetailsPanelOpen);
				ImGui::MenuItem("Logging", nullptr, &EditorApplication::Get()->m_LogSettings->GetUI().bWindowOpen);

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Misc"))
			{
				ImGui::MenuItem("Diagnostics", nullptr, &m_bDiagnosticsPanelOpen);

				ImGui::Separator();

				static bool c_bVSync = Renderer::Get()->IsVSyncEnabled();
				if (ImGui::MenuItem("Enable VSync", nullptr, &c_bVSync))
				{
					Renderer::Get()->SetVSyncEnabled(c_bVSync);
				}

				ImGui::Separator();

				ImGui::MenuItem("ImGui Metrics", nullptr, &m_bImGuiMetricsOpen);
				ImGui::MenuItem("ImGui Demo", nullptr, &m_bImGuiDemoOpen);

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void EditorLayer::DrawViewportWindow()
	{

	}

	void EditorLayer::DrawViewportWindows()
	{
		EditorApplication::Get()->DrawViewports();
	}

	void EditorLayer::DrawInsertPanel()
	{
		if (m_bInsertPanelOpen)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
			if (ImGui::Begin("Insert", &m_bInsertPanelOpen))
			{
				if (ImGui::BeginTabBar("InsertType"))
				{
					DrawInsertPanelEntityTab();
					DrawInsertPanelComponentTab();
					ImGui::EndTabBar();
				}
			}
			ImGui::End();
			ImGui::PopStyleVar();
		}
	}

	void EditorLayer::DrawInsertPanelEntityTab()
	{
		if (ImGui::BeginTabItem("Entity", nullptr))
		{
			ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysUseWindowPadding;
			if (ImGui::BeginChild("InsertEntityFrame", ImVec2(0, 0), false, flags))
			{
				DrawInsertPanelElement<ESceneObjectType::Entity>("Empty Entity", [](World* context, void*) -> EntityOld*
				{
					return context->SpawnEntityOfClass<EntityOld>().Raw();
				});
				DrawInsertPanelElement<ESceneObjectType::Entity>("Mesh Entity", [](World* context, void*) -> EntityOld*
				{
					return context->SpawnEntityOfClass<MeshEntity>().Raw();
				});
			}
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
	}

	void EditorLayer::DrawInsertPanelComponentTab()
	{
		if (ImGui::BeginTabItem("Component", nullptr))
		{
			ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysUseWindowPadding;
			if (ImGui::BeginChild("InsertComponentFrame", ImVec2(0, 0), false, flags))
			{
				for (auto& [id, info] : ComponentRegistry::GetRegisteredTypes())
				{
					DrawInsertPanelElement<ESceneObjectType::Component>(info.ClassDisplayName,
					// This will be called on (drag) drop
					[](World* context, ComponentTypeID id) -> ComponentOld*
					{
						return context->GetComponentRegistry().CreateComponent(id);
					}, &info);
				}
			}
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
	}

	void EditorLayer::DrawResourcesPanel()
	{
		if (m_bResourcesPanelOpen)
		{
			ImGui::Begin("Resources", &m_bResourcesPanelOpen);

			ImGui::Text("Mesh Resources");
			ImGui::Indent();
			{
				TArray<TSharedPtr<MeshResource>> meshes = ResourceManager::GetResourcesOfType<MeshResource>();
				for (TSharedPtr<MeshResource>& mesh : meshes)
				{
					String assetPath = mesh->GetAssetHandle()->GetImportPath().ToString();
					ImGui::Selectable(assetPath.c_str());
				}
			}
			ImGui::Unindent();

			ImGui::Text("Texture Resources");
			ImGui::Indent();
			{
				TArray<TSharedPtr<TextureResource>> textures = ResourceManager::GetResourcesOfType<TextureResource>();
				for (TSharedPtr<TextureResource>& texture : textures)
				{
					String assetPath = texture->GetAssetHandle()->GetImportPath().ToString();
					ImGui::Selectable(assetPath.c_str());
				}
			}
			ImGui::Unindent();

			ImGui::End();
		}
	}

	void EditorLayer::DrawWorldTreePanel()
	{
		TRACE_FUNCTION();

		// Set dragging state (bit 0) if bit 1 is true
		m_DraggedWorldTreeNodeInfo = (m_DraggedWorldTreeNodeInfo & Bitflag(1)) ? Bitflag(0) : 0;
		// Reset the target if not dragging
		if (!(m_DraggedWorldTreeNodeInfo & Bitflag(0)))
		{
			m_HoveredWorldTreeNodeDragTarget = nullptr;
		}
		// If the bit 0 doesn't get set, the hovered node will be reset.
		m_HoveredWorldTreeNodeDragTarget.SetMetaFlag<0>(false);

		if (m_bWorldTreePanelOpen)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
			if (ImGui::Begin("World Tree", &m_bWorldTreePanelOpen))
			{
				World* editorWorld = EditorApplication::Get()->GetEditorWorld();
				if (editorWorld)
				{
					ImGui::BeginChild("WorldTree");
					{
						DrawWorldTreeNodes();
					}
					ImGui::EndChild();
					// Deselect entity when free space is clicked
					if (ImGui::IsItemClicked())
					{
						EditorApplication::Get()->DeselectCurrentEntity();
					}
				}
			}
			ImGui::End();
			ImGui::PopStyleVar();
		}
		// Bit 0 hasn't been set, reset the hovered node
		if (!m_HoveredWorldTreeNodeDragTarget.GetMetaFlag<0>())
		{
			m_HoveredWorldTreeNodeDragTarget = nullptr;
		}
	}

	void EditorLayer::DrawWorldTreeNodes()
	{
		TRACE_FUNCTION();

		WorldTreeNode* firstExpandNode = PopWorldTreeExpandChain();
		DrawWorldTreeNodeChildren(EditorApplication::Get()->GetEditorWorld()->GetWorldTreeRoot(), firstExpandNode);
	}

	void EditorLayer::DrawWorldTreeNodeChildren(const WorldTreeNode& node, WorldTreeNode* nextExpandNode)
	{
		TRACE_FUNCTION();

		int64 imguiNodeIndex = 0;
		for (const WorldTreeNode* child : node.GetChildren())
		{
			ionassert(child);
			DrawWorldTreeNode(*child, nextExpandNode);
		}
	}

	void EditorLayer::DrawWorldTreeNode(const WorldTreeNode& node, WorldTreeNode* nextExpandNode)
	{
		const WorldTreeNodeData& nodeData = node.Get();

		bool bHasChildren = node.HasChildren();
		String nodeName;
		EntityOld* entity = nullptr;

		ImGuiTreeNodeFlags imguiNodeFlags =
			ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_FramePadding;
		// It shouldn't expand if there are no children
		if (!bHasChildren)
		{
			imguiNodeFlags |=
				ImGuiTreeNodeFlags_Leaf |
				ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		entity = nodeData.GetEntity();
		nodeName = entity->GetName();

		// Highlight the selected entity
		if (entity == EditorApplication::Get()->GetSelectedEntity())
		{
			imguiNodeFlags |= ImGuiTreeNodeFlags_Selected;
		}

		// Expand (open) the tree node if it is in the expand chain
		if (nextExpandNode == &node)
		{
			ImGui::SetNextItemOpen(true);
			// And get the next node to expand
			nextExpandNode = PopWorldTreeExpandChain();
		}

		const void* nodeId = &node;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
		bool bImguiTreeNodeOpen = ImGui::TreeNodeEx(nodeId, imguiNodeFlags, "%s", nodeName.c_str());
		ImGui::PopStyleVar(2);
		// Select entity on click, only if the node has not toggled the open state
		// (unless it has no children, since the state won't change then).
		if (entity && ImGui::IsItemClicked() &&
			!ImGui::IsTreeNodeToggled())
		{
			EditorApplication::Get()->SelectObject(entity);
		}
		if (ImGui::BeginDragDropSource())
		{
			// Dragging state (bit 0) will be set on the next frame based on bit 1
			m_DraggedWorldTreeNodeInfo |= Bitflag(1);

			const WorldTreeNode* nodePtr = &node;
			ImGui::SetDragDropPayload(DNDID_WorldTreeNode, &nodePtr, sizeof(WorldTreeNode*), ImGuiCond_Once);

			if (m_HoveredWorldTreeNodeDragTarget)
			{
				// Show the action that is going to be performed
				const WorldTreeNodeData& hovered = m_HoveredWorldTreeNodeDragTarget->Get();
				bool bCanAttach = entity->CanAttachTo(hovered.GetEntity()->This());
				const char* action = bCanAttach ? "Attach" : "Cannot attach";
				ImGui::Text("%s %s to %s", action, node.Get().GetName().c_str(), hovered.GetName().c_str());
			}
			else
			{
				// Show the name of the currently dragged node
				ImGui::Text(node.Get().GetName().c_str());
			}

			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			m_HoveredWorldTreeNodeDragTarget = &node;
			m_HoveredWorldTreeNodeDragTarget.SetMetaFlag<0>(true);

			bool bCanAttachTo = false;
			EntityOld* sourceEntity = nullptr;

			ImGuiDragDropFlags dndFlags = ImGuiDragDropFlags_AcceptPeekOnly;
			// First, check if the drag and drop node can even be attached to the target.
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DNDID_WorldTreeNode, dndFlags))
			{
				ionassert(payload->DataSize == sizeof(WorldTreeNode*));
				WorldTreeNode* nodePtr = *(WorldTreeNode**)payload->Data;
				if (nodePtr->Get().GetEntity())
				{
					sourceEntity = nodePtr->Get().GetEntity();
					ionassert(sourceEntity);
					// If the target entity is in the source entity's children,
					// it cannot be attached to.
					bCanAttachTo = sourceEntity->CanAttachTo(entity->This());
				}
			}
			if (bCanAttachTo)
			{
				// Accept again so the outline rect gets drawn
				if (ImGui::AcceptDragDropPayload(DNDID_WorldTreeNode, ImGuiDragDropFlags_None))
				{
					// Attach source entity to this node's entity
					sourceEntity->AttachTo(entity->This());
					ExpandWorldTreeToEntity(sourceEntity);
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (ImGui::BeginPopupContextItem())
		{
			// Duplicate
			if (ImGui::MenuItem("Duplicate"))
			{
				if (entity)
				{
					m_EntitiesToDuplicate.push_back(entity);
				}
			}
			// Delete
			if (ImGui::MenuItem("Delete"))
			{
				if (entity)
				{
					m_EntitiesToDestroy.push_back(entity);
				}
			}
			// Detach
			bool bCanDetach = entity ? entity->HasParent() : false;
			if (ImGui::MenuItem("Detach", nullptr, false, bCanDetach))
			{
				entity->Detach();
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Parent to the world root.");
			}
			ImGui::EndPopup();
		}

		if (bHasChildren && bImguiTreeNodeOpen)
		{
			DrawWorldTreeNodeChildren(node, nextExpandNode);
			ImGui::TreePop();
		}
	}

	void EditorLayer::DrawDetailsPanel()
	{
		TRACE_FUNCTION();

		if (m_bDetailsPanelOpen)
		{
			if (ImGui::Begin("Details", &m_bDetailsPanelOpen))
			{
				EntityOld* selectedEntity = EditorApplication::Get()->GetSelectedEntity();
				if (selectedEntity)
				{
					ComponentOld* selectedComponent = EditorApplication::Get()->GetSelectedComponent();

					const String& selectedObjectName = selectedComponent ?
						selectedComponent->GetName() :
						selectedEntity->GetName();

					const String& selectedObjectClass = selectedComponent ?
						selectedComponent->GetClassName() :
						"Entity";

					ImGui::PushID("Details");

					DrawDetailsNameSection(*selectedEntity);
					DrawDetailsRelationsSection(*selectedEntity);
					ImGui::Separator();
					DrawDetailsComponentTreeSection(*selectedEntity);
					ImGui::Separator();
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 0.9f), "%s", selectedObjectName.c_str());
					ImGui::SameLine(); ImGui::TextDisabled("(%s)", selectedObjectClass.c_str());
					ImGui::Separator();
					if (selectedComponent)
					{
						DrawDetailsComponentSection(*selectedComponent);
					}
					else
					{
						DrawDetailsEntitySection(*selectedEntity);
					}

					ImGui::PopID();
				}
				else
				{
					ImGui::Text("No entity is currently selected.");
				}
			}
			ImGui::End();
		}
	}

	void EditorLayer::DrawDetailsNameSection(EntityOld& entity)
	{
		TRACE_FUNCTION();

		ImGui::PushID("Name");

		ImGuiInputTextFlags imguiInputTextFlags =
			ImGuiInputTextFlags_EnterReturnsTrue;

		char name[128] = { };
		strcpy_s(name, entity.GetName().c_str());
		if (ImGui::InputText("Entity Name", name, sizeof(name), imguiInputTextFlags))
		{
			entity.SetName(name);
		}

		ImGui::PopID();
	}

	void EditorLayer::DrawDetailsRelationsSection(EntityOld& entity)
	{
		TRACE_FUNCTION();

		ImGui::PushID("Relations");

		if (ImGui::CollapsingHeaderUnframed("Relations"))
		{
			// Parent

			EntityOld* parent = entity.GetParent().Raw();
			const String& parentNameStr = parent ? parent->GetName() : "None";

			ImGui::Indent();
			ImGui::Text("Parent");
			ImGui::SameLine(120);
			if (parent)
			{
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "%s", parentNameStr.c_str());
				ImGui::SameLine();
				if (ImGui::SmallButton("Select"))
				{
					EditorApplication::Get()->SelectObject(parent);
					ExpandWorldTreeToEntity(parent);
				}
			}
			else
			{
				ImGui::TextDisabled("%s", parentNameStr.c_str());
			}
			ImGui::Unindent();

			// Children

			if (entity.HasChildren())
			{
				ImGuiTreeNodeFlags flags =
					ImGuiTreeNodeFlags_NoTreePushOnOpen |
					ImGuiTreeNodeFlags_DefaultOpen;
				bool bOpen = ImGui::TreeNodeEx("Children", flags);
				ImGui::SameLine(120);
				ImGui::Text("(%llu)", entity.GetChildren().size());
				if (bOpen)
				{
					DrawDetailsRelationsChildrenSection(entity);
				}
			}
			else
			{
				ImGui::Indent();
				ImGui::Text("Children");
				ImGui::SameLine(120);
				ImGui::TextDisabled("None");
				ImGui::Unindent();
			}
		}

		ImGui::PopID();
	}

	void EditorLayer::DrawDetailsRelationsChildrenSection(EntityOld& entity)
	{
		TRACE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 2.0f));
		if (ImGui::BeginChild("children_list", ImVec2(-FLT_MIN, 4 * ImGui::GetTextLineHeightWithSpacing()), true))
		{
			ImGuiStyle& style = ImGui::GetStyle();
			const TArray<TObjectPtr<EntityOld>>& children = entity.GetChildren();
			int32 index = 0;
			for (const TObjectPtr<EntityOld>& child : children)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

				ImGui::AlignTextToFramePadding();
				ImGui::Text("%s", child->GetName().c_str());
				ImGui::SameLine();

				ImVec2 selectSize = ImGui::CalcTextSize("Select");
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - selectSize.x - style.FramePadding.x);
				String selectLabel = "Select"s + "##" + ToString(index++);
				if (ImGui::Button(selectLabel.c_str()))
				{
					EditorApplication::Get()->SelectObject(child.Raw());
					ExpandWorldTreeToEntity(child.Raw());
				}
				ImGui::PopStyleVar(2);
			}
			ImGui::EndChild();
		}
		ImGui::PopStyleVar();
	}

	void EditorLayer::DrawDetailsComponentTreeSection(EntityOld& entity)
	{
		TRACE_FUNCTION();

		if (ImGui::CollapsingHeader("Component Tree", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("ComponentTree", ImVec2(0, 100), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar);
			DrawComponentTreeContent(entity);
			ImGui::EndChild();
			// Deselect component when free space is clicked
			if (ImGui::IsItemClicked())
			{
				EditorApplication::Get()->DeselectCurrentComponent();
			}
			if (ImGui::BeginDragDropTarget())
			{
				ImGuiDragDropFlags dndFlags = ImGuiDragDropFlags_None;
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DNDID_InsertComponent, dndFlags))
				{
					ionassert(payload->DataSize == sizeof(DNDInsertComponentData));

					DNDInsertComponentData& data = *(DNDInsertComponentData*)payload->Data;
					ComponentOld* component = data.Instantiate(EditorApplication::Get()->GetEditorWorld(), data.ID);
					entity.AddComponent(component);
				}
				ImGui::EndDragDropTarget();
			}
		}
	}

	void EditorLayer::DrawDetailsEntitySection(EntityOld& entity)
	{
		DrawDetailsTransformSection(entity);
		DrawDetailsRenderingSection(entity);
		if (MeshEntity* meshEntity = dynamic_cast<MeshEntity*>(&entity))
		{
			DrawMeshSection(*meshEntity->GetMeshComponent());
		}
	}

	void EditorLayer::DrawSceneComponentDetails(SceneComponent& component)
	{
		TRACE_FUNCTION();

		ImGui::PushID("SceneComponentDetails");

		DrawSceneComponentDetailsTransformSection(component);
		DrawSceneComponentDetailsRenderingSection(component);
		if (component.GetClassName() == "MeshComponent")
		{
			DrawMeshSection((MeshComponent&)component);
		}

		ImGui::PopID();
	}

	void EditorLayer::DrawSceneComponentDetailsTransformSection(SceneComponent& component)
	{
		TRACE_FUNCTION();

		Transform transform = component.GetTransform();
		if (DrawTransformSection(transform))
		{
			component.SetTransform(transform);
		}
	}

	void EditorLayer::DrawSceneComponentDetailsRenderingSection(SceneComponent& component)
	{
		TRACE_FUNCTION();

		bool bVisible = component.IsVisible();
		bool bVisibleInGame = component.IsVisibleInGame();
		if (DrawRenderingSection(bVisible, bVisibleInGame))
		{
			component.SetVisible(bVisible);
			component.SetVisibleInGame(bVisibleInGame);
		}
	}

	void EditorLayer::DrawComponentDetails(ComponentOld& component)
	{
		TRACE_FUNCTION();

		if (ImGui::CollapsingHeader(component.GetClassName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("ComponentDetails");

			const ComponentDatabase::TypeInfo& typeInfo = component.GetFinalTypeInfo();
			for (INCProperty* prop : typeInfo.EditableProperties)
			{
				DrawComponentDetailsProperty(component, prop);
			}

			ImGui::PopID();
		}
	}

	template<typename T>
	static void DrawPropertyScalar(ComponentOld& component, INCProperty* prop, ImGuiDataType type, int32 nDims)
	{
		TINCPropertyTyped<T>* tProp = (TINCPropertyTyped<T>*)prop;
		const TNCPropertyOptionalParams<T>& params = tProp->GetParams();

		const T* minValue = params.bUseMinValue ?
			&params.MinValue : nullptr;
		const T* maxValue = params.bUseMaxValue ?
			&params.MaxValue : nullptr;

		T value = tProp->Get(component);
		if (ImGui::DragScalarN(prop->GetDisplayName().c_str(), type, &value, nDims, 0.0125f,
			minValue, maxValue, "%.3f", ImGuiSliderFlags_None))
			tProp->Set(component, value);
		if constexpr (TIsSameV<T, Vector3>)
		{
			ImGui::Text("Default: {%f, %f, %f}", params.DefaultValue.x, params.DefaultValue.y, params.DefaultValue.z);
		}
	}

	void EditorLayer::DrawComponentDetailsProperty(ComponentOld& component, INCProperty* prop)
	{
		ionassert(prop);

		const String& name = prop->GetDisplayName();
		ENCPropertyType type = prop->GetType();

		switch (type)
		{
			case ENCPropertyType::Bool:
			{
				TINCPropertyTyped<bool>* tProp = (TINCPropertyTyped<bool>*)prop;
				bool value = tProp->Get(component);
				if (ImGui::Checkbox(prop->GetDisplayName().c_str(), &value))
					tProp->Set(component, value);
				break;
			}
			case ENCPropertyType::Int32:
			{
				DrawPropertyScalar<int32>(component, prop, ImGuiDataType_S32, 1);
				break;
			}
			case ENCPropertyType::Int64:
			{
				DrawPropertyScalar<int64>(component, prop, ImGuiDataType_S64, 1);
				break;
			}
			case ENCPropertyType::Float:
			{
				DrawPropertyScalar<float>(component, prop, ImGuiDataType_Float, 1);
				break;
			}
			case ENCPropertyType::Vector2:
			{
				DrawPropertyScalar<Vector2>(component, prop, ImGuiDataType_Float, 2);
				break;
			}
			case ENCPropertyType::Vector3:
			{
				DrawPropertyScalar<Vector3>(component, prop, ImGuiDataType_Float, 3);
				break;
			}
			case ENCPropertyType::Vector4:
			{
				DrawPropertyScalar<Vector4>(component, prop, ImGuiDataType_Float, 4);
				break;
			}
		}
	}

	void EditorLayer::DrawDetailsTransformSection(EntityOld& entity)
	{
		TRACE_FUNCTION();

		Transform transform = entity.GetTransform();
		if (DrawTransformSection(transform))
		{
			entity.SetTransform(transform);
		}
	}

	void EditorLayer::DrawDetailsRenderingSection(EntityOld& entity)
	{
		TRACE_FUNCTION();

		bool bVisible = entity.IsVisible();
		bool bVisibleInGame = entity.IsVisibleInGame();
		if (DrawRenderingSection(bVisible, bVisibleInGame))
		{
			entity.SetVisible(bVisible);
			entity.SetVisibleInGame(bVisibleInGame);
		}
	}

	void EditorLayer::DrawDetailsComponentSection(ComponentOld& component)
	{
		if (component.IsSceneComponent())
		{
			DrawSceneComponentDetails((SceneComponent&)component);
		}
		DrawComponentDetails(component);
	}

	bool EditorLayer::DrawTransformSection(Transform& inOutTransform)
	{
		TRACE_FUNCTION();

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Transform");

			Vector3 location = inOutTransform.GetLocation();
			Vector3 rotationAngles = inOutTransform.GetRotation().Angles();
			Vector3 scale = inOutTransform.GetScale();

			// @TODO: Write warning: Modyfing non-uniformly scaled object's 
			// transform in world space may have unpredictable results.

			bool bChanged = false;
			bChanged |= ImGui::DragFloat3("Location", (float*)&location, 0.0125f, 0.0f, 0.0f, "%.5f");
			bChanged |= ImGui::DragFloat3("Rotation", (float*)&rotationAngles, 0.5f, 0.0f, 0.0f, "%.5f");
			bChanged |= ImGui::DragFloat3("Scale", (float*)&scale, 0.0125f, 0.0f, 0.0f, "%.5f");

			if (bChanged)
			{
				inOutTransform = { location, Rotator(rotationAngles), scale };
			}

			ImGui::PopID();

			return bChanged;
		}
		return false;
	}

	bool EditorLayer::DrawRenderingSection(bool& bInOutVisible, bool& bInOutVisibleInGame)
	{
		TRACE_FUNCTION();

		if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Rendering");

			bool bChanged = false;

			bChanged |= ImGui::Checkbox("Visible", &bInOutVisible);
			bChanged |= ImGui::Checkbox("Visible in game", &bInOutVisibleInGame);

			ImGui::PopID();

			return bChanged;
		}
		return false;
	}

	bool EditorLayer::DrawMeshSection(MeshComponent& meshComponent)
	{
		TRACE_FUNCTION();

		bool bChanged = false;
		if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("MeshSettings");

			std::shared_ptr<Mesh> mesh = meshComponent.GetMesh();

			Asset currentMeshAsset = mesh && mesh->GetMeshResource() ?
				mesh->GetMeshResource()->GetAssetHandle() :
				Asset::None;

			String previewName = currentMeshAsset ?
				currentMeshAsset->GetInfo().Name :
				"[None]";

			ImGuiComboFlags flags = ImGuiComboFlags_HeightLargest;
			bool bComboOpen = ImGui::BeginCombo("Mesh Asset", previewName.c_str(), flags);
			if (ImGui::BeginDragDropTarget())
			{
				ImGuiDragDropFlags dndFlags = ImGuiDragDropFlags_None;
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DNDID_MeshAsset, dndFlags))
				{
					ionassert(payload->DataSize == sizeof(DNDAssetData));

					DNDAssetData& data = *(DNDAssetData*)payload->Data;

					TSharedPtr<MeshResource> meshResource = MeshResource::Query(data.AssetHandle);

					std::shared_ptr<Mesh> mesh = Mesh::CreateFromResource(meshResource);

					// @TODO: Yeah well this is a bit too much
					meshComponent.SetMeshAsset(data.AssetHandle);
					meshComponent.SetMeshResource(meshResource);
					meshComponent.SetMesh(mesh);

					bChanged = true;
				}

				ImGui::EndDragDropTarget();
			}
			if (bComboOpen)
			{
				if (ImGui::Selectable("[None]", !currentMeshAsset))
				{
					meshComponent.SetMeshAsset(Asset::None);
					meshComponent.SetMeshResource(nullptr);
					meshComponent.SetMesh(nullptr);

					bChanged = true;
				}

				TArray<Asset> meshAssets = AssetRegistry::GetAllRegisteredAssets(AT_MeshAssetType);
				for (Asset& meshAsset : meshAssets)
				{
					bool bSelected = meshAsset == currentMeshAsset;
					if (ImGui::Selectable(meshAsset->GetInfo().Name.c_str(), bSelected))
					{
						TSharedPtr<MeshResource> meshResource = MeshResource::Query(meshAsset);

						std::shared_ptr<Mesh> mesh = Mesh::CreateFromResource(meshResource);

						// @TODO: Yeah well this is a bit too much
						meshComponent.SetMeshAsset(meshAsset);
						meshComponent.SetMeshResource(meshResource);
						meshComponent.SetMesh(mesh);

						bChanged = true;
					}
				}

				ImGui::EndCombo();
			}

			if (meshComponent.GetMesh())
			{
				Asset currentMaterialAsset = meshComponent.GetMesh()->GetMaterialInSlot(0) ?
					meshComponent.GetMesh()->GetMaterialInSlot(0)->GetAsset() :
					Asset::None;

				previewName = currentMaterialAsset ?
					currentMaterialAsset->GetInfo().Name :
					"[None]";

				bComboOpen = ImGui::BeginCombo("Material Asset", previewName.c_str(), flags);

				if (bComboOpen)
				{
					if (ImGui::Selectable("[None]", !currentMaterialAsset))
					{
						meshComponent.GetMesh()->AssignMaterialToSlot(0, nullptr);

						bChanged = true;
					}

					TArray<Asset> materialAssets = AssetRegistry::GetAllRegisteredAssets(AT_MaterialInstanceAssetType);
					for (Asset& materialAsset : materialAssets)
					{
						bool bSelected = materialAsset == currentMaterialAsset;
						if (ImGui::Selectable(materialAsset->GetInfo().Name.c_str(), bSelected))
						{
							std::shared_ptr<MaterialInstance> materialInstance = MaterialRegistry::QueryMaterialInstance(materialAsset);
							meshComponent.GetMesh()->AssignMaterialToSlot(0, materialInstance);

							bChanged = true;
						}
					}
					ImGui::EndCombo();
				}
			}

			ImGui::PopID();
		}
		return bChanged;
	}

	void EditorLayer::DrawDiagnosticsPanel()
	{
		TRACE_FUNCTION();

		if (m_bDiagnosticsPanelOpen)
		{
			ImGui::Begin("Diagnostics", &m_bDiagnosticsPanelOpen);

			if (ImGui::Button("Start Recording"))
			{
				TRACE_RECORD_START();
			}
			if (ImGui::Button("Stop Recording"))
			{
				TRACE_RECORD_STOP();
			}

			ImGui::End();
		}
	}

	void EditorLayer::DrawComponentTreeContent(EntityOld& entity)
	{
		TRACE_FUNCTION();

		SceneComponent* rootComponent = entity.GetRootComponent();
		ionassert(rootComponent);

		bool bRootOpen = DrawComponentTreeSceneComponentNode(*rootComponent);
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 0.9f), "Root");
		if (bRootOpen)
		{
			ImGui::Indent();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			DrawComponentTreeNodeChildren(*rootComponent, 1);
			ImGui::Unindent();
		}
		if (!entity.GetComponents().empty())
		{
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			DrawComponentTreeNonSceneComponents(entity);
		}
	}

	void EditorLayer::DrawComponentTreeNodeChildren(SceneComponent& component, int64 startId)
	{
		TRACE_FUNCTION();

		for (SceneComponent* child : component.GetChildren())
		{
			DrawComponentTreeSceneComponentNode(*child, startId++, true);
		}
	}

	// True if open
	bool EditorLayer::DrawComponentTreeSceneComponentNode(SceneComponent& component, int64 id, bool bDrawChildren)
	{
		TRACE_FUNCTION();

		bool bHasChildren = component.HasChildren();

		ImGuiTreeNodeFlags imguiNodeFlags =
			ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_FramePadding;
		// It shouldn't expand if there are no children
		if (!bHasChildren)
		{
			imguiNodeFlags |=
				ImGuiTreeNodeFlags_Leaf |
				ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}
		// Don't tree push, because the nodes won't be drawn
		if (!bDrawChildren)
		{
			imguiNodeFlags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		String nodeName = component.GetName();

		// Highlight the selected component
		if ((ComponentOld*)&component == EditorApplication::Get()->GetSelectedComponent())
		{
			imguiNodeFlags |= ImGuiTreeNodeFlags_Selected;
		}

		void* nodeId = &component;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
		bool bImguiTreeNodeOpen = ImGui::TreeNodeEx(nodeId, imguiNodeFlags, "%s", nodeName.c_str());
		ImGui::PopStyleVar(2);
		// Select component on click, only if the state of the node has not changed
		// (unless it has no children, since the state won't change then).
		if (ImGui::IsItemClicked() && !ImGui::IsTreeNodeToggled())
		{
			EditorApplication::Get()->SelectObject(&component);
		}
		if (ImGui::BeginDragDropTarget())
		{
			ImGuiDragDropFlags dndFlags = ImGuiDragDropFlags_None;
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DNDID_InsertSceneComponent, dndFlags))
			{
				ionassert(payload->DataSize == sizeof(DNDInsertComponentData));

				DNDInsertComponentData& data = *(DNDInsertComponentData*)payload->Data;
				SceneComponent* comp = (SceneComponent*)data.Instantiate(EditorApplication::Get()->GetEditorWorld(), data.ID);
				comp->AttachTo(&component);
			}

			ImGui::EndDragDropTarget();
		}
		if (ImGui::BeginPopupContextItem())
		{
			EntityOld* owner = component.GetOwner();
			ionassert(owner);
			bool bCanDelete = component.GetOwner()->GetRootComponent() != &component;
			String deleteLabel = bCanDelete ? "Delete" : "Delete Entity";
			if (ImGui::MenuItem(deleteLabel.c_str()))
			{
				bCanDelete ?
					m_ComponentsToDestroy.push_back(&component) :
					m_EntitiesToDestroy.push_back(owner);
			}
			if (ImGui::IsItemHovered() && !bCanDelete)
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(180);
				ImGui::Text("Cannot delete the root component. Deletes the owning entity instead.");
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		ImGui::TextDisabled("(%s)", component.GetClassDisplayName().c_str());

		if (bDrawChildren && bHasChildren && bImguiTreeNodeOpen)
		{
			DrawComponentTreeNodeChildren(component);
			ImGui::TreePop();
		}

		return bHasChildren && bImguiTreeNodeOpen;
	}

	void EditorLayer::DrawComponentTreeNonSceneComponents(EntityOld& entity)
	{
		TRACE_FUNCTION();

		const EntityOld::ComponentSet& nonSceneComponents = entity.GetComponents();

		int64 uniqueIndex = 0;
		for (ComponentOld* component : nonSceneComponents)
		{
			ionassert(component);

			String nodeName = component->GetName() + "##" + ToString(uniqueIndex++);
			bool bSelected = EditorApplication::Get()->GetSelectedComponent() == component;
			if (ImGui::Selectable(nodeName.c_str(), bSelected))
			{
				EditorApplication::Get()->SelectObject(component);
			}
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Delete"))
				{
					m_ComponentsToDestroy.push_back(component);
				}
				ImGui::EndPopup();
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(%s)", component->GetClassDisplayName().c_str());
		}
	}

	void EditorLayer::ExpandWorldTreeToEntity(EntityOld* entity)
	{
		ionassert(entity);

		// Discard the previous attempt
		m_ExpandWorldTreeChain.clear();

		WorldTreeNode* node = EditorApplication::Get()->GetEditorWorld()->FindWorldTreeNode(entity->This());
		// Don't include the tree root
		while ((node = node->GetParent()) && node->HasParent())
		{
			m_ExpandWorldTreeChain.push_back(node);
		}
	}

	EditorLayer::WorldTreeNode* EditorLayer::PopWorldTreeExpandChain()
	{
		WorldTreeNode* node = !m_ExpandWorldTreeChain.empty() ? m_ExpandWorldTreeChain.back() : nullptr;
		if (node)
			m_ExpandWorldTreeChain.pop_back();
		return node;
	}

	void EditorLayer::SetMainViewportOpenFlagPtr(bool* flagPtr)
	{
		m_bMainViewportOpenPtr = flagPtr;
	}

	void EditorLayer::OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event)
	{
	}

	void EditorLayer::OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event)
	{
	}

	void EditorLayer::OnMouseDoubleClickEvent(const MouseDoubleClickEvent& event)
	{
	}

	void EditorLayer::OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event)
	{
	}

	void EditorLayer::OnRawInputMouseScrolledEvent(const RawInputMouseScrolledEvent& event)
	{
	}

	void EditorLayer::OnKeyPressedEvent(const KeyPressedEvent& event)
	{
	}

	void EditorLayer::OnKeyReleasedEvent(const KeyReleasedEvent& event)
	{
	}
}
