#pragma once

#include "Ion.h"

namespace Ion
{
namespace Editor
{
	class EDITOR_API EditorApplication : public IonApplication
	{
	public:
		inline static EditorApplication* Get() { return s_Instance; }

		EditorApplication();
		virtual ~EditorApplication();

		virtual void OnInit() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnRender() override;
		virtual void OnShutdown() override;
		virtual void OnEvent(const Event& event) override;

		void CaptureViewport(bool bCapture);
		void DriveEditorCameraRotation(float yawDelta, float pitchDelta);

	protected:
		void OnWindowResizeEvent(const WindowResizeEvent& event);
		void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
		void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);
		void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event);

	private:
		void CreateViewportFramebuffer();
		void ResizeViewportFramebuffer(const UVector2& size);
		void TryResizeViewportFramebuffer();

		void UpdateEditorCamera(float deltaTime);
		void UpdateEditorCameraLocation(float deltaTime);

	private:
		static EditorApplication* s_Instance;

		using EventFunctions = TEventFunctionPack<
			TMemberEventFunction<EditorApplication, WindowResizeEvent,        &OnWindowResizeEvent>,
			TMemberEventFunction<EditorApplication, MouseButtonPressedEvent,  &OnMouseButtonPressedEvent>,
			TMemberEventFunction<EditorApplication, MouseButtonReleasedEvent, &OnMouseButtonReleasedEvent>,
			TMemberEventFunction<EditorApplication, RawInputMouseMovedEvent,  &OnRawInputMouseMovedEvent>
		>;
		EventDispatcher<EventFunctions, EditorApplication> m_EventDispatcher;

		TShared<Texture> m_ViewportFramebuffer;
		UVector2 m_ViewportSize;

		TShared<Scene> m_Scene;
		TShared<Camera> m_EditorCamera;
		Transform m_EditorCameraTransform;
		float m_EditorCameraMoveSpeed;

		bool m_bViewportCaptured;

		friend class EditorLayer;
	};
}
}
