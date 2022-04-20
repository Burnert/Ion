#pragma once

#include "Material.h"
#include "Mesh.h"
#include "Light.h"
#include "Camera.h"
#include "Scene.h"
#include "RHI/VertexBuffer.h"
#include "RHI/IndexBuffer.h"
#include "RHI/UniformBuffer.h"
#include "RHI/Shader.h"
#include "RHI/Texture.h"

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

	struct RendererClearOptions
	{
		static inline const Vector4  DefaultClearColor   = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		static constexpr float       DefaultClearDepth   = 1.0f;
		static constexpr uint8       DefaultClearStencil = 0;

		Vector4 ClearColorValue;
		float ClearDepthValue;
		uint8 ClearStencilValue;

		uint8 bClearColor : 1;
		uint8 bClearDepth : 1;
		uint8 bClearStencil : 1;

		RendererClearOptions(
			const Vector4& color = DefaultClearColor,
			float depth          = DefaultClearDepth, 
			uint8 stencil        = DefaultClearStencil) :
			ClearColorValue(color),
			ClearDepthValue(depth),
			ClearStencilValue(stencil),
			bClearColor(true),
			bClearDepth(true),
			bClearStencil(true)
		{
		}
	};

	struct EditorPassData
	{
		/* UInt128GUID */
		TShared<Texture> RTObjectID;
		TShared<Texture> RTObjectIDDepth;
		/* Mainly Depth */
		TShared<Texture> RTSelectionDepth;

		TArray<REditorPassPrimitive> Primitives;
		TArray<REditorPassPrimitive> SelectedPrimitives;
		TArray<REditorPassBillboardPrimitive> Billboards;
		TArray<REditorPassBillboardPrimitive> SelectedBillboards;
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

		virtual void Init();

		void Clear() const;
		void RenderScene(const Scene* scene) const;

		void Draw(const RPrimitiveRenderProxy& primitive, const Scene* targetScene = nullptr) const;
		/** Draw a screen aligned quad positioned in world space. */
		void DrawBillboard(const RBillboardRenderProxy& billboard, const Shader* shader, const Scene* targetScene) const;
		void DrawScreenTexture(const TShared<Texture>& texture) const;
		void DrawScreenTexture(const TShared<Texture>& texture, const Shader* shader) const;

		virtual void Clear(const RendererClearOptions& options) const = 0;

		virtual void DrawIndexed(uint32 indexCount) const = 0;

		virtual void UnbindResources() const = 0;

		virtual void SetBlendingEnabled(bool bEnable) const = 0;

		virtual void SetVSyncEnabled(bool bEnabled) const = 0;
		virtual bool IsVSyncEnabled() const = 0;

		virtual void SetViewport(const ViewportDescription& viewport) = 0;
		virtual ViewportDescription GetViewport() const = 0;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const = 0;
		virtual EPolygonDrawMode GetPolygonDrawMode() const = 0;

		/* Sets the render target (resets the depth stencil to null) */
		virtual void SetRenderTarget(const TShared<Texture>& targetTexture) = 0;
		/* Sets the depth stencil target (call this only after SetRenderTarget) */
		virtual void SetDepthStencil(const TShared<Texture>& targetTexture) = 0;

		void RenderEditorPass(const Scene* scene, const EditorPassData& data);

		inline static const TShared<Shader>& GetBasicShader()
		{
			return Renderer::Get()->m_BasicShader;
		}

		inline static const TShared<Shader>& GetBasicUnlitMaskedShader()
		{
			return Renderer::Get()->m_BasicUnlitMaskedShader;
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

		inline static const TShared<Shader>& GetEditorViewportMSShader()
		{
			return Renderer::Get()->m_EditorViewportMSShader;
		}

		inline static const TShared<Shader>& GetFXAAShader()
		{
			return Renderer::Get()->m_PPFXAAShader;
		}

		inline static const TShared<Mesh>& GetBillboardMesh()
		{
			return Renderer::Get()->m_BillboardMesh;
		}

		inline static const TShared<Texture>& GetWhiteTexture()
		{
			return Renderer::Get()->m_WhiteTexture;
		}

	protected:
		Renderer() { }

		void InitUtilityPrimitives();

		void InitScreenTextureRendering();
		void BindScreenTexturePrimitives() const;
		void BindScreenTexturePrimitives(const Shader* customShader) const;

		void InitShaders();
		void InitBasicShader();
		void InitBasicUnlitMaskedShader();
		void InitFXAAShader();
		void InitEditorObjectIDShader();
		void InitEditorSelectedShader();
		void InitEditorViewportShader();
		void InitEditorViewportMSShader();

	private:
		void CreateScreenTexturePrimitives();

	private:
		ScreenTextureRenderData m_ScreenTextureRenderData;

		TShared<Shader> m_BasicShader;
		TShared<Shader> m_BasicUnlitMaskedShader;

		TShared<Shader> m_PPFXAAShader;

		TShared<Shader> m_EditorObjectIDShader;
		TShared<Shader> m_EditorSelectedShader;
		TShared<Shader> m_EditorViewportShader;
		TShared<Shader> m_EditorViewportMSShader;

		TShared<Mesh> m_BillboardMesh;

		TShared<Texture> m_WhiteTexture;

		static Renderer* s_Instance;
	};
}
