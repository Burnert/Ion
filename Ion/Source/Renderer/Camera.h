#pragma once

namespace Ion
{
	struct RCameraRenderProxy
	{
		Matrix4 Transform;
		Matrix4 ViewProjectionMatrix;
		Matrix4 ViewMatrix;
		Matrix4 ProjectionMatrix;
		Vector3 Location;
		Vector3 Forward;
	};

	class ION_API Camera
	{
	public:
		static TShared<Camera> Create();

		~Camera() { };

		// @TODO: Setting the location here is a temporary solution

		FORCEINLINE const Vector3 GetLocation() const
		{
			return m_CameraTransform[3];
		}

		void SetTransform(const Matrix4& transformMatrix);
		FORCEINLINE const Matrix4& GetTransform() const { return m_CameraTransform; }

		FORCEINLINE const Matrix4& GetViewProjectionMatrix() const
		{
			UpdateMatrixCache();
			return m_ViewProjectionMatrix;
		}

		FORCEINLINE const Matrix4& GetViewMatrix() const
		{
			UpdateMatrixCache();
			return m_ViewMatrix;
		}

		FORCEINLINE const Matrix4& GetProjectionMatrix() const
		{
			UpdateMatrixCache();
			return m_ProjectionMatrix;
		}

		void SetFOV(float fov);
		FORCEINLINE float GetFOV() const { return m_FOV; }

		void SetAspectRatio(float aspectRatio);
		FORCEINLINE float GetAspectRatio() const { return m_AspectRatio; }

		void SetNearClip(float nearClip);
		FORCEINLINE float GetNearClip() const { return m_NearClip; }

		void SetFarClip(float farClip);
		FORCEINLINE float GetFarClip() const { return m_FarClip; }

		void CopyRenderData(RCameraRenderProxy& outRenderProxy);

	protected:
		Camera();

		void UpdateMatrixCache() const;

	private:
		// @TODO: Bad memory layout...
		Matrix4 m_CameraTransform;

		mutable Matrix4 m_ViewProjectionMatrix;
		mutable Matrix4 m_ViewMatrix;
		mutable Matrix4 m_ProjectionMatrix;

		float m_FOV;
		float m_AspectRatio;
		float m_NearClip;
		float m_FarClip;

		bool m_bDirty;
	};
}
