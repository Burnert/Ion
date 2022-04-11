#include "EditorPCH.h"

#include "EditorViewport.h"
#include "EditorApplication/EditorApplication.h"
#include "Editor/UI/ViewportUI.h"

#include "Renderer/Renderer.h"

namespace Ion::Editor
{
	EditorViewport::EditorViewport(const UVector2& initialSize, int32 index) :
		m_UI(MakeShared<EditorUIViewport>(this, index)),
		m_Camera(Camera::Create()),
		m_CameraMoveSpeed(5.0f),
		m_ClickedPoint({ -1, -1 }),
		m_bCaptured(false)
	{
		m_CameraTransform.SetLocation(Vector3(1.0f, 0.5f, 1.0f));
		m_CameraTransform.SetRotation(Rotator(Vector3(-10.0f, 45.0f, 0.0f)));
		m_Camera->SetFOV(Math::Radians(100.0f));
		m_Camera->SetNearClip(0.1f);
		m_Camera->SetFarClip(100.0f);

		CreateFramebuffers(initialSize);

		m_UI->SetOnResize([this](const UVector2& size)
		{
			TryResizeFramebuffers(size);
		});
	}

	void EditorViewport::Update(float deltaTime)
	{
		if (WasClickedLastFrame())
		{
			GUID clicked;
			if (ObtainClickedObject(clicked))
			{
				DispatchOnClicked(clicked);
			}
			m_ClickedPoint = { -1, -1 };
		}
	}

	void EditorViewport::Render()
	{
		if (m_ViewportFramebuffer)
		{
			Scene* editorScene = EditorApplication::Get()->GetEditorScene();
			ionassert(editorScene);

			m_Camera->SetTransform(m_CameraTransform.GetMatrix());
			editorScene->LoadCamera(m_Camera);

			TShared<EditorPassData> editorPassData = EditorApplication::Get()->GetEditorPassData();

			editorPassData->RTObjectID = m_ObjectIDFramebuffer;
			editorPassData->RTSelection = m_SelectedFramebuffer;

			// Render the scene

			Renderer::Get()->SetRenderTarget(m_FinalSceneFramebuffer);
			Renderer::Get()->Clear(Vector4(0.1f, 0.1f, 0.1f, 1.0f));
			Renderer::Get()->RenderScene(editorScene);

			// Render editor pass

			Renderer::Get()->RenderEditorPass(editorScene, *editorPassData);

			// Render to viewport framebuffer

			Renderer::Get()->SetRenderTarget(m_ViewportFramebuffer);
			Renderer::Get()->Clear(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			Renderer::Get()->RenderEditorViewport(m_FinalSceneFramebuffer, m_SelectedFramebuffer);
		}
	}

	void EditorViewport::Click(const IVector2& position)
	{
		m_ClickedPoint = position;
	}

	void EditorViewport::SetCaptureState(bool bCapture)
	{
		if (m_bCaptured != bCapture)
		{
			m_bCaptured = bCapture;

			EditorApplication::Get()->GetWindow()->ShowCursor(!bCapture);
			EditorApplication::Get()->GetWindow()->LockCursor(bCapture);

			if (m_bCaptured)
				DispatchOnCaptured();
			else
				DispatchOnReleased();
		}
	}

	void EditorViewport::DriveCameraRotation(const Vector2& axisValues)
	{
		float yawDelta = axisValues.y * 0.2f;
		float pitchDelta = axisValues.x * 0.2f;

		Rotator cameraRotation = m_CameraTransform.GetRotation();
		cameraRotation.SetPitch(Math::Clamp(cameraRotation.Pitch() - pitchDelta, -89.99f, 89.99f));
		cameraRotation.SetYaw(cameraRotation.Yaw() - yawDelta);
		m_CameraTransform.SetRotation(cameraRotation);
	}

	void EditorViewport::DriveCamera(const Vector3& axisValues, float deltaTime)
	{
		if (axisValues == Vector3(0.0f))
			return;

		Vector3 direction = Math::Normalize(
			m_CameraTransform.GetRightVector()   * axisValues.x +
			Vector3(0.0f, 1.0f, 0.0f)            * axisValues.y +
			m_CameraTransform.GetForwardVector() * axisValues.z);

		m_CameraTransform += direction * deltaTime * m_CameraMoveSpeed;
	}

	void EditorViewport::DispatchOnCaptured()
	{
		if (m_OnCaptured)
			m_OnCaptured(*this);
	}

	void EditorViewport::DispatchOnReleased()
	{
		if (m_OnReleased)
			m_OnReleased(*this);
	}

	bool EditorViewport::WasClickedLastFrame() const
	{
		return !(m_ClickedPoint.x == -1 || m_ClickedPoint.y == -1);
	}

	bool EditorViewport::ObtainClickedObject(GUID& outGuid)
	{
		if (!m_ObjectIDFramebuffer || !m_ObjectIDStagingFramebuffer)
			return false;

		// Copy the object ID to the staging texture
		m_ObjectIDFramebuffer->CopyTo(m_ObjectIDStagingFramebuffer);

		// Get the data on the CPU
		void* buffer = nullptr;
		int32 lineSize = 0;
		m_ObjectIDStagingFramebuffer->Map(buffer, lineSize, ETextureMapType::Read);
		ionassert(lineSize / sizeof(GUID) >= m_ObjectIDStagingFramebuffer->GetDimensions().Width);

		// Get the pixel as GUID bytes
		outGuid = GUID::Zero;
		memcpy(&outGuid, (GUID*)buffer + ((int32)m_ClickedPoint.y * (lineSize / sizeof(GUID)) + (int32)m_ClickedPoint.x), sizeof(GUID));

		// Cleanup
		m_ObjectIDStagingFramebuffer->Unmap();

		return true;
	}

	void EditorViewport::DispatchOnClicked(const GUID& clickedGuid) const
	{
		if (m_OnClicked)
			m_OnClicked(clickedGuid);
	}

	void EditorViewport::CreateFramebuffers(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc { };
		desc.DebugName = "Viewport";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateColorAttachment = true;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;

		m_ViewportFramebuffer = Texture::Create(desc);

		CreateFinalSceneFramebuffer(size);
		CreateEditorPassFramebuffers(size);
	}

	void EditorViewport::ResizeFramebuffers(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc = m_ViewportFramebuffer->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_ViewportFramebuffer = Texture::Create(desc);

		ResizeFinalSceneFramebuffer(size);
		ResizeEditorPassFramebuffers(size);
	}

	void EditorViewport::TryResizeFramebuffers(const UVector2& size)
	{
		if (size.x && size.y)
		{
			TextureDimensions viewportDimensions = m_ViewportFramebuffer->GetDimensions();
			if (size != UVector2(viewportDimensions.Width, viewportDimensions.Height))
			{
				ResizeFramebuffers(size);

				m_Camera->SetAspectRatio((float)size.x / (float)size.y);
			}
		}
	}

	void EditorViewport::CreateFinalSceneFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc { };
		desc.DebugName = "FinalScene";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateColorAttachment = true;
		desc.bCreateDepthStencilAttachment = true;
		desc.bCreateDepthSampler = true;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;

		m_FinalSceneFramebuffer = Texture::Create(desc);
	}

