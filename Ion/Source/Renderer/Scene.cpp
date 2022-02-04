#include "IonPCH.h"

#include "Scene.h"
#include "Drawable.h"
#include "Material.h"
#include "Light.h"
#include "UniformBuffer.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	Scene::Scene() :
		m_AmbientLightColor(0.0f),
		m_ActiveDirectionalLight(nullptr),
		m_RenderCamera({ }),
		m_RenderDirLight({ })
	{
		m_SceneUniformBuffer = MakeShareable(UniformBuffer::Create(SceneUniforms()));
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

	void Scene::UpdateRenderData() // @TODO: scrap all of this (the old way)
	{
		ionassert(m_ActiveCamera, "Cannot render without an active camera.");

		m_RenderPrimitives.clear();
		for (IDrawable* drawable : m_DrawableObjects)
		{
			RPrimitiveRenderProxy primitiveProxy;
			primitiveProxy.VertexBuffer = drawable->GetVertexBufferRaw();
			primitiveProxy.IndexBuffer = drawable->GetIndexBufferRaw();
			primitiveProxy.UniformBuffer = drawable->GetUniformBufferRaw();
			primitiveProxy.Material = drawable->GetMaterialRaw();
			primitiveProxy.Shader = primitiveProxy.Material->GetShaderRaw();
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

	void Scene::LoadSceneData(const RRendererData& data)
	{
		if (!m_ActiveCamera)
			return;

		m_RenderPrimitives = data.Primitives;
		m_RenderLights = data.Lights;

		if (data.DirectionalLight.Type != ELightType::Disabled)
		{
			m_RenderDirLight = data.DirectionalLight;
		}

		m_RenderAmbientLight = data.AmbientLightColor;

		m_ActiveCamera->CopyRenderData(m_RenderCamera);

		// Shrink arrays

		if (m_RenderPrimitives.capacity() > m_RenderPrimitives.size() * 2)
			m_RenderPrimitives.shrink_to_fit();

		if (m_RenderLights.capacity() > m_RenderLights.size() * 2)
			m_RenderLights.shrink_to_fit();
	}
}
