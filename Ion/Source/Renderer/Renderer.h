#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UniformBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "Light.h"
#include "Camera.h"
#include "Scene.h"

#include "Drawable.h"

namespace Ion
{
	struct RRendererData
	{
		TArray<RPrimitiveRenderProxy> Primitives;
		TArray<RLightRenderProxy> Lights;
		RLightRenderProxy DirectionalLight;
		Vector4 AmbientLightColor;

		inline void AddLight(RLightRenderProxy& light)
		{
			Lights.push_back(light);
		}

		inline void AddPrimitive(RPrimitiveRenderProxy& primitive)
		{
			Primitives.push_back(primitive);
		}
	};

	struct ScreenTextureRenderData
	{
		TShared<Shader> Shader;
		TShared<VertexBuffer> VertexBuffer;
		TShared<IndexBuffer> IndexBuffer;
	};

	struct REditorPassPrimitive
	{
		GUID Guid = GUID::Zero;
		const VertexBuffer* VertexBuffer;
		const IndexBuffer* IndexBuffer;
		const UniformBuffer* UniformBuffer;
		Matrix4 Transform;
	};

	struct EditorPassData
	{
		/* UInt128GUID */
		TShared<Texture> RTObjectID;
		/* Mainly Depth */
		TShared<Texture> RTSelection;

		TArray<REditorPassPrimitive> Primitives;
		TArray<REditorPassPrimitive> SelectedPrimitives;
	};

	class ION_API Renderer
	{
	public:
		static Renderer* Create();
		inline static Renderer* Get()
		{
			ionassert(s_Instance);
			return s_Instance;
		}

		virtual ~Renderer() { };

		virtual void Init() = 0;

		virtual void Clear() const = 0;
		virtual void Clear(const Vector4& color) const = 0;

		virtual void Draw(const RPrimitiveRenderProxy& primitive, const Scene* targetScene = nullptr) const = 0;
		virtual void DrawScreenTexture(const TShared<Texture>& texture) const = 0;
		virtual void DrawEditorViewport(const TShared<Texture>& sceneFinalTexture, const TShared<Texture>& editorDataTexture) const = 0; // @TODO: this is a bad idea

		virtual void SetCurrentScene(const Scene* scene) = 0;
		virtual const Scene* GetCurrentScene() const = 0;

		virtual void RenderScene(const Scene* scene) = 0;

		virtual void SetVSyncEnabled(bool bEnabled) const = 0;
		virtual bool IsVSyncEnabled() const = 0;

		virtual void SetViewportDimensions(const ViewportDimensions& dimensions) const = 0;
		virtual ViewportDimensions GetViewportDimensions() const = 0;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const = 0;
		virtual EPolygonDrawMode GetPolygonDrawMode() const = 0;

		virtual void SetRenderTarget(const TShared<Texture>& targetTexture) = 0;

		void RenderEditorPass(const Scene* scene, const EditorPassData& data);

		inline static const TShared<Shader>& GetBasicShader()
		{
			return Renderer::Get()->m_BasicShader;
		}

		inline static const TShared<Shader>& GetEditorObjectIDShader()
		{
			return Renderer::Get()->m_EditorObjectIDShader;
		}

		inline static const TShared<Shader>& GetEditorSelectedShader()
		{
			return Renderer::Get()->m_EditorSelectedShader;
		}

		inline static const TShared<Shader>& GetEditorViewportShader()
		{
			return Renderer::Get()->m_EditorViewportShader;
		}

	protected:
		Renderer() { }

		void InitScreenTextureRendering();
		void BindScreenTexturePrimitives() const;
		void BindScreenTexturePrimitives(Shader* customShader) const;

		void InitShaders();
		void InitBasicShader();
		void InitEditorObjectIDShader();
		void InitEditorSelectedShader();
		void InitEditorViewportShader();

	private:
		void CreateScreenTexturePrimitives();

	private:
		ScreenTextureRenderData m_ScreenTextureRenderData;
		
		TShared<Shader> m_BasicShader;

		TShared<Shader> m_EditorObjectIDShader;
		TShared<Shader> m_EditorSelectedShader;
		TShared<Shader> m_EditorViewportShader;

		static Renderer* s_Instance;
	};
}
