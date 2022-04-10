#pragma once

#include "IonApp.h"

namespace Ion::Editor
{
	class EditorLayer;

	class EDITOR_API EditorApplication : public App
	{
	public:
		inline static EditorApplication* Get() { return s_Instance; }

		EditorApplication();
		virtual ~EditorApplication();

		virtual void OnInit() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void PostUpdate() override;
		virtual void OnRender() override;
		virtual void OnShutdown() override;
		virtual void OnEvent(const Event& event) override;

		void CaptureViewport(bool bCapture);
		void DriveEditorCameraRotation(float yawDelta, float pitchDelta);

		Entity* GetSelectedEntity() const;
		Component* GetSelectedComponent() const;
		bool IsAnyObjectSelected() const;

		void SelectObject(Entity* entity);
		void SelectObject(Component* component);
		void DeselectCurrentEntity();
		void DeselectCurrentComponent();
		void DeselectCurrentObject();
		void DeleteObject(Entity* entity);
		bool DeleteObject(Component* component);
		void DeleteSelectedObject();

		void ClickViewport(const IVector2& position);

		World* GetEditorWorld() const;
		TShared<Camera> GetEditorCamera() const;
		Scene* GetEditorScene() const;

		static void ExitEditor();

	protected:
		void OnWindowResizeEvent(const WindowResizeEvent& event);
		void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
		void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);
		void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event);
		void OnKeyPressedEvent(const KeyPressedEvent& event);

	private:
		void SetSelectedEntity(Entity* entity);
		void SetSelectedComponent(Component* component);

		void UpdateEditorCamera(float deltaTime);
		void UpdateEditorCameraLocation(float deltaTime);

		static REditorPassPrimitive CreateEditorPassPrimitive(SceneComponent* component);
		void PrepareEditorPass();
		void RenderEditorScene();

		void SelectClickedObject();

		void CreateViewportFramebuffer(const UVector2& size);
		void ResizeViewportFramebuffer(const UVector2& size);
		const TShared<Texture>& GetViewportFramebuffer() const;
		/* Creates the viewport, if it hasn't been done yet. */
		void TryResizeViewportFramebuffer(const UVector2& size);

		void CreateFinalSceneFramebuffer(const UVector2& size);
		void ResizeFinalSceneFramebuffer(const UVector2& size);
		void CreateEditorDataFramebuffer(const UVector2& size);
		void ResizeEditorDataFramebuffer(const UVector2& size);

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
		TShared<Texture> m_FinalSceneFramebuffer;

		TShared<EditorPassData> m_EditorPassData;
		TShared<Texture> m_EditorDataSelected;
		TShared<Texture> m_EditorDataObjectID;
		TShared<Texture> m_EditorDataObjectIDStaging;
		IVector2 m_ClickedViewportPoint;

		World* m_EditorMainWorld;
		Entity* m_SelectedEntity;
		Component* m_SelectedComponent;

		TShared<Camera> m_EditorCamera;
		Transform m_EditorCameraTransform;
		float m_EditorCameraMoveSpeed;

		bool m_bViewportCaptured;

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

	inline bool EditorApplication::IsAnyObjectSelected() const
	{
		return m_SelectedEntity || m_SelectedComponent;
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
