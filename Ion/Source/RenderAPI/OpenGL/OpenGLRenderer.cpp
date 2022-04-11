#include "IonPCH.h"

#include "OpenGLRenderer.h"

#include "OpenGLBuffer.h"
#include "OpenGLShader.h"
#include "OpenGLTexture.h"

namespace Ion
{
	OpenGLRenderer::OpenGLRenderer() :
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

	void OpenGLRenderer::Clear(const RendererClearOptions& options) const
	{
		TRACE_FUNCTION();

		if (options.bClearColor)
		{
			glClearColor(options.ClearColorValue.x, options.ClearColorValue.y,
				options.ClearColorValue.z, options.ClearColorValue.w);
		}
		if (options.bClearDepth)
		{
			glClearDepth(options.ClearDepthValue);
		}
		if (options.bClearStencil)
		{
			glClearStencil(options.ClearStencilValue);
		}

		glClear(
			FlagsIf(options.bClearColor,   GL_COLOR_BUFFER_BIT) |
			FlagsIf(options.bClearDepth,   GL_DEPTH_BUFFER_BIT) |
			FlagsIf(options.bClearStencil, GL_STENCIL_BUFFER_BIT)
		);
	}

	//void OpenGLRenderer::Draw(const RPrimitiveRenderProxy& primitive, const Scene* targetScene) const
	//{
	//	TRACE_FUNCTION();

	//	ionassert(targetScene);

	//	const Material* material = primitive.Material;
	//	const OpenGLVertexBuffer* vertexBuffer = (OpenGLVertexBuffer*)primitive.VertexBuffer;
	//	const OpenGLIndexBuffer* indexBuffer = (OpenGLIndexBuffer*)primitive.IndexBuffer;
	//	const OpenGLUniformBuffer* uniformBuffer = (OpenGLUniformBuffer*)primitive.UniformBuffer;
	//	const OpenGLShader* shader = (OpenGLShader*)primitive.Shader;

	//	vertexBuffer->Bind();
	//	vertexBuffer->BindLayout();
	//	indexBuffer->Bind();
	//	shader->Bind();

	//	if (material)
	//	{
	//		material->BindTextures();
	//		//material->UpdateShaderUniforms();
	//		material->ForEachTexture([shader](const TShared<Texture> texture, uint32 slot)
	//		{
	//			char uniformName[20];
	//			sprintf_s(uniformName, "g_Samplers[%u]", slot);
	//			shader->SetUniform1i(uniformName, slot);
	//		});
	//	}

	//	// Calculate the Model View Projection Matrix based on the current scene camera
	//	const RCameraRenderProxy& activeCamera = targetScene->GetCameraRenderProxy();

	//	const Matrix4& viewProjectionMatrix = activeCamera.ViewProjectionMatrix;
	//	const Matrix4& modelMatrix = primitive.Transform;

	//	MeshUniforms& uniformData = uniformBuffer->DataRef<MeshUniforms>();
	//	uniformData.TransformMatrix = modelMatrix;
	//	uniformData.InverseTransposeMatrix = Math::InverseTranspose(modelMatrix);
	//	uniformData.ModelViewProjectionMatrix = viewProjectionMatrix * modelMatrix;

	//	uniformBuffer->UpdateData();
	//	uniformBuffer->Bind(1);

	//	uint32 indexCount = indexBuffer->GetIndexCount();
	//}

	void OpenGLRenderer::DrawIndexed(uint32 indexCount) const
	{
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderer::RenderEditorViewport(const TShared<Texture>& sceneFinalTexture, const TShared<Texture>& editorDataTexture) const
	{
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
