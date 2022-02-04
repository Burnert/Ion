#pragma once

#include "Core/Core.h"
#include "RendererCore.h"
#include "Camera.h"
#include "Light.h"

#define MAX_LIGHTS 100

namespace Ion
{
	struct UNIFORMBUFFER SceneUniforms
	{
		Matrix4 ViewMatrix;
		Matrix4 ProjectionMatrix;
		Matrix4 ViewProjectionMatrix;
		
		LightUniforms Lights[MAX_LIGHTS];
		LightUniforms DirLight;
		Vector4 AmbientLightColor;

		Vector3 CameraLocation;

		uint32 LightNum;
	};

	struct RPrimitiveRenderProxy
	{
		const VertexBuffer* VertexBuffer;
		const IndexBuffer* IndexBuffer;
		const UniformBuffer* UniformBuffer;
		const Material* Material;
		const Shader* Shader;
		Matrix4 Transform;
	};

	class ION_API Scene
	{
	public:
		const static uint32 MaxLights = 100;

		void SetActiveCamera(const TShared<Camera>& camera);
		FORCEINLINE const TShared<Camera>& GetActiveCamera() const { return m_ActiveCamera; }

		void SetAmbientLightColor(const Vector4& color);
		inline const Vector4& GetAmbientLightColor() const { return m_AmbientLightColor; }

		void SetActiveDirectionalLight(DirectionalLight* light);
		inline const DirectionalLight* GetActiveDirectionalLight() const { return m_ActiveDirectionalLight; }

		void AddLight(Light* light);
		bool RemoveLight(Light* light);
		inline const THashSet<Light*>& GetLights() const { return m_Lights; }
		inline uint32 GetLightNumber() const { return (uint32)m_Lights.size(); }

		// @TODO: For now, adding object to scene is going to be done this way.
		// Later, the World is going to pass renderable objects to the scene automatically.
		void AddDrawableObject(IDrawable* drawable);
		bool RemoveDrawableObject(IDrawable* drawable);
		FORCEINLINE const THashSet<IDrawable*>& GetDrawableObjects() const { return m_DrawableObjects; }

		Scene();
		~Scene() { }

		void UpdateRenderData();
		void LoadSceneData(const RRendererData& data);

		// Render Thread: --------------------------------------------------------------------------

		FORCEINLINE const TArray<RPrimitiveRenderProxy>& GetScenePrimitives() const { return m_RenderPrimitives; }
		FORCEINLINE const TArray<RLightRenderProxy>& GetRenderLights() const { return m_RenderLights; }
		FORCEINLINE const RLightRenderProxy& GetRenderDirLight() const { return m_RenderDirLight; }
		FORCEINLINE const RCameraRenderProxy& GetCameraRenderProxy() const { return m_RenderCamera; }
		
		FORCEINLINE bool HasDirectionalLight() const { return m_ActiveDirectionalLight; }

	private:
		World* m_OwningWorld;

		THashSet<IDrawable*> m_DrawableObjects;

		TShared<Camera> m_ActiveCamera;

		Vector4 m_AmbientLightColor;
		DirectionalLight* m_ActiveDirectionalLight;
		THashSet<Light*> m_Lights;

		TShared<UniformBuffer> m_SceneUniformBuffer;

		// Render Thread: -------------------------------------

		TArray<RPrimitiveRenderProxy> m_RenderPrimitives;
		TArray<RLightRenderProxy> m_RenderLights;
		RLightRenderProxy m_RenderDirLight;
		Vector4 m_RenderAmbientLight;
		RCameraRenderProxy m_RenderCamera;

		friend class Renderer;
		friend class OpenGLRenderer;
		friend class DX11Renderer;
		friend class World;
	};
}
