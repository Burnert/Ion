#include "IonPCH.h"

#include "Camera.h"

#include "RenderAPI/RenderAPI.h"

namespace Ion
{
	TShared<Camera> Camera::Create()
	{
		return MakeShareable(new Camera);
	}

	void Camera::Activate()
	{
		// @TODO: Make the camera active when calling the function
		// Make the renderer use this camera's view projection matrix
	}

	void Camera::SetTransform(const FMatrix4& transformMatrix)
	{
		m_CameraTransform = transformMatrix;
		m_bDirty = true;
	}

	void Camera::SetFOV(float fov)
	{
		m_FOV = fov;
		m_bDirty = true;
	}

	void Camera::SetAspectRatio(float aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		m_bDirty = true;
	}

	void Camera::SetNearClip(float nearClip)
	{
		m_NearClip = nearClip;
		m_bDirty = true;
	}

	void Camera::SetFarClip(float farClip)
	{
		m_FarClip = farClip;
		m_bDirty = true;
	}

	void Camera::UpdateViewProjectionMatrix()
	{
		TRACE_FUNCTION();

		if (m_bDirty)
		{
			// @TODO: Cache these intermediate matrices (view, projection)
			m_ViewProjectionMatrix = FMatrix4(1.0f)
				* glm::perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip)
				* glm::affineInverse(m_CameraTransform);
		}
	}

	Camera::Camera() :
		m_CameraTransform(FMatrix4(1.0f)),
		m_ViewProjectionMatrix(FMatrix4(1.0f)),
		m_FOV(90.0f),
		m_AspectRatio(16.0f / 9.0f),
		m_NearClip(0.1f),
		m_FarClip(100.0f),
		m_bDirty(false)
	{ }
}
