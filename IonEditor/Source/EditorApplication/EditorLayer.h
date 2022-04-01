#pragma once

namespace Ion
{
namespace Editor
{
	class EditorApplication;

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

		void CreateViewportFramebuffer();
		void ResizeViewportFramebuffer(const UVector2& size);
		void TryResizeViewportFramebuffer();

	public:
		TShared<Texture>& GetViewportFramebuffer();

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

		TShared<Texture> m_ViewportFramebuffer;
		UVector2 m_ViewportSize;
		Vector4 m_ViewportRect;
		bool m_bViewportHovered;
		bool m_bViewportCaptured;

		bool m_bViewportOpen;
		bool m_bContentBrowserOpen;
		bool m_bWorldTreePanelOpen;
		bool m_bDetailsPanelOpen;

		bool m_bDiagnosticsPanelOpen;

		friend class EditorApplication;
	};

	inline TShared<Texture>& EditorLayer::GetViewportFramebuffer()
	{
		return m_ViewportFramebuffer;
	}
}
}
