#pragma once

#include "Core/Core.h"
#include "EditorCore/CoreApi.h"

namespace Ion::Editor
{
	class EditorUIViewport;

	class EDITOR_API EditorViewport
	{
	public:
		explicit EditorViewport(const UVector2& initialSize, int32 index);

		void Update(float deltaTime);
		void Render();

		void Click(const IVector2& position);
		template<typename Lambda>
		void SetOnClicked(Lambda onClicked);

		template<typename Lambda>
		void SetOnCaptured(Lambda onCaptured);
		template<typename Lambda>
		void SetOnReleased(Lambda onReleased);

		/* X - pitch, Y - yaw */
		void DriveCameraRotation(const Vector2& axisValues);
		/* X - right/left, Y - up/down, Z - forward/backward */
		void DriveCamera(const Vector3& axisValues, float deltaTime);

		const TShared<Texture>& GetViewportFramebuffer() const;
		const TShared<Camera>& GetCamera() const;

		const TShared<EditorUIViewport>& GetUI() const;

		const GUID& GetGuid() const;

	private:
		void SetCaptureState(bool bCapture);

		void DispatchOnCaptured();
		void DispatchOnReleased();

		bool WasClickedLastFrame() const;
		bool ObtainClickedObject(GUID& outGuid);
		void DispatchOnClicked(const GUID& clickedGuid) const;

		void CreateFramebuffers(const UVector2& size);
		void ResizeFramebuffers(const UVector2& size);
		/* Creates the viewport, if it hasn't been done yet. */
		void TryResizeFramebuffers(const UVector2& size);

		void CreateFinalSceneFramebuffer(const UVector2& size);
		void ResizeFinalSceneFramebuffer(const UVector2& size);
		void CreateEditorPassFramebuffers(const UVector2& size);
		void ResizeEditorPassFramebuffers(const UVector2& size);

	private:
		GUID m_ViewportGuid;

		TFunction<void(const GUID&)> m_OnClicked;
		TFunction<void(EditorViewport&)> m_OnCaptured;
		TFunction<void(EditorViewport&)> m_OnReleased;

		TShared<EditorUIViewport> m_UI;

		TShared<Texture> m_ViewportFramebuffer;
		TShared<Texture> m_FinalSceneFramebuffer;

		TShared<Texture> m_SelectedFramebuffer;
		TShared<Texture> m_ObjectIDFramebuffer;
		TShared<Texture> m_ObjectIDStagingFramebuffer;

		TShared<Camera> m_Camera;
		Transform m_CameraTransform;
		float m_CameraMoveSpeed;

		IVector2 m_ClickedPoint;

		bool m_bCaptured;

		friend class EditorUIViewport;
	};

	inline const TShared<Camera>& EditorViewport::GetCamera() const
	{
		return m_Camera;
	}

	inline const TShared<EditorUIViewport>& EditorViewport::GetUI() const
	{
		return m_UI;
	}

	inline const GUID& EditorViewport::GetGuid() const
	{
		return m_ViewportGuid;
	}

	inline const TShared<Texture>& EditorViewport::GetViewportFramebuffer() const
	{
		return m_ViewportFramebuffer;
	}

	template<typename Lambda>
	inline void EditorViewport::SetOnClicked(Lambda onClicked)
	{
		m_OnClicked = onClicked;
	}

	template<typename Lambda>
	inline void EditorViewport::SetOnCaptured(Lambda onCaptured)
	{
		m_OnCaptured = onCaptured;
	}

	template<typename Lambda>
	inline void EditorViewport::SetOnReleased(Lambda onReleased)
	{
		m_OnReleased = onReleased;
	}
}
