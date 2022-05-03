#include "EditorPCH.h"

#include "EditorLayer.h"
#include "EditorApplication.h"
#include "Editor/EditorAssets.h"

#include "Engine/World.h"
#include "Engine/Entity/Entity.h"
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
		m_bContentBrowserOpen(true),
		m_bResourcesPanelOpen(false),
		m_bWorldTreePanelOpen(true),
		m_bDetailsPanelOpen(true),
		m_bDiagnosticsPanelOpen(false),
		m_bImGuiMetricsOpen(false),
		m_bImGuiDemoOpen(false),
		m_DraggedWorldTreeNodeInfo(0),
		m_HoveredWorldTreeNodeDragTarget(nullptr),
		m_SelectedAsset()
	{
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

		for (Entity* entity : m_EntitiesToDestroy)
		{
			EditorApplication::Get()->DeleteObject(entity);
		}
		m_EntitiesToDestroy.clear();
		for (Component* component : m_ComponentsToDestroy)
		{
			EditorApplication::Get()->DeleteObject(component);
		}
		m_ComponentsToDestroy.clear();
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
		DrawInsertPanel();
		DrawContentBrowser();
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
				ImGui::MenuItem("Content Browser", nullptr, &m_bContentBrowserOpen);
				ImGui::MenuItem("Resources", nullptr, &m_bResourcesPanelOpen);
				ImGui::MenuItem("World Tree", nullptr, &m_bWorldTreePanelOpen);
				ImGui::MenuItem("Details", nullptr, &m_bDetailsPanelOpen);

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
				DrawInsertPanelElement<ESceneObjectType::Entity>("Empty Entity", [](World* context) -> Entity*
				{
					return context->SpawnEntityOfClass<Entity>();
				});
				DrawInsertPanelElement<ESceneObjectType::Entity>("Mesh Entity", [](World* context) -> Entity*
				{
					return context->SpawnEntityOfClass<MeshEntity>();
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
					[](World* context, ComponentTypeID id) -> Component*
					{
						return context->GetComponentRegistry().CreateComponent(id);
					}, &info);
				}
			}
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
	}

	void EditorLayer::DrawContentBrowser()
	{
		TRACE_FUNCTION();

		if (m_bContentBrowserOpen)
		{
			ImGui::Begin("Content Browser", &m_bContentBrowserOpen);

			ImGui::PushID("ContentBrowser");

			// @TODO: Make some kind of a directory tree here.

			ImGui::BeginChild("AssetTree", ImVec2(160, 0));
			{
				if (ImGui::Button("Refresh"))
				{
					UpdateRegisteredAssetsCache();
				}

				ImGui::Text("Asset Tree Lol");
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("Assets");
			{
				for (Asset& asset : m_RegisteredAssetsCache)
				{
					ImVec2 assetIconSize = { 96, 128 };
					float assetIconSeparation = 8;

					String sType = ToString(asset->GetType());
					String sName = asset->GetInfo().Name;

					ImVec2 avail = ImGui::GetContentRegionAvail();

					// Place the image in the same place the selectable is
					ImVec2 selectableCursor = ImGui::GetCursorPos();

					bool bSelected = m_SelectedAsset == asset;

					// Draw an empty selectable
					if (ImGui::Selectable(("##" + sType + sName).c_str(), bSelected, ImGuiSelectableFlags_None, assetIconSize))
					{
						m_SelectedAsset = asset;
					}

					// Where to place the next item
					ImVec2 nextCursor;
					if (avail.x - assetIconSize.x - assetIconSeparation > assetIconSize.x)
					{
						ImGui::SameLine();
						nextCursor = ImGui::GetCursorPos();
						nextCursor.x += assetIconSeparation;
					}
					else
					{
						nextCursor = ImGui::GetCursorPos();
						nextCursor.y += assetIconSeparation;
					}

					bool bHovered;
					if (bHovered = ImGui::IsItemHovered())
					{
						EditorApplication::Get()->SetCursor(ECursorType::Hand);
					}

					// Draw the asset icon

					ImGui::SetCursorPos(selectableCursor);

					void* iconTexture;
					switch (asset->GetType())
					{
					case EAssetType::Mesh:  iconTexture = EditorIcons::IconMeshAsset.Texture->GetNativeID();  break;
					case EAssetType::Image: iconTexture = EditorIcons::IconImageAsset.Texture->GetNativeID(); break;
					default:                iconTexture = EditorIcons::IconAsset.Texture->GetNativeID();      break;
					}

					ImGui::Image(iconTexture, ImVec2(assetIconSize.x, assetIconSize.x), ImVec2(0, 1), ImVec2(1, 0));

					// Draw resource usage icons

					if (bHovered || bSelected)
					{
						ImGui::SetCursorPos(selectableCursor);

						for (const String& resource : asset->GetInfo().ResourceUsage)
						{
							TShared<RHITexture> resourceIconTexture;

							if (resource == "Mesh")
							{
								resourceIconTexture = EditorIcons::IconMeshResource.Texture;
							}
							else if (resource == "Texture")
							{
								resourceIconTexture = EditorIcons::IconTextureResource.Texture;
							}

							ImGui::Image(resourceIconTexture->GetNativeID(), ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0));
							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								ImGui::Text("The asset can be used as a %s resource.", resource.c_str());
								ImGui::EndTooltip();
							}
							ImGui::SameLine();
						}
					}

					// Draw the label

					ImVec2 textSize = ImGui::CalcTextSize(sName.c_str());
					ImGui::SetCursorPos(ImVec2(selectableCursor.x + (assetIconSize.x - textSize.x) * 0.5f, selectableCursor.y + assetIconSize.x));
					ImGui::Text(sName.c_str());

					ImGui::SetCursorPos(nextCursor);
				}
			}
			ImGui::EndChild();
			// Deselect the asset on empty space click
			if (ImGui::IsItemClicked())
			{
				m_SelectedAsset = Asset();
			}

			ImGui::PopID();

			ImGui::End();
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
				TArray<TResourcePtr<MeshResource>> meshes = ResourceManager::GetResourcesOfType<MeshResource>();
				for (TResourcePtr<MeshResource>& mesh : meshes)
				{
					WString assetPath = mesh->GetAssetHandle()->GetPath().ToString();
					ImGui::Selectable(StringConverter::WStringToString(assetPath).c_str());
				}
			}
			ImGui::Unindent();

			ImGui::Text("Texture Resources");
			ImGui::Indent();
			{
				TArray<TResourcePtr<TextureResource>> textures = ResourceManager::GetResourcesOfType<TextureResource>();
				for (TResourcePtr<TextureResource>& texture : textures)
				{
					WString assetPath = texture->GetAssetHandle()->GetPath().ToString();
					ImGui::Selectable(StringConverter::WStringToString(assetPath).c_str());
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

		bool bIsFolder = nodeData.IsFolder();
		bool bHasChildren = node.HasChildren();
		String nodeName;
		Entity* entity = nullptr;

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

		if (bIsFolder)
		{
			nodeName = nodeData.AsFolder()->Name;
		}
		else
		{
			entity = nodeData.AsEntity();
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
			ImGui::SetDragDropPayload("Ion_DND_WorldTreeNode", &nodePtr, sizeof(WorldTreeNode*), ImGuiCond_Once);

			if (m_HoveredWorldTreeNodeDragTarget)
			{
				// Show the action that is going to be performed
				const WorldTreeNodeData& hovered = m_HoveredWorldTreeNodeDragTarget->Get();
				bool bCanAttach = entity->CanAttachTo(hovered.AsEntity());
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

			// First, check if the drag and drop node can even be attached to the target.
			ImGuiDragDropFlags dndFlags = ImGuiDragDropFlags_AcceptPeekOnly;
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Ion_DND_WorldTreeNode", dndFlags))
			{
				ionassert(payload->DataSize == sizeof(WorldTreeNode*));
				WorldTreeNode* nodePtr = *(WorldTreeNode**)payload->Data;
				if (nodePtr->Get().IsEntity())
				{
					Entity* source = nodePtr->Get().AsEntity();
					ionassert(source);
					// If the target entity is in the source entity's children,
					// it cannot be attached to.
					bCanAttachTo = source->CanAttachTo(entity);
				}
			}
			if (bCanAttachTo)
			{
				dndFlags = ImGuiDragDropFlags_None;
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Ion_DND_WorldTreeNode", dndFlags))
				{
					ionassert(payload->DataSize == sizeof(WorldTreeNode*));
					WorldTreeNode* nodePtr = *(WorldTreeNode**)payload->Data;

					if (entity)
					{
						// Attach source entity to this node's entity
						const WorldTreeNodeData& data = nodePtr->Get();
						if (data.IsEntity())
						{
							Entity* source = data.AsEntity();
							source->AttachTo(entity);
							ExpandWorldTreeToEntity(source);
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (ImGui::BeginPopupContextItem())
		{
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
				Entity* selectedEntity = EditorApplication::Get()->GetSelectedEntity();
				if (selectedEntity)
				{
					Component* selectedComponent = EditorApplication::Get()->GetSelectedComponent();

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

	void EditorLayer::DrawDetailsNameSection(Entity& entity)
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

	void EditorLayer::DrawDetailsRelationsSection(Entity& entity)
	{
		TRACE_FUNCTION();

		ImGui::PushID("Relations");

		if (ImGui::CollapsingHeaderUnframed("Relations"))
		{
			// Parent

			Entity* parent = entity.GetParent();
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

	void EditorLayer::DrawDetailsRelationsChildrenSection(Entity& entity)
	{
		TRACE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 2.0f));
		if (ImGui::BeginChild("children_list", ImVec2(-FLT_MIN, 4 * ImGui::GetTextLineHeightWithSpacing()), true))
		{
			ImGuiStyle& style = ImGui::GetStyle();
			const TArray<Entity*>& children = entity.GetChildren();
			int32 index = 0;
			for (Entity* child : children)
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
					EditorApplication::Get()->SelectObject(child);
					ExpandWorldTreeToEntity(child);
				}
				ImGui::PopStyleVar(2);
			}
			ImGui::EndChild();
		}
		ImGui::PopStyleVar();
	}

	void EditorLayer::DrawDetailsComponentTreeSection(Entity& entity)
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
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Ion_DND_InsertComponent", dndFlags))
				{
					ionassert(payload->DataSize == sizeof(DNDInsertComponentData));

					DNDInsertComponentData& data = *(DNDInsertComponentData*)payload->Data;
					Component* component = data.Instantiate(EditorApplication::Get()->GetEditorWorld(), data.ID);
					entity.AddComponent(component);
				}
				ImGui::EndDragDropTarget();
			}
		}
	}

	void EditorLayer::DrawDetailsEntitySection(Entity& entity)
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

	void EditorLayer::DrawComponentDetails(Component& component)
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
	static void DrawPropertyScalar(Component& component, INCProperty* prop, ImGuiDataType type, int32 nDims)
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

	void EditorLayer::DrawComponentDetailsProperty(Component& component, INCProperty* prop)
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

	void EditorLayer::DrawDetailsTransformSection(Entity& entity)
	{
		TRACE_FUNCTION();

		Transform transform = entity.GetTransform();
		if (DrawTransformSection(transform))
		{
			entity.SetTransform(transform);
		}
	}

	void EditorLayer::DrawDetailsRenderingSection(Entity& entity)
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

	void EditorLayer::DrawDetailsComponentSection(Component& component)
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

			Asset currentMeshAsset = meshComponent.GetMeshAsset();

			String previewName = currentMeshAsset ?
				currentMeshAsset->GetInfo().Name :
				"[None]";

			ImGuiComboFlags flags = ImGuiComboFlags_HeightLargest;
			if (ImGui::BeginCombo("Mesh Asset", previewName.c_str(), flags))
			{
				TArray<Asset> meshAssets = AssetRegistry::GetAllRegisteredAssets(EAssetType::Mesh);
				for (Asset& meshAsset : meshAssets)
				{
					bool bSelected = meshAsset == currentMeshAsset;
					if (ImGui::Selectable(meshAsset->GetInfo().Name.c_str(), bSelected))
					{
						TResourcePtr<MeshResource> meshResource = MeshResource::Query(meshAsset);

						TShared<Mesh> mesh = Mesh::CreateFromResource(meshResource);

						// @TODO: Yeah well this is a bit too much
						meshComponent.SetMeshAsset(meshAsset);
						meshComponent.SetMeshResource(meshResource);
						meshComponent.SetMesh(mesh);

						bChanged = true;
					}
				}

				ImGui::EndCombo();
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

	void EditorLayer::DrawComponentTreeContent(Entity& entity)
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
		if ((Component*)&component == EditorApplication::Get()->GetSelectedComponent())
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
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Ion_DND_InsertSceneComponent", dndFlags))
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
			Entity* owner = component.GetOwner();
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

	void EditorLayer::DrawComponentTreeNonSceneComponents(Entity& entity)
	{
		TRACE_FUNCTION();

		const Entity::ComponentSet& nonSceneComponents = entity.GetComponents();

		int64 uniqueIndex = 0;
		for (Component* component : nonSceneComponents)
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

	void EditorLayer::ExpandWorldTreeToEntity(Entity* entity)
	{
		ionassert(entity);

		// Discard the previous attempt
		m_ExpandWorldTreeChain.clear();

		WorldTreeNode* node = EditorApplication::Get()->GetEditorWorld()->FindWorldTreeNode(entity);
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

	void EditorLayer::UpdateRegisteredAssetsCache()
	{
		m_RegisteredAssetsCache = AssetRegistry::GetAllRegisteredAssets();
		// @TODO: Update the tree
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
