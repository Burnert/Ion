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

	void Scene::AddDrawableObject(const TShared<IDrawable>& drawable)
	{
		m_DrawableObjects.insert(drawable);
	}

	bool Scene::RemoveDrawableObject(const TShared<IDrawable>& drawable)
	{
		return (bool)m_DrawableObjects.erase(drawable);
	}
}