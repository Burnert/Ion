#include "EditorPCH.h"

#include "EditorLayer.h"
#include "EditorApplication.h"

#include "Engine/World.h"
#include "Engine/Entity.h"

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

		DrawWorldTreeNodeChildren(EditorApplication::Get()->GetEditorWorld()->GetWorldTreeRoot());
	}

	void EditorLayer::DrawWorldTreeNodeChildren(const WorldTreeNode& node)
	{
		TRACE_FUNCTION();

		int64 imguiNodeIndex = 0;
		for (const WorldTreeNode* child : node.GetChildren())
		{
			const WorldTreeNodeData& nodeData = child->Element();

			bool bIsFolder = nodeData.IsFolder();
			bool bHasChildren = child->HasChildren();
			String nodeName;
			Entity* entity = nullptr;

			ImGuiTreeNodeFlags imguiNodeFlags =
				ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_OpenOnDoubleClick |
				ImGuiTreeNodeFlags_SpanAvailWidth;
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
			}

			bool bImguiTreeNodeOpen = ImGui::TreeNodeEx((void*)imguiNodeIndex++, imguiNodeFlags, "%s", nodeName.c_str());

			// Select entity on click
			if (entity && ImGui::IsItemClicked())
			{
				EditorApplication::Get()->SetSelectedEntity(const_cast<Entity*>(entity));
			}

			if (bHasChildren && bImguiTreeNodeOpen)
			{
				DrawWorldTreeNodeChildren(*child);
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
					Component* selectedComponent = EditorApplication::Get()->GetSelectedComponent();

					ImGui::PushID("Details");

					DrawDetailsNameSection(selectedEntity);

					ImGui::Separator();

					DrawDetailsComponentTreeSection(selectedEntity);
					DrawDetailsTransformSection(selectedEntity);
					DrawDetailsRenderingSection(selectedEntity);

					if (selectedComponent)
					{
						ImGui::Separator();
						ImGui::Spacing();
						ImGui::Text("Component Details");
						ImGui::Spacing();
						ImGui::Separator();
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

	void EditorLayer::DrawDetailsNameSection(Entity* entity)
	{
		TRACE_FUNCTION();

		ImGui::PushID("Name");

		ImGuiInputTextFlags imguiInputTextFlags =
			ImGuiInputTextFlags_EnterReturnsTrue;

		char name[128] = { };
		strcpy_s(name, entity->GetName().c_str());
		if (ImGui::InputText("Entity Name", name, sizeof(name), imguiInputTextFlags))
		{
			entity->SetName(name);
		}

		ImGui::PopID();
	}

	void EditorLayer::DrawDetailsComponentTreeSection(Entity* entity)
	{
		TRACE_FUNCTION();

		if (ImGui::CollapsingHeader("Component Tree", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::BeginChild("ComponentTree", ImVec2(0, 100), true);
			ImGui::PushID("ComponentTree");

			ImGui::Text("WORK IN PROGRESS");
			ImGui::Text("The entity has no components.");

			ImGui::PopID();
			ImGui::EndChild();
		}
	}

	void EditorLayer::DrawDetailsTransformSection(Entity* entity)
	{
		TRACE_FUNCTION();

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Transform");

			const Transform& currentTransform = entity->GetTransform();

			Vector3 location       = currentTransform.GetLocation();
			Vector3 rotationAngles = currentTransform.GetRotation().Angles();
			Vector3 scale          = currentTransform.GetScale();

			bool bChanged = false;
			bChanged |= ImGui::DragFloat3("Location", (float*)&location,       0.0125f, 0.0f, 0.0f, "%.5f");
			bChanged |= ImGui::DragFloat3("Rotation", (float*)&rotationAngles, 0.5f,    0.0f, 0.0f, "%.5f");
			bChanged |= ImGui::DragFloat3("Scale",    (float*)&scale,          0.0125f, 0.0f, 0.0f, "%.5f");

			if (bChanged)
			{
				Transform newTransform = { location, Rotator(rotationAngles), scale };
				entity->SetTransform(newTransform);
			}

			ImGui::PopID();
		}
	}

	void EditorLayer::DrawDetailsRenderingSection(Entity* entity)
	{
		TRACE_FUNCTION();

		if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID("Rendering");

			bool bVisible = entity->IsVisible();
			bool bVisibleInGame = entity->IsVisibleInGame();

			if (ImGui::Checkbox("Visible", &bVisible))
			{
				entity->SetVisible(bVisible);
			}
			if (ImGui::Checkbox("Visible in game", &bVisibleInGame))
			{
				entity->SetVisibleInGame(bVisibleInGame);
			}

			ImGui::PopID();
		}
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

		TextureDescription desc{ };
		desc.Dimensions.Width = windowDimensions.Width;
		desc.Dimensions.Height = windowDimensions.Height;
		desc.bUseAsRenderTarget = true;
		desc.bCreateDepthStencilAttachment = true;

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
