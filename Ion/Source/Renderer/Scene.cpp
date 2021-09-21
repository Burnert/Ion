#include "IonPCH.h"

#include "Scene.h"

namespace Ion
{
	TShared<Scene> Scene::Create()
	{
		return MakeShareable(new Scene);
	}

	void Scene::SetActiveCamera(const TShared<Camera>& camera)
	{
		m_ActiveCamera = camera;
	}

	void Scene::SetAmbientLightColor(const Vector4& color)
	{
		m_AmbientLightColor = color;
	}

	void Scene::SetActiveDirectionalLight(DirectionalLight* light)
	{
		m_ActiveDirectionalLight = light;
	}

	void Scene::AddLight(Light* light)
	{
		m_Lights.insert(light);
	}

	bool Scene::RemoveLight(Light* light)
	{
		return (bool)m_Lights.erase(light);
	}

	void Scene::AddDrawableObject(IDrawable* drawable)
	{
		m_DrawableObjects.insert(drawable);
	}

	bool Scene::RemoveDrawableObject(IDrawable* drawable)
	{
		return (bool)m_DrawableObjects.erase(drawable);
	}
}
