#pragma once

#include "Core/Core.h"
#include "Camera.h"
#include "Light.h"

namespace Ion
{
	class IDrawable;
	class Camera;
	class Light;
	class DirectionalLight;

	class VertexBuffer;
	class IndexBuffer;
	class Material;
	class Shader;

	struct RVertexBufferProxy
	{
		const VertexBuffer* Ptr;
		uint32 RendererID;
	};

	struct RIndexBufferProxy
	{
		const IndexBuffer* Ptr;
		uint32 RendererID;
	};

	struct RMaterialProxy
	{
		const Material* Ptr;
		// Uniforms?
	};

	struct RShaderProxy
	{
		const Shader* Ptr;
		uint32 RendererID;
	};

	struct RPrimitiveRenderProxy
	{
		RVertexBufferProxy VertexBuffer;
		RIndexBufferProxy IndexBuffer;
		RMaterialProxy Material;
		RShaderProxy Shader;
		Matrix4 Transform;
	};

	struct RSceneProxy
	{
		TArray<RPrimitiveRenderProxy> RenderPrimitives;
		TArray<RLightRenderProxy> RenderLights;
		RLightRenderProxy RenderDirLight;
		RCameraRenderProxy RenderCamera;
		Vector4 AmbientLightColor;
		union
		{
			uint64 PackedFlags;
			struct
			{
				uint64 bHasDirLight : 1;
			};
		};
	};

	class ION_API Scene
	{
	public:
		const static uint32 MaxLights = 100;

		static TShared<Scene> Create();

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

		~Scene() { }

		// Render Thread related:

		void UpdateRenderData();
		void CopySceneData(RSceneProxy& outProxy) const;

		// Render Thread: --------------------------------------------------------------------------

		FORCEINLINE const TArray<RPrimitiveRenderProxy>& GetScenePrimitives() const { return m_RenderPrimitives; }
		FORCEINLINE const TArray<RLightRenderProxy>& GetRenderLights() const { return m_RenderLights; }
		FORCEINLINE const RLightRenderProxy& GetRenderDirLight() const { return m_RenderDirLight; }
		FORCEINLINE const RCameraRenderProxy& GetCameraRenderProxy() const { return m_RenderCamera; }
		
		FORCEINLINE bool HasDirectionalLight() const { return m_ActiveDirectionalLight; }

		// End of Render Thread --------------------------------------------------------------------------

	protected:
		Scene() :
			m_AmbientLightColor(0.0f),
			m_ActiveDirectionalLight(nullptr),
			m_RenderCamera({ }),
			m_RenderDirLight({ })
		{ }

	private:
		THashSet<IDrawable*> m_DrawableObjects;

		TShared<Camera> m_ActiveCamera;

		Vector4 m_AmbientLightColor;
		DirectionalLight* m_ActiveDirectionalLight;
		THashSet<Light*> m_Lights;

		// Render Thread: -------------------------------------

		TArray<RPrimitiveRenderProxy> m_RenderPrimitives;
		TArray<RLightRenderProxy> m_RenderLights;
		RLightRenderProxy m_RenderDirLight;
		RCameraRenderProxy m_RenderCamera;

		// End of Render Thread --------------------------------------------------------------------------
	};
}
