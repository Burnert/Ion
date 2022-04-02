#include "EditorPCH.h"

#include "EditorLayer.h"
#include "EditorApplication.h"

#include "Engine/World.h"
#include "Engine/Entity.h"
#include "Engine/Components/SceneComponent.h"

#include "Renderer/Renderer.h"

#include "UserInterface/ImGui.h"

namespace Ion
{
namespace Editor
{
	EditorLayer::EditorLayer(const char* name) :
		Layer(name),
		m_EventDispatcher(this),
		m_ViewportSize({ }),
		m_ViewportRect({ }),
		m_bViewportHovered(false),
		m_bViewportCaptured(false),
		m_bViewportOpen(true),
		m_bContentBrowserOpen(true),
		m_bWorldTreePanelOpen(true),
		m_bDetailsPanelOpen(true),
		m_bDiagnosticsPanelOpen(false)
	{
	}

	void EditorLayer::OnAttach()
	{
		CreateViewportFramebuffer();
	}

	void EditorLayer::OnDetach()
	{
	}

	void EditorLayer::OnUpdate(float DeltaTime)
	{
		TRACE_FUNCTION();

		TryResizeViewportFramebuffer();
		DrawEditorUI();
	}

	void EditorLayer::OnRender()
	{
		TRACE_FUNCTION();

		Renderer* renderer = Renderer::Get();

		// Render to viewport

		renderer->SetRenderTarget(m_ViewportFramebuffer);

		renderer->Clear(Vector4(0.1f, 0.1f, 0.1f, 1.0f));
		renderer->RenderScene(EditorApplication::Get()->GetEditorScene());
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
		DrawViewportWindow();
		DrawContentBrowser();
		DrawWorldTreePanel();
		DrawDetailsPanel();
		DrawDiagnosticsPanel();

		ImGui::ShowDemoWindow();
		ImGui::ShowMetricsWindow();
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
				ImGui::MenuItem("Undo");
				ImGui::MenuItem("Redo");
				ImGui::Separator();
				ImGui::MenuItem("Cut");
				ImGui::MenuItem("Copy");
				ImGui::MenuItem("Paste");
				ImGui::MenuItem("Delete");
				ImGui::Separator();
				if (ImGui::MenuItem("Deselect"))
				{
					EditorApplication::Get()->SetSelectedEntity(nullptr);
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Viewport", nullptr, &m_bViewportOpen);
				ImGui::MenuItem("Content Browser", nullptr, &m_bContentBrowserOpen);
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
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void EditorLayer::DrawViewportWindow()
	{
		TRACE_FUNCTION();

		if (m_bViewportOpen)
		{
			ImGuiWindowFlags windowFlags =
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoNavFocus |
				ImGuiWindowFlags_NoBackground;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			if (m_bViewportCaptured)
				ImGui::SetNextWindowFocus();

			char windowName[16];
			sprintf_s(windowName, "Viewport##%i", /* ID */ 0);
			if (!ImGui::Begin(windowName, &m_bViewportOpen, windowFlags))
			{
				ImGui::End();
				ImGui::PopStyleVar(3);
				return;
			}
			ImGui::PopStyleVar(3);

			// Save the viewport rect for later
			m_ViewportRect = ImGui::GetWindowWorkRect();
			m_bViewportHovered = ImGui::IsWindowHovered();

			m_ViewportSize = UVector2(m_ViewportRect.z - m_ViewportRect.x, m_ViewportRect.w - m_ViewportRect.y);

			const TextureDimensions& viewportDimensions = m_ViewportFramebuffer->GetDimensions();
			ImGui::Image(m_ViewportFramebuffer->GetNativeID(), ImVec2((float)viewportDimensions.Width, (float)viewportDimensions.Height));

			ImGui::End();
		}
	}

	void EditorLayer::DrawContentBrowser()
	{
		TRACE_FUNCTION();

		if (m_bContentBrowserOpen)
		{
			ImGui::Begin("Content Browser", &m_bContentBrowserOpen);

			ImGui::PushID("ContentBrowser");

			ImGui::Text("Test");

			if (ImGui::Button("Change Mesh"))
			{
				EditorApplication::Get()->TestChangeMesh();
			}

			ImGui::PopID();

			ImGui::End();
		}
	}

	void EditorLayer::DrawWorldTreePanel()
	{
		TRACE_FUNCTION();

		if (m_bWorldTreePanelOpen)
		{
			if (ImGui::Begin("World Tree", &m_bWorldTreePanelOpen))
			{
				if (ImGui::Button("Deselect"))
				{
					EditorApplication::Get()->SetSelectedEntity(nullptr);
				}

				ImGui::Separator();

				World* editorWorld = EditorApplication::Get()->GetEditorWorld();
				if (editorWorld)
				{
					ImGui::PushID("WorldTree");

					DrawWorldTreeNodes();

					ImGui::PopID();
				}
			}
			ImGui::End();
		}
	}

	void EditorLayer::DrawWorldTreeNodes()
	{
		TRACE_FUNCTION();

		WorldTreeNode* firstExpandNode = PopWorldTreeExpandChain();
		DrawWorldTreeNodeChildren(EditorApplication::Get()->GetEditorWorld()->GetWorldTreeRoot(), firstExpandNode);
		// Clear the expand chain just in case
		if (!m_ExpandWorldTreeChain.empty())
			m_ExpandWorldTreeChain.clear();
	}

	void EditorLayer::DrawWorldTreeNodeChildren(const WorldTreeNode& node, WorldTreeNode* nextExpandNode)
	{
		TRACE_FUNCTION();

		int64 imguiNodeIndex = 0;
		for (const WorldTreeNode* child : node.GetChildren())
		{
			ionassert(child);

			const WorldTreeNodeData& nodeData = child->Get();

			bool bIsFolder = nodeData.IsFolder();
			bool bHasChildren = child->HasChildren();
			String nodeName;
			Entity* entity = nullptr;

			ImGuiTreeNodeFlags imguiNodeFlags =
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
				if (nextExpandNode == child)
				{
					ImGui::SetNextItemOpen(true);
					// And get the next node to expand
					nextExpandNode = PopWorldTreeExpandChain();
				}
			}

			void* nodeId = (void*)imguiNodeIndex++;
			// Retrieve the previous state of the tree node
			bool bWasOpen = ImGui::IsTreeNodeOpen(nodeId);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
			bool bImguiTreeNodeOpen = ImGui::TreeNodeEx(nodeId, imguiNodeFlags, "%s", nodeName.c_str());
			ImGui::PopStyleVar(2);

			// Select entity on click, only if the state of the node has not changed
			// (unless it has no children, since the state won't change then).
			if ((!bHasChildren || (bWasOpen == bImguiTreeNodeOpen)) &&
				entity && ImGui::IsItemClicked())
			{
				EditorApplication::Get()->SetSelectedEntity(entity);
			}

			if (bHasChildren && bImguiTreeNodeOpen)
			{
				DrawWorldTreeNodeChildren(*child, nextExpandNode);
				ImGui::TreePop();
			}
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
					ImGui::PushID("Details");

					DrawDetailsNameSection(*selectedEntity);
					DrawDetailsRelationsSection(*selectedEntity);
					ImGui::Separator();
					DrawDetailsComponentTreeSection(*selectedEntity);
					DrawDetailsTransformSection(*selectedEntity);
					DrawDetailsRenderingSection(*selectedEntity);

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
					EditorApplication::Get()->SetSelectedEntity(parent);
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
					EditorApplication::Get()->SetSelectedEntity(child);
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
				EditorApplication::Get()->SetSelectedComponent(nullptr);
			}

			Component* selectedComponent = EditorApplication::Get()->GetSelectedComponent();

			if (selectedComponent)
			{
				ImGui::Separator();

				bool bComponentDetailsOpen = ImGui::CollapsingHeader("Component Details");

				if (bComponentDetailsOpen)
				{
					if (selectedComponent->IsSceneComponent())
					{
						DrawSceneComponentDetails(*(SceneComponent*)selectedComponent);
					}
					else
					{
						// @TODO: Non-scene component details
					}
				}

				ImGui::Separator();
			}
		}
	}

