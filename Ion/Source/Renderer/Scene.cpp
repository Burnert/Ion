#include "IonPCH.h"

#include "Scene.h"
#include "Drawable.h"
#include "Material.h"
#include "Light.h"

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
		ionassert(m_Lights.size() < MaxLights, "There cannot be more than %d lights in a scene!", MaxLights);
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

	void Scene::UpdateRenderData()
	{
		ionassert(m_ActiveCamera, "Cannot render without an active camera.");

		m_RenderPrimitives.clear();
		for (IDrawable* drawable : m_DrawableObjects)
		{
			RPrimitiveRenderProxy primitiveProxy;
			primitiveProxy.VertexBuffer.Ptr = drawable->GetVertexBufferRaw();
			primitiveProxy.IndexBuffer.Ptr = drawable->GetIndexBufferRaw();
			primitiveProxy.Material.Ptr = drawable->GetMaterialRaw();
			primitiveProxy.Shader.Ptr = primitiveProxy.Material.Ptr->GetShaderRaw();
			primitiveProxy.Transform = drawable->GetTransformMatrix();

			m_RenderPrimitives.push_back(primitiveProxy);
		}

		m_RenderLights.clear();
		for (Light* light : m_Lights)
		{
			RLightRenderProxy lightProxy;
			light->CopyRenderData(lightProxy);
			m_RenderLights.push_back(lightProxy);
		}

		if (m_ActiveDirectionalLight)
		{
			m_ActiveDirectionalLight->CopyRenderData(m_RenderDirLight);
		}
		else
		{
			m_RenderDirLight = { };
		}

		m_ActiveCamera->CopyRenderData(m_RenderCamera);

		// Shrink arrays

		if (m_RenderPrimitives.capacity() > m_RenderPrimitives.size() * 2)
			m_RenderPrimitives.shrink_to_fit();

		if (m_RenderLights.capacity() > m_RenderLights.size() * 2)
			m_RenderLights.shrink_to_fit();
	}

	void Scene::CopySceneData(RSceneProxy& outProxy) const
	{
		outProxy.RenderPrimitives = m_RenderPrimitives;
		outProxy.RenderLights = m_RenderLights;
		outProxy.RenderDirLight = m_RenderDirLight;
		outProxy.RenderCamera = m_RenderCamera;
		outProxy.AmbientLightColor = m_AmbientLightColor;
		outProxy.bHasDirLight = (bool)m_ActiveDirectionalLight;
	}
}