	void EditorViewport::ResizeFinalSceneFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc = m_FinalSceneFramebuffer->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_FinalSceneFramebuffer = Texture::Create(desc);
	}

	void EditorViewport::CreateEditorPassFramebuffers(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc { };
		desc.DebugName = "EditorSelected";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateColorAttachment = true;
		desc.bCreateDepthStencilAttachment = true;
		desc.bCreateDepthSampler = true;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;

		m_SelectedFramebuffer = Texture::Create(desc);

		desc = { };
		desc.DebugName = "EditorObjectID";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateColorAttachment = true;
		desc.bCreateDepthStencilAttachment = true;
		desc.Format = ETextureFormat::UInt128GUID;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;

		m_ObjectIDFramebuffer = Texture::Create(desc);

		desc = { };
		desc.DebugName = "EditorObjectIDStaging";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bCreateColorAttachment = true;
		desc.bAllowCPUReadAccess = true;
		desc.Usage = ETextureUsage::Staging;
		desc.Format = ETextureFormat::UInt128GUID;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;

		m_ObjectIDStagingFramebuffer = Texture::Create(desc);
	}

	void EditorViewport::ResizeEditorPassFramebuffers(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc = m_SelectedFramebuffer->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_SelectedFramebuffer = Texture::Create(desc);

		desc = m_ObjectIDFramebuffer->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_ObjectIDFramebuffer = Texture::Create(desc);

		desc = m_ObjectIDStagingFramebuffer->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;

		m_ObjectIDStagingFramebuffer = Texture::Create(desc);
	}
}