	void EditorLayer::DrawSceneComponentDetails(SceneComponent& component)
	{
		TRACE_FUNCTION();

		ImGui::PushID("ComponentDetails");
		ImGui::Indent();
		DrawSceneComponentDetailsTransformSection(component);
		DrawSceneComponentDetailsRenderingSection(component);
		ImGui::Unindent();
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
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "Root");
		if (bRootOpen)
		{
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			DrawComponentTreeNodeChildren(*rootComponent, 1);
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

		void* nodeId = (void*)id;
		// Retrieve the previous state of the tree node
		bool bWasOpen = ImGui::IsTreeNodeOpen(nodeId);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
		bool bImguiTreeNodeOpen = ImGui::TreeNodeEx(nodeId, imguiNodeFlags, "%s", nodeName.c_str());
		bool bTreeNodeClicked = ImGui::IsItemClicked();
		ImGui::PopStyleVar(2);

		ImGui::SameLine();
		ImGui::TextDisabled("(%s)", component.GetClassDisplayName().c_str());

		// Select component on click, only if the state of the node has not changed
		// (unless it has no children, since the state won't change then).
		if ((!bHasChildren || (bWasOpen == bImguiTreeNodeOpen)) && bTreeNodeClicked)
		{
			EditorApplication::Get()->SetSelectedComponent(&component);
		}

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
				EditorApplication::Get()->SetSelectedComponent(component);
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(%s)", component->GetClassDisplayName().c_str());
		}
	}

