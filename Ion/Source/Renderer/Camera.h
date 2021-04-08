#pragma once

namespace Ion
{
	class ION_API Camera
	{
	public:
		static TShared<Camera> Create();

		~Camera() { };

		void Activate();

		void SetTransform(const FMatrix4& transformMatrix);
		FORCEINLINE const FMatrix4& GetTransform() const { return m_CameraTransform; }

		FORCEINLINE const FMatrix4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		void SetFOV(float fov);
		FORCEINLINE float GetFOV() const { return m_FOV; }

		void SetAspectRatio(float aspectRatio);
		FORCEINLINE float GetAspectRatio() const { return m_AspectRatio; }

		void SetNearClip(float nearClip);
		FORCEINLINE float GetNearClip() const { return m_NearClip; }

		void SetFarClip(float farClip);
		FORCEINLINE float GetFarClip() const { return m_FarClip; }

		const FMatrix4& UpdateViewProjectionMatrix();

	protected:
		Camera();

	private:
		FMatrix4 m_CameraTransform;
		FMatrix4 m_ViewProjectionMatrix;

		float m_FOV;
		float m_AspectRatio;
		float m_NearClip;
		float m_FarClip;

		bool m_bDirty;
	};
}
