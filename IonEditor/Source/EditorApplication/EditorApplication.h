#pragma once

#define DISABLE_USING_NAMESPACE_ION
#define DONT_PAUSE_ON_EXIT
#include "IonApp.h"

namespace Ion
{
namespace Editor
{
	class EditorApplication : public IonApplication
	{
	public:
		EditorApplication();
		virtual ~EditorApplication();

		virtual void OnInit() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnRender() override;
		virtual void OnShutdown() override;
		virtual void OnEvent(const Event& event) override;

	protected:
		void DrawEditorUI();

		void DrawViewportWindow();

		void OnWindowResizeEvent(const WindowResizeEvent& event);
		void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
		void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);
		void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event);

	private:
		void CreateViewportFramebuffer();
		void ResizeViewportFramebuffer(const UVector2& size);
		void TryResizeViewportFramebuffer();

	private:
		using EventFunctions = TEventFunctionPack<
			TMemberEventFunction<EditorApplication, WindowResizeEvent,        &EditorApplication::OnWindowResizeEvent>,
			TMemberEventFunction<EditorApplication, MouseButtonPressedEvent,  &EditorApplication::OnMouseButtonPressedEvent>,
			TMemberEventFunction<EditorApplication, MouseButtonReleasedEvent, &EditorApplication::OnMouseButtonReleasedEvent>,
			TMemberEventFunction<EditorApplication, RawInputMouseMovedEvent,  &EditorApplication::OnRawInputMouseMovedEvent>
		>;
		EventDispatcher<EventFunctions, EditorApplication> m_EventDispatcher;

		TShared<Texture> m_ViewportFramebuffer;
		UVector2 m_ViewportSize;

		TShared<Scene> m_Scene;
		TShared<Camera> m_EditorCamera;

		Transform m_EditorCameraTransform;

		bool m_bEditorCameraCaptured;
	};
}
}

USE_APPLICATION_CLASS(Ion::Editor::EditorApplication);
