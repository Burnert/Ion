#pragma once

#include "IonApp.h"
#include "EditorCore/EditorCore.h"

#include "Engine/Components/Component.h"

namespace Ion::Editor
{
	class EditorLayer;
	class EditorViewport;

	struct ViewportObject
	{
		TShared<EditorViewport> Viewport;
	};

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

		World* GetEditorWorld() const;
		Scene* GetEditorScene() const;

		static void ExitEditor();

		/* Valid for the frame after PostUpdate */
		const TShared<EditorPassData>& GetEditorPassData() const;

	protected:
		void OnWindowResizeEvent(const WindowResizeEvent& event);
		void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
		void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);
		void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event);
		void OnKeyPressedEvent(const KeyPressedEvent& event);

	private:
		TShared<EditorViewport>& AddViewport();
		void RemoveViewport(const GUID& viewportID);
		TShared<EditorViewport> GetViewport(const GUID& viewportID);
		void UpdateViewports(float deltaTime);
		void RenderViewports();
		void DrawViewports();

		void DriveCapturedViewportCamera(const Vector3& axisValues, float deltaTime);
		void DriveCapturedViewportCameraRotation(const Vector2& axisValues);

		void SetSelectedEntity(Entity* entity);
		void SetSelectedComponent(Component* component);

		void DriveCameraUpdate(float deltaTime);

		REditorPassPrimitive CreateEditorPassPrimitive(SceneComponent* component);
		void PrepareEditorPass();
		RPrimitiveRenderProxy CreateEditorBillboardPrimitive(SceneComponent* component, const TShared<Texture>& texture);
		void PrepareEditorBillboards();

		void SelectClickedObject(const GUID& clickedGuid);

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

		TShared<EditorPassData> m_EditorPassData;

		World* m_EditorMainWorld;
		Entity* m_SelectedEntity;
		Component* m_SelectedComponent;

		THashMap<GUID, TShared<EditorViewport>> m_Viewports;
		TWeak<EditorViewport> m_MainViewport;
		TShared<EditorViewport> m_CapturedViewport;

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

	inline const TShared<EditorPassData>& EditorApplication::GetEditorPassData() const
	{
		return m_EditorPassData;
	}
}
