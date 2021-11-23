#include "IonPCH.h"

#include "Camera.h"

#include "RenderAPI/RenderAPI.h"

namespace Ion
{
	TShared<Camera> Camera::Create()
	{
		return MakeShareable(new Camera);
	}

	void Camera::SetTransform(const Matrix4& transformMatrix)
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

	void Camera::CopyRenderData(RCameraRenderProxy& outRenderProxy)
	{
		outRenderProxy.Transform = GetTransform();
		outRenderProxy.ViewProjectionMatrix = GetViewProjectionMatrix();
		outRenderProxy.ViewMatrix = GetViewMatrix();
		outRenderProxy.ProjectionMatrix = GetProjectionMatrix();
		outRenderProxy.Location = GetLocation();
		outRenderProxy.Forward = Vector3(GetTransform() * Vector4(0.0f, 0.0f, -1.0f, 0.0f));
	}

	void Camera::UpdateMatrixCache() const
	{
		if (m_bDirty)
		{
			m_ViewMatrix = Math::AffineInverse(m_CameraTransform);
			m_ProjectionMatrix = Math::Perspective(m_FOV, m_AspectRatio, m_NearClip, m_FarClip);

			m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
		}
	}

	Camera::Camera() :
		m_CameraLocation(Vector3(0.0f)),
		m_CameraTransform(Matrix4(1.0f)),
		m_ViewProjectionMatrix(Matrix4(1.0f)),
		m_ViewMatrix(Matrix4(1.0f)),
		m_ProjectionMatrix(Matrix4(1.0f)),
		m_FOV(90.0f),
		m_AspectRatio(16.0f / 9.0f),
		m_NearClip(0.1f),
		m_FarClip(100.0f),
		m_bDirty(false)
	{ }
}
