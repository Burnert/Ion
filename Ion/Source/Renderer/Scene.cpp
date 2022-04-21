#include "IonPCH.h"

#include "Scene.h"
#include "Material.h"
#include "Light.h"
#include "RHI/UniformBuffer.h"
#include "Renderer/Renderer.h"

namespace Ion
{
	Scene::Scene() :
		m_AmbientLightColor(0.0f),
		m_ActiveDirectionalLight(nullptr),
		m_RenderCamera({ }),
		m_RenderDirLight({ })
	{
		m_SceneUniformBuffer = MakeShareable(RHIUniformBuffer::Create(SceneUniforms()));
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

	void Scene::LoadSceneData(const RRendererData& data)
	{
		m_RenderPrimitives = data.Primitives;
		m_RenderLights = data.Lights;
		m_RenderDirLight = data.DirectionalLight;
		m_RenderAmbientLight = data.AmbientLightColor;

		if (m_ActiveCamera)
			m_ActiveCamera->CopyRenderData(m_RenderCamera);

		// Shrink arrays

		if (m_RenderPrimitives.capacity() > m_RenderPrimitives.size() * 2)
			m_RenderPrimitives.shrink_to_fit();

		if (m_RenderLights.capacity() > m_RenderLights.size() * 2)
			m_RenderLights.shrink_to_fit();
	}

	void Scene::LoadCamera(const TShared<Camera>& camera)
	{
		camera->CopyRenderData(m_RenderCamera);
	}
}
