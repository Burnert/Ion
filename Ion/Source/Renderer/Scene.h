#pragma once

#include "Core/Core.h"

namespace Ion
{
	class Camera;
	class IDrawable;

	class ION_API Scene
	{
	public:
		static TShared<Scene> Create();

		void SetActiveCamera(const TShared<Camera>& camera);
		FORCEINLINE const TShared<Camera>& GetActiveCamera() const { return m_ActiveCamera; }

		// @TODO: For now, adding object to scene is going to be done this way.
		// Later, the World is going to pass renderable objects to the scene automatically.
		void AddDrawableObject(const TShared<IDrawable>& drawable);
		bool RemoveDrawableObject(const TShared<IDrawable>& drawable);
		FORCEINLINE const std::unordered_set<TShared<IDrawable>>& GetDrawableObjects() const { return m_DrawableObjects; }

		~Scene() { }

	protected:
		Scene() { }

	private:
		std::unordered_set<TShared<IDrawable>> m_DrawableObjects;
		TShared<Camera> m_ActiveCamera;
	};
}
