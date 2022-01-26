#include "IonPCH.h"

#include "OpenGLRenderer.h"

#include "OpenGLBuffer.h"
#include "OpenGLShader.h"
#include "OpenGLTexture.h"

namespace Ion
{
	OpenGLRenderer::OpenGLRenderer()
		: m_CurrentScene({ }),
		m_CurrentRenderTarget(0)
	{ }

	OpenGLRenderer::~OpenGLRenderer()
	{
	}

	void OpenGLRenderer::Init()
	{
		TRACE_FUNCTION();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		// Bind the default VAO and forget about it for the rest of the world.
		// Maybe I'll implement it in some way in the future but I don't think it's necessary.
		// @TODO: Think about it!
		uint32 vao;
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);

		InitScreenTextureRendering();
	}

	void OpenGLRenderer::Clear() const
	{
		Clear(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void OpenGLRenderer::Clear(const Vector4& color) const
	{
		TRACE_FUNCTION();

		glClearColor(color.x, color.y, color.z, color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderer::Draw(const RPrimitiveRenderProxy& primitive, const TShared<Scene>& targetScene) const
	{
		TRACE_FUNCTION();

		ionassert(targetScene);

		const Material* material = primitive.Material;
		const OpenGLVertexBuffer* vertexBuffer = (OpenGLVertexBuffer*)primitive.VertexBuffer;
		const OpenGLIndexBuffer* indexBuffer = (OpenGLIndexBuffer*)primitive.IndexBuffer;
		const OpenGLUniformBuffer* uniformBuffer = (OpenGLUniformBuffer*)primitive.UniformBuffer;
		const OpenGLShader* shader = (OpenGLShader*)primitive.Shader;

		vertexBuffer->Bind();
		vertexBuffer->BindLayout();
		indexBuffer->Bind();
		shader->Bind();

		if (material)
		{
			material->BindTextures();
			//material->UpdateShaderUniforms();
			material->ForEachTexture([shader](const TShared<Texture> texture, uint32 slot)
			{
				char uniformName[20];
				sprintf_s(uniformName, "g_Samplers[%u]", slot);
				shader->SetUniform1i(uniformName, slot);
			});
		}

		// Calculate the Model View Projection Matrix based on the current scene camera
		const RCameraRenderProxy& activeCamera = targetScene->GetCameraRenderProxy();

		const Matrix4& viewProjectionMatrix = activeCamera.ViewProjectionMatrix;
		const Matrix4& modelMatrix = primitive.Transform;

		MeshUniforms& uniformData = uniformBuffer->DataRef<MeshUniforms>();
		uniformData.TransformMatrix = modelMatrix;
		uniformData.InverseTransposeMatrix = Math::InverseTranspose(modelMatrix);
		uniformData.ModelViewProjectionMatrix = viewProjectionMatrix * modelMatrix;

		uniformBuffer->UpdateData();
		uniformBuffer->Bind(1);

		uint32 indexCount = indexBuffer->GetIndexCount();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderer::DrawScreenTexture(const TShared<Texture>& texture) const
	{
		TRACE_FUNCTION();

		BindScreenTexturePrimitives();
		texture->Bind(0);

		// Index count is always 6 (2 triangles)
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderer::RenderScene(const TShared<Scene>& scene)
	{
		TRACE_FUNCTION();

		m_CurrentScene = scene;

		// Set global matrices and other scene data
		SceneUniforms& uniforms = scene->m_SceneUniformBuffer->DataRef<SceneUniforms>();
		uniforms.ViewMatrix = scene->m_RenderCamera.ViewMatrix;
		uniforms.ProjectionMatrix = scene->m_RenderCamera.ProjectionMatrix;
		uniforms.ViewProjectionMatrix = scene->m_RenderCamera.ViewProjectionMatrix;
		uniforms.CameraLocation = scene->m_RenderCamera.Location;

		// Set lights
		const RLightRenderProxy& dirLight = scene->GetRenderDirLight();
		const TArray<RLightRenderProxy>& lights = scene->GetRenderLights();
		uint32 lightNum = scene->GetLightNumber();

		if (scene->HasDirectionalLight())
		{
			uniforms.DirLight.Direction = Vector4(dirLight.Direction, 0.0f);
			uniforms.DirLight.Color = Vector4(dirLight.Color, 1.0f);
			uniforms.DirLight.Intensity = dirLight.Intensity;
		}
		else
		{
			uniforms.DirLight.Intensity = 0.0f;
		}

		uniforms.AmbientLightColor = scene->GetAmbientLightColor();
		uniforms.LightNum = lightNum;

		uint32 lightIndex = 0;
		for (const RLightRenderProxy& light : lights)
		{
			LightUniforms& lightUniforms = uniforms.Lights[lightIndex];

			lightUniforms.Location = Vector4(light.Location, 1.0f);
			lightUniforms.Color = Vector4(light.Color, 1.0f);
			lightUniforms.Intensity = light.Intensity;
			lightUniforms.Falloff = light.Falloff;

			lightIndex++;
		}

		// Update Constant Buffers
		scene->m_SceneUniformBuffer->UpdateData();
		scene->m_SceneUniformBuffer->Bind(0);

		for (const RPrimitiveRenderProxy& primitive : scene->GetScenePrimitives())
		{
			Draw(primitive, scene);
		}
	}

	void OpenGLRenderer::SetCurrentScene(const TShared<Scene>& scene)
	{
		m_CurrentScene = scene;
	}

	const TShared<Scene>& OpenGLRenderer::GetCurrentScene() const
	{
		return m_CurrentScene;
	}

	void OpenGLRenderer::SetVSyncEnabled(bool bEnabled) const
	{
		OpenGL::SetSwapInterval((int32)bEnabled);
	}

	bool OpenGLRenderer::IsVSyncEnabled() const
	{
		return (bool)OpenGL::GetSwapInterval();
	}

	void OpenGLRenderer::SetViewportDimensions(const ViewportDimensions& dimensions) const
	{
		TRACE_FUNCTION();

		glViewport(dimensions.X, dimensions.Y, dimensions.Width, dimensions.Height);
	}

	ViewportDimensions OpenGLRenderer::GetViewportDimensions() const
	{
		TRACE_FUNCTION();

		ViewportDimensions dimensions;
		glGetIntegerv(GL_VIEWPORT, (int32*)&dimensions);
		return dimensions;
	}

	void OpenGLRenderer::SetPolygonDrawMode(EPolygonDrawMode drawMode) const
	{
		TRACE_FUNCTION();

		glPolygonMode(GL_FRONT_AND_BACK, PolygonDrawModeToGLPolygonMode(drawMode));

		switch (drawMode)
		{
		case Ion::EPolygonDrawMode::Fill:
			glEnable(GL_CULL_FACE);
			break;
		case Ion::EPolygonDrawMode::Lines:
		case Ion::EPolygonDrawMode::Points:
			glDisable(GL_CULL_FACE);
			break;
		}
	}

	EPolygonDrawMode OpenGLRenderer::GetPolygonDrawMode() const
	{
		TRACE_FUNCTION();

		int32 polygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
		return GLPolygonModeToPolygonDrawMode(polygonMode);
	}

	void OpenGLRenderer::SetRenderTarget(const TShared<Texture>& targetTexture)
	{
		ionassert(!targetTexture || targetTexture->GetDescription().bUseAsRenderTarget);

		m_CurrentRenderTarget = targetTexture ?
			((OpenGLTexture*)targetTexture.get())->m_FramebufferID : 0;
		glBindFramebuffer(GL_FRAMEBUFFER, m_CurrentRenderTarget);
	}
}
