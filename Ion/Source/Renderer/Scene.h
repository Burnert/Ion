#pragma once

#include "Core/Core.h"

namespace Ion
{
	class IDrawable;
	class Camera;
	class Light;
	class DirectionalLight;

	class ION_API Scene
	{
	public:
		static TShared<Scene> Create();

		void SetActiveCamera(const TShared<Camera>& camera);
		FORCEINLINE const TShared<Camera>& GetActiveCamera() const { return m_ActiveCamera; }

		void SetAmbientLightColor(const Vector4& color);
		inline const Vector4& GetAmbientLightColor() const { return m_AmbientLightColor; }

		void SetActiveDirectionalLight(DirectionalLight* light);
		inline const DirectionalLight* GetActiveDirectionalLight() const { return m_ActiveDirectionalLight; }

		void AddLight(Light* light);
		bool RemoveLight(Light* light);
		inline const std::unordered_set<Light*>& GetLights() const { return m_Lights; }

		// @TODO: For now, adding object to scene is going to be done this way.
		// Later, the World is going to pass renderable objects to the scene automatically.
		void AddDrawableObject(IDrawable* drawable);
		bool RemoveDrawableObject(IDrawable* drawable);
		FORCEINLINE const std::unordered_set<IDrawable*>& GetDrawableObjects() const { return m_DrawableObjects; }

		~Scene() { }

	protected:
		Scene() :
			m_ActiveDirectionalLight(nullptr)
		{ }

	private:
		std::unordered_set<IDrawable*> m_DrawableObjects;
		TShared<Camera> m_ActiveCamera;

		Vector4 m_AmbientLightColor;
		DirectionalLight* m_ActiveDirectionalLight;
		std::unordered_set<Light*> m_Lights;
	};
}
