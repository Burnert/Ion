#pragma once

#include "IonApp.h"
#include "EditorCore/EditorCore.h"
#include "Editor/EditorCommon.h"

#include "Engine/Components/ComponentOld.h"

#include "Renderer/RendererCore.h"

namespace Ion::Editor
{
	class EditorLayer;
	class EditorViewport;
	class ContentBrowser;
	class LogSettings;

	struct ViewportObject
	{
		std::shared_ptr<EditorViewport> Viewport;
	};

	class EDITOR_API EditorApplication : public IApp
	{
	public:
		inline static EditorApplication* Get() { return s_Instance; }

		EditorApplication();

		// IApp overrides:

		virtual void OnInit() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void PostUpdate() override;
		virtual void OnRender() override;
		virtual void OnShutdown() override;
		virtual void OnEvent(const Event& event) override;

		// End of IApp overrides

		EntityOld* GetSelectedEntity() const;
		ComponentOld* GetSelectedComponent() const;
		bool IsAnyObjectSelected() const;

		void SelectObject(EntityOld* entity);
		void SelectObject(ComponentOld* component);
		void DeselectCurrentEntity();
		void DeselectCurrentComponent();
		void DeselectCurrentObject();

		void DeleteObject(EntityOld* entity);
		bool DeleteObject(ComponentOld* component);
		void DeleteSelectedObject();

		void DuplicateObject(EntityOld* entity);

		World* GetEditorWorld() const;
		Scene* GetEditorScene() const;

		static void ExitEditor();

		/* Valid for the frame after PostUpdate */
		const std::shared_ptr<EditorPassData>& GetEditorPassData() const;

	protected:
		void OnWindowResizeEvent(const WindowResizeEvent& event);
		void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
		void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);
		void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event);
		void OnKeyPressedEvent(const KeyPressedEvent& event);

	private:
		std::shared_ptr<EditorViewport>& AddViewport();
		void RemoveViewport(const GUID& viewportID);
		std::shared_ptr<EditorViewport> GetViewport(const GUID& viewportID);
		void UpdateViewports(float deltaTime);
		void RenderViewports();
		void DrawViewports();

		void DriveCapturedViewportCamera(const Vector3& axisValues, float deltaTime);
		void DriveCapturedViewportCameraRotation(const Vector2& axisValues);

		void SetSelectedEntity(EntityOld* entity);
		void SetSelectedComponent(ComponentOld* component);

		void DriveCameraUpdate(float deltaTime);

		REditorPassPrimitive CreateEditorPassPrimitive(SceneComponent* component);
		REditorPassBillboardPrimitive CreateEditorPassBillboard(SceneComponent* component);
		void PrepareEditorPass();

		void SelectClickedObject(const GUID& clickedGuid);

	private:
		static EditorApplication* s_Instance;

		TEventDispatcher<EditorApplication> m_EventDispatcher;

		std::shared_ptr<EditorLayer> m_EditorLayer;

		std::shared_ptr<EditorPassData> m_EditorPassData;

		World* m_EditorMainWorld;
		EntityOld* m_SelectedEntity;
		ComponentOld* m_SelectedComponent;

		THashMap<GUID, std::shared_ptr<EditorViewport>> m_Viewports;
		std::weak_ptr<EditorViewport> m_MainViewport;
		std::shared_ptr<EditorViewport> m_CapturedViewport;

		std::shared_ptr<ContentBrowser> m_ContentBrowser;

		std::shared_ptr<LogSettings> m_LogSettings;

		friend class EditorLayer;
	};

	inline EntityOld* EditorApplication::GetSelectedEntity() const
	{
		return m_SelectedEntity;
	}

	inline ComponentOld* EditorApplication::GetSelectedComponent() const
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

	inline const std::shared_ptr<EditorPassData>& EditorApplication::GetEditorPassData() const
	{
		return m_EditorPassData;
	}
}
