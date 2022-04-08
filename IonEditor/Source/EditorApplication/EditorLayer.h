#pragma once

#include "Engine/Components/Component.h"

namespace Ion::Editor
{
	class EditorApplication;

	struct DNDInsertEntityData
	{
		using InstantiateFunc = Entity*(World*);
		InstantiateFunc* Instantiate;
	};

	struct DNDInsertComponentData
	{
		using InstantiateFunc = Component*(World*, ComponentTypeID);
		InstantiateFunc* Instantiate;
		ComponentTypeID ID;
	};

	enum class ESceneObjectType
	{
		Entity,
		Component,
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
		void DrawInsertPanelEntityTab();
		void DrawInsertPanelComponentTab();
		template<ESceneObjectType ObjectType, typename Lambda>
		void DrawInsertPanelElement(const String& name, Lambda instantiate, const ComponentDatabase::TypeInfo* info = nullptr);

		void DrawContentBrowser();

		void DrawWorldTreePanel();
		void DrawWorldTreeNodes();
		void DrawWorldTreeNodeChildren(const WorldTreeNode& node, WorldTreeNode* nextExpandNode = nullptr);
		void DrawWorldTreeNode(const WorldTreeNode& node, WorldTreeNode* nextExpandNode = nullptr);

		void DrawDetailsPanel();
		void DrawDetailsNameSection(Entity& entity);
		void DrawDetailsRelationsSection(Entity& entity);
		void DrawDetailsRelationsChildrenSection(Entity& entity);
		void DrawDetailsComponentTreeSection(Entity& entity);
		void DrawDetailsEntitySection(Entity& entity);
		void DrawDetailsTransformSection(Entity& entity);
		void DrawDetailsRenderingSection(Entity& entity);
		void DrawDetailsComponentSection(Component& component);

		void DrawSceneComponentDetails(SceneComponent& component);
		void DrawSceneComponentDetailsTransformSection(SceneComponent& component);
		void DrawSceneComponentDetailsRenderingSection(SceneComponent& component);
		void DrawComponentDetails(Component& component);
		void DrawComponentDetailsProperty(Component& component, INCProperty* prop);

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
		TArray<Entity*> m_EntitiesToDestroy;
		TArray<Component*> m_ComponentsToDestroy;

		UVector2 m_ViewportSize;
		Vector4 m_ViewportRect;
		bool m_bViewportHovered;
		bool m_bViewportCaptured;

		bool m_bViewportOpen;
		bool m_bInsertPanelOpen;
		bool m_bContentBrowserOpen;
		bool m_bWorldTreePanelOpen;
		bool m_bDetailsPanelOpen;

		bool m_bDiagnosticsPanelOpen;

		bool m_bImGuiMetricsOpen;
		bool m_bImGuiDemoOpen;

		friend class EditorApplication;
	};

	template<ESceneObjectType ObjectType, typename Lambda>
	inline void EditorLayer::DrawInsertPanelElement(const String& name, Lambda instantiate, const ComponentDatabase::TypeInfo* info)
	{
		static_assert(
			ObjectType == ESceneObjectType::Entity    && TIsConvertibleV<Lambda, TFunction<DNDInsertEntityData::InstantiateFunc>> ||
			ObjectType == ESceneObjectType::Component && TIsConvertibleV<Lambda, TFunction<DNDInsertComponentData::InstantiateFunc>>);

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
			if constexpr (ObjectType == ESceneObjectType::Entity)
			{
				ImGui::Text("Drag and drop to the viewport or double-click to add an entity.");
			}
			else if constexpr (ObjectType == ESceneObjectType::Component)
			{
				ImGui::Text("Drag and drop to the component tree to add a component.");
			}
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
			if constexpr (ObjectType == ESceneObjectType::Entity)
			{
				DNDInsertEntityData data { };
				data.Instantiate = instantiate;
				ImGui::SetDragDropPayload("Ion_DND_InsertEntity", &data, sizeof(DNDInsertEntityData), ImGuiCond_Once);
			}
			else if constexpr (ObjectType == ESceneObjectType::Component)
			{
				ionassert(info);

				String payloadType = info->bIsSceneComponent ?
					"Ion_DND_InsertSceneComponent" :
					"Ion_DND_InsertComponent";
				DNDInsertComponentData data { };
				data.Instantiate = instantiate;
				data.ID = info->ID;
				ImGui::SetDragDropPayload(payloadType.c_str(), &data, sizeof(DNDInsertComponentData), ImGuiCond_Once);
			}

			ImGui::Text(name.c_str());

			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		if constexpr (ObjectType == ESceneObjectType::Entity)
		{
			if (bActivated)
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					World* editorWorld = EditorApplication::Get()->GetEditorWorld();
					instantiate(editorWorld);
				}
			}
		}
	}
}
