#pragma once

#include "Core.h"
#include "EditorCore/EditorApi.h"
#include "Renderer/RendererCore.h"

namespace Ion::Editor
{
	class EditorViewport;

	// UIEditorViewport -------------------------------------------------------------------

	class EDITOR_API EditorUIViewport
	{
	public:
		EditorUIViewport(EditorViewport* owner, int32 index);

		void Draw();

		void SetOpen(bool bOpen);

		void SetWindowName(const String& name);

		const UVector2& GetSize() const;
		bool CanCapture() const;
		bool IsMouseInRect() const;

		bool& GetWindowOpenFlagRef();

	private:
		void Capture();
		void Release();

		template<typename Lambda>
		void SetOnResize(Lambda onResize);
		void DispatchOnResize(const UVector2& size);

	private:
		EditorViewport* m_Owner;

		TFunction<void(const UVector2&)> m_OnResize;

		String m_WindowName;

		UVector2 m_Size;
		Vector4 m_Rect;

		uint32 m_Index;

		bool m_bWindowOpen;
		bool m_bHovered;
		bool m_bCaptured;
		bool m_bMSAA;
		bool m_bFXAA;

		friend class EditorViewport;
	};

	inline const UVector2& EditorUIViewport::GetSize() const
	{
		return m_Size;
	}

	inline bool& EditorUIViewport::GetWindowOpenFlagRef()
	{
		return m_bWindowOpen;
	}

	template<typename Lambda>
	inline void EditorUIViewport::SetOnResize(Lambda onResize)
	{
		m_OnResize = onResize;
	}

	// EditorViewport -------------------------------------------------------------------

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

		void SetMSAAEnabled(bool bEnabled);
		void SetFXAAEnabled(bool bEnabled);

		const std::shared_ptr<RHITexture>& GetViewportFramebuffer() const;
		const std::shared_ptr<Camera>& GetCamera() const;
		const Transform& GetCameraTransform() const;

		const std::shared_ptr<EditorUIViewport>& GetUI() const;

		UVector2 GetSize() const;

		const GUID& GetGuid() const;

	private:
		void SetCaptureState(bool bCapture);

		void DispatchOnCaptured();
		void DispatchOnReleased();

		bool WasClickedLastFrame() const;
		bool ObtainClickedObject(GUID& outGuid);
		void DispatchOnClicked(const GUID& clickedGuid) const;

		void RenderEditorGrid();
		void RenderEditorBillboards();

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

		std::shared_ptr<EditorUIViewport> m_UI;

		std::shared_ptr<RHITexture> m_ViewportFinalColor;
		std::shared_ptr<RHITexture> m_ViewportPreFX;
		std::shared_ptr<RHITexture> m_ObjectIDColor;
		std::shared_ptr<RHITexture> m_ObjectIDDepthStencil;
		std::shared_ptr<RHITexture> m_ObjectIDStaging;
		EditorViewportTextures m_ViewportTextures;
		UVector2 m_ViewportSize;

		std::shared_ptr<Camera> m_Camera;
		Transform m_CameraTransform;
		float m_CameraMoveSpeed;

		IVector2 m_ClickedPoint;

		bool m_bCaptured;
		bool m_bEnableMSAA;
		bool m_bEnableFXAA;

		friend class EditorUIViewport;
	};

	inline const std::shared_ptr<Camera>& EditorViewport::GetCamera() const
	{
		return m_Camera;
	}

	inline const Transform& EditorViewport::GetCameraTransform() const
	{
		return m_CameraTransform;
	}

	inline const std::shared_ptr<EditorUIViewport>& EditorViewport::GetUI() const
	{
		return m_UI;
	}

	inline const GUID& EditorViewport::GetGuid() const
	{
		return m_ViewportGuid;
	}

	inline const std::shared_ptr<RHITexture>& EditorViewport::GetViewportFramebuffer() const
	{
		return m_ViewportFinalColor;
	}

	inline UVector2 EditorViewport::GetSize() const
	{
		return m_ViewportSize;
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
