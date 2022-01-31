#pragma once

namespace Ion
{
namespace Editor
{
	class EditorApplication;

	class EDITOR_API EditorLayer : public Layer
	{
	public:
		EditorLayer(const char* name);

	protected:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float DeltaTime) override;
		virtual void OnRender() override;
		virtual void OnEvent(const Event& event) override;

		void DrawEditorUI();
		void DrawMainMenuBar();
		void DrawViewportWindow();
		void DrawContentBrowser();
		void DrawWorldTreePanel();
		void DrawDetailsPanel();

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

		Vector4 m_ViewportRect;
		bool m_bViewportHovered;
		bool m_bViewportCaptured;

		bool m_bViewportOpen;
		bool m_bContentBrowserOpen;
		bool m_bWorldTreePanelOpen;
		bool m_bDetailsPanelOpen;

		friend class EditorApplication;
	};
}
}
