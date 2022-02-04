#pragma once

#include "Ion.h"

namespace Ion
{
namespace Editor
{
	class EditorLayer;

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

		World* GetEditorWorld() const;
		TShared<Camera> GetEditorCamera() const;
		Scene* GetScene() const { return m_Scene; }

		void TestChangeMesh();

		static void ExitEditor();

	protected:
		void OnWindowResizeEvent(const WindowResizeEvent& event);
		void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
		void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);
		void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event);

	private:
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

		TShared<EditorLayer> m_EditorLayer;

		World* m_EditorMainWorld;

		Scene* m_Scene;
		TShared<Camera> m_EditorCamera;
		Transform m_EditorCameraTransform;
		float m_EditorCameraMoveSpeed;

		bool m_bViewportCaptured;

		MeshComponent* m_TestMeshComponent;
		LightComponent* m_TestLightComponent;
		DirectionalLightComponent* m_TestDirLightComponent;

		friend class EditorLayer;
	};

	inline World* EditorApplication::GetEditorWorld() const
	{
		return m_EditorMainWorld;
	}

	inline TShared<Camera> EditorApplication::GetEditorCamera() const
	{
		return m_EditorCamera;
	}
}
}
