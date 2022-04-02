#pragma once

#include "Ion.h"

namespace Ion::Editor
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

		void SetSelectedEntity(Entity* entity);
		Entity* GetSelectedEntity() const;

		void SetSelectedComponent(Component* component);
		Component* GetSelectedComponent() const;

		World* GetEditorWorld() const;
		TShared<Camera> GetEditorCamera() const;
		Scene* GetEditorScene() const;

		void TestChangeMesh();

		static void ExitEditor();

	protected:
		void OnWindowResizeEvent(const WindowResizeEvent& event);
		void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
		void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);
		void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event);
		void OnKeyPressedEvent(const KeyPressedEvent& event);

	private:
		void UpdateEditorCamera(float deltaTime);
		void UpdateEditorCameraLocation(float deltaTime);

		void CreateViewportFramebuffer();
		void ResizeViewportFramebuffer(const UVector2& size);
		void TryResizeViewportFramebuffer(const UVector2& size);
		const TShared<Texture>& GetViewportFramebuffer() const;

	private:
		static EditorApplication* s_Instance;

		using EventFunctions = TEventFunctionPack<
			TMemberEventFunction<EditorApplication, WindowResizeEvent,        &OnWindowResizeEvent>,
			TMemberEventFunction<EditorApplication, MouseButtonPressedEvent,  &OnMouseButtonPressedEvent>,
			TMemberEventFunction<EditorApplication, MouseButtonReleasedEvent, &OnMouseButtonReleasedEvent>,
			TMemberEventFunction<EditorApplication, RawInputMouseMovedEvent,  &OnRawInputMouseMovedEvent>,
			TMemberEventFunction<EditorApplication, KeyPressedEvent,          &OnKeyPressedEvent>
		>;
		EventDispatcher<EventFunctions, EditorApplication> m_EventDispatcher;

		TShared<EditorLayer> m_EditorLayer;
		TShared<Texture> m_ViewportFramebuffer;

		World* m_EditorMainWorld;
		Entity* m_SelectedEntity;
		Component* m_SelectedComponent;

		TShared<Camera> m_EditorCamera;
		Transform m_EditorCameraTransform;
		float m_EditorCameraMoveSpeed;

		bool m_bViewportCaptured;

		MeshComponent* m_TestMeshComponent;
		LightComponent* m_TestLightComponent;
		DirectionalLightComponent* m_TestDirLightComponent;

		friend class EditorLayer;
	};

	inline Entity* EditorApplication::GetSelectedEntity() const
	{
		return m_SelectedEntity;
	}

	inline Component* EditorApplication::GetSelectedComponent() const
	{
		return m_SelectedComponent;
	}

	inline World* EditorApplication::GetEditorWorld() const
	{
		return m_EditorMainWorld;
	}

	inline TShared<Camera> EditorApplication::GetEditorCamera() const
	{
		return m_EditorCamera;
	}

	inline const TShared<Texture>& EditorApplication::GetViewportFramebuffer() const
	{
		return m_ViewportFramebuffer;
	}
}
