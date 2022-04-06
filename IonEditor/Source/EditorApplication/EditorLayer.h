#pragma once

namespace Ion::Editor
{
	class EditorApplication;

	struct DNDEntityInsertData
	{
		using InstantiateFunc = void(World*);
		InstantiateFunc* Instantiate;
	};

	class EDITOR_API EditorLayer : public Layer
	{
	public:
		using WorldTreeNode = TTreeNode<WorldTreeNodeData>;

		EditorLayer(const char* name);

		void ExpandWorldTreeToEntity(Entity* entity);

	protected:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float DeltaTime) override;
		virtual void OnRender() override;
		virtual void OnEvent(const Event& event) override;

		// UI drawing related functions:

		void DrawEditorUI();

		void DrawMainMenuBar();

		void DrawViewportWindow();

		void DrawInsertPanel();
		template<typename Lambda>
		void DrawInsertPanelElement(const String& name, Lambda onInstantiate);

		void DrawContentBrowser();

		void DrawWorldTreePanel();
		void DrawWorldTreeNodes();
		void DrawWorldTreeNodeChildren(const WorldTreeNode& node, WorldTreeNode* nextExpandNode = nullptr);

		void DrawDetailsPanel();
		void DrawDetailsNameSection(Entity& entity);
		void DrawDetailsRelationsSection(Entity& entity);
		void DrawDetailsRelationsChildrenSection(Entity& entity);
		void DrawDetailsComponentTreeSection(Entity& entity);
		void DrawDetailsTransformSection(Entity& entity);
		void DrawDetailsRenderingSection(Entity& entity);

		void DrawSceneComponentDetails(SceneComponent& component);
		void DrawSceneComponentDetailsTransformSection(SceneComponent& component);
		void DrawSceneComponentDetailsRenderingSection(SceneComponent& component);

		/* Returns true if the transform has changed. */
		bool DrawTransformSection(Transform& inOutTransform);
		/* Returns true if the settings have changed */
		bool DrawRenderingSection(bool& bInOutVisible, bool& bInOutVisibleInGame);
		/* Returns true if the settings have changed */
		bool DrawMeshSection(MeshComponent& meshComponent);

		void DrawDiagnosticsPanel();

		void DrawComponentTreeContent(Entity& entity);
		void DrawComponentTreeNodeChildren(SceneComponent& component, int64 startId = 0);
		bool DrawComponentTreeSceneComponentNode(SceneComponent& component, int64 id = 0, bool bDrawChildren = false);
		void DrawComponentTreeNonSceneComponents(Entity& entity);

		// End of UI drawing related functions

		WorldTreeNode* PopWorldTreeExpandChain();

		bool IsMouseInViewportRect() const;
		bool CanCaptureViewport() const;

		void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
		void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);
		void OnMouseDoubleClickEvent(const MouseDoubleClickEvent& event);
		void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event);
		void OnRawInputMouseScrolledEvent(const RawInputMouseScrolledEvent& event);
		void OnKeyPressedEvent(const KeyPressedEvent& event);
		void OnKeyReleasedEvent(const KeyReleasedEvent& event);

	private:
		using EventFunctions = TEventFunctionPack<
			TMemberEventFunction<EditorLayer, MouseButtonPressedEvent,    &OnMouseButtonPressedEvent>,
			TMemberEventFunction<EditorLayer, MouseButtonReleasedEvent,   &OnMouseButtonReleasedEvent>,
			TMemberEventFunction<EditorLayer, MouseDoubleClickEvent,      &OnMouseDoubleClickEvent>,
			TMemberEventFunction<EditorLayer, RawInputMouseMovedEvent,    &OnRawInputMouseMovedEvent>,
			TMemberEventFunction<EditorLayer, RawInputMouseScrolledEvent, &OnRawInputMouseScrolledEvent>,
			TMemberEventFunction<EditorLayer, KeyPressedEvent,            &OnKeyPressedEvent>,
			TMemberEventFunction<EditorLayer, KeyReleasedEvent,           &OnKeyReleasedEvent>
		>;
		EventDispatcher<EventFunctions, EditorLayer> m_EventDispatcher;

		TArray<WorldTreeNode*> m_ExpandWorldTreeChain;

		UVector2 m_ViewportSize;
		Vector4 m_ViewportRect;
		bool m_bViewportHovered;
		bool m_bViewportCaptured;

		bool m_bViewportOpen;
		bool m_bInsertWindowOpen;
		bool m_bContentBrowserOpen;
		bool m_bWorldTreePanelOpen;
		bool m_bDetailsPanelOpen;

		bool m_bDiagnosticsPanelOpen;

		bool m_bImGuiMetricsOpen;
		bool m_bImGuiDemoOpen;

		friend class EditorApplication;
	};

	template<typename Lambda>
	inline void EditorLayer::DrawInsertPanelElement(const String& name, Lambda onInstantiate)
	{
		static_assert(TIsConvertibleV<Lambda, TFunction<void(World*)>>);

		ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0, 0.5f));
		// Make the selectable's style unresponsive to clicks
		ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, activeColor);
		ImGuiSelectableFlags flags = ImGuiSelectableFlags_AllowDoubleClick;
		bool bActivated = ImGui::Selectable(name.c_str(), false, flags, ImVec2(ImGui::GetContentRegionAvail().x, 30));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(180);
			ImGui::Text("Drag and drop to the viewport or double-click to add an entity.");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
			EditorApplication::Get()->SetCursor(
				ImGui::IsItemActive() ?
				ECursorType::GrabClosed :
				ECursorType::Grab);
		}
		ImGuiDragDropFlags dndFlags = ImGuiDragDropFlags_None;
		if (ImGui::BeginDragDropSource(dndFlags))
		{
			DNDEntityInsertData data { };
			data.Instantiate = onInstantiate;
			ImGui::SetDragDropPayload("Ion_DND_InsertEntity", &data, sizeof(DNDEntityInsertData), ImGuiCond_Once);

			ImGui::Text(name.c_str());

			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		if (bActivated)
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				World* editorWorld = EditorApplication::Get()->GetEditorWorld();
				onInstantiate(editorWorld);
			}
		}
	}
}