	void EditorLayer::ExpandWorldTreeToEntity(Entity* entity)
	{
		ionassert(entity);

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

	bool EditorLayer::IsMouseInViewportRect() const
	{
		IVector2 cursorPos = InputManager::GetCursorPosition();
		return
			// @TODO: Get some IsPointInRect function goin' (like PtInRect)
			cursorPos.x >= m_ViewportRect.x && cursorPos.x <= m_ViewportRect.z &&
			cursorPos.y >= m_ViewportRect.y && cursorPos.y <= m_ViewportRect.w;
	}

	bool EditorLayer::CanCaptureViewport() const
	{
		return m_bViewportHovered && IsMouseInViewportRect();
	}

	void EditorLayer::OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event)
	{
		if (CanCaptureViewport())
		{
			// @TODO: Dispatch event to Viewport

			// Move editor camera on right click
			if (event.GetMouseButton() == Mouse::Right)
			{
				m_bViewportCaptured = true;
				EditorApplication::Get()->CaptureViewport(true);
			}
		}
	}

	void EditorLayer::OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event)
	{
		m_bViewportCaptured = false;
		EditorApplication::Get()->CaptureViewport(false);
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

	void EditorLayer::CreateViewportFramebuffer()
	{
		TRACE_FUNCTION();

		WindowDimensions windowDimensions = EditorApplication::GetWindow()->GetDimensions();

		TextureDescription desc { };
		desc.Dimensions.Width = windowDimensions.Width;
		desc.Dimensions.Height = windowDimensions.Height;
		desc.bUseAsRenderTarget = true;
		desc.bCreateColorAttachment = true;
		desc.bCreateDepthStencilAttachment = true;
		desc.bCreateDepthSampler = true;

		m_ViewportFramebuffer = Texture::Create(desc);
	}
	void EditorLayer::ResizeViewportFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		// @TODO: This function probably shouldn't even be called, if the framebuffer is not set
		if (!m_ViewportFramebuffer)
			return;

		TextureDescription desc = m_ViewportFramebuffer->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_ViewportFramebuffer = Texture::Create(desc);
	}

	void EditorLayer::TryResizeViewportFramebuffer()
	{
		TRACE_FUNCTION();

		if (m_ViewportSize.x && m_ViewportSize.y)
		{
			TextureDimensions viewportDimensions = m_ViewportFramebuffer->GetDimensions();
			if (m_ViewportSize != UVector2(viewportDimensions.Width, viewportDimensions.Height))
			{
				ResizeViewportFramebuffer(m_ViewportSize);

				EditorApplication::Get()->GetEditorCamera()->SetAspectRatio((float)m_ViewportSize.x / (float)m_ViewportSize.y);
			}
		}
	}
}
}
