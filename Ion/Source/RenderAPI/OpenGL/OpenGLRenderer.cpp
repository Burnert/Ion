#include "IonPCH.h"

#include "OpenGLRenderer.h"

#include "OpenGLVertexBuffer.h"
#include "OpenGLIndexBuffer.h"

namespace Ion
{
	OpenGLRenderer::OpenGLRenderer()
		: m_CurrentScene({ })
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
		uint vao;
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	void OpenGLRenderer::Clear() const
	{
		Clear(FVector4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void OpenGLRenderer::Clear(const FVector4& color) const
	{
		TRACE_FUNCTION();

		glClearColor(color.x, color.y, color.z, color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderer::Draw(const TShared<IDrawable>& drawable, const TShared<Scene>& targetScene) const
	{
		TRACE_FUNCTION();

		TShared<Scene> scene;
		if (targetScene)
		{
			scene = targetScene;
		}
		else
		{
			ionassert(m_CurrentScene, "Cannot render before setting the current scene if the target scene is not specified.");
			scene = m_CurrentScene;
		}

		TShared<OpenGLVertexBuffer> vertexBuffer = std::static_pointer_cast<OpenGLVertexBuffer>(drawable->GetVertexBuffer());
		TShared<OpenGLIndexBuffer> indexBuffer = std::static_pointer_cast<OpenGLIndexBuffer>(drawable->GetIndexBuffer());
		TShared<Material> material = drawable->GetMaterial();

		vertexBuffer->Bind();
		vertexBuffer->BindLayout();

		indexBuffer->Bind();

		TShared<Shader> shader = material->GetShader();
		shader->Bind();

		material->UpdateShaderUniforms();

		// Calculate the Model View Projection Matrix based on the current scene camera
		TShared<Camera> activeCamera = m_CurrentScene->GetActiveCamera();
		ionassert(activeCamera, "Cannot render without an active camera.");
		activeCamera->UpdateViewProjectionMatrix();

		shader->SetUniform3f("u_CameraLocation", activeCamera->GetLocation());

		// Set matrices

		const FMatrix4& modelMatrix = drawable->GetTransformMatrix();
		shader->SetUniformMatrix4f("u_Transform", modelMatrix);

		const FMatrix4 inverseTranspose = glm::inverseTranspose(modelMatrix);
		shader->SetUniformMatrix4f("u_InverseTranspose", inverseTranspose);

		const FMatrix4& viewProjectionMatrix = activeCamera->GetViewProjectionMatrix();
		const FMatrix4 modelViewProjectionMatrix = viewProjectionMatrix * modelMatrix;
		shader->SetUniformMatrix4f("u_MVP", modelViewProjectionMatrix);

		uint indexCount = indexBuffer->GetIndexCount();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderer::RenderScene(const TShared<Scene>& scene)
	{
		TRACE_FUNCTION();

		m_CurrentScene = scene;
		for (TShared<IDrawable> drawable : scene->GetDrawableObjects())
		{
			Draw(drawable, scene);
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
		OpenGL::SetSwapInterval((int)bEnabled);
	}

	bool OpenGLRenderer::IsVSyncEnabled() const
	{
		return (bool)OpenGL::GetSwapInterval();
	}

	void OpenGLRenderer::SetViewportDimensions(const SViewportDimensions& dimensions) const
	{
		TRACE_FUNCTION();

		glViewport(dimensions.X, dimensions.Y, dimensions.Width, dimensions.Height);
	}

	SViewportDimensions OpenGLRenderer::GetViewportDimensions() const
	{
		TRACE_FUNCTION();

		SViewportDimensions dimensions;
		glGetIntegerv(GL_VIEWPORT, (int*)&dimensions);
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

		int polygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
		return GLPolygonModeToPolygonDrawMode(polygonMode);
	}
}
