#include "IonPCH.h"

#include "Renderer.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLRenderer.h"
#include "RenderAPI/DX11/DX11Renderer.h"

#include "Application/EnginePath.h"

namespace Ion
{
	Renderer* Renderer::Create()
	{
		TRACE_FUNCTION();

		ionassert(!s_Instance);

		switch (RenderAPI::GetCurrent())
		{
			case ERenderAPI::OpenGL:
			{
				s_Instance = new OpenGLRenderer;
				break;
			}
			case ERenderAPI::DX11:
			{
				s_Instance = new DX11Renderer;
				break;
			}
			default:
			{
				s_Instance = nullptr;
				break;
			}
		}
		return s_Instance;
	}

	void Renderer::Clear() const
	{
		Clear(RendererClearOptions());
	}

	void Renderer::CreateScreenTexturePrimitives()
	{
		TRACE_FUNCTION();

		// Setup quad shader

		String vertexSrc;
		String pixelSrc;
		if (RenderAPI::GetCurrent() == ERenderAPI::DX11)
		{
			File::ReadToString(EnginePath::GetCheckedShadersPath() + L"TextureRenderVS.hlsl", vertexSrc);
			File::ReadToString(EnginePath::GetCheckedShadersPath() + L"TextureRenderPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(EnginePath::GetCheckedShadersPath() + L"TextureRender.vert", vertexSrc);
			File::ReadToString(EnginePath::GetCheckedShadersPath() + L"TextureRender.frag", pixelSrc);
		}

		m_ScreenTextureRenderData.Shader = Shader::Create();
		m_ScreenTextureRenderData.Shader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_ScreenTextureRenderData.Shader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		bool bResult = m_ScreenTextureRenderData.Shader->Compile();
		ionassert(bResult);

		// Setup quad buffers

		float quadVertices[] = {
			-1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		};

		uint32 quadIndices[] = {
			0, 2, 1,
			2, 0, 3,
		};

		TShared<VertexLayout> quadLayout = MakeShareable(new VertexLayout(2));
		quadLayout->AddAttribute(EVertexAttributeSemantic::Position, EVertexAttributeType::Float, 3, false);
		quadLayout->AddAttribute(EVertexAttributeSemantic::TexCoord, EVertexAttributeType::Float, 2, false);

		m_ScreenTextureRenderData.VertexBuffer = VertexBuffer::Create(quadVertices, sizeof(quadVertices) / sizeof(float));
		m_ScreenTextureRenderData.VertexBuffer->SetLayout(quadLayout);
		m_ScreenTextureRenderData.VertexBuffer->SetLayoutShader(m_ScreenTextureRenderData.Shader);

		m_ScreenTextureRenderData.IndexBuffer = IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32));
	}

	void Renderer::InitScreenTextureRendering()
	{
		TRACE_FUNCTION();

		CreateScreenTexturePrimitives();
	}

	void Renderer::BindScreenTexturePrimitives() const
	{
		BindScreenTexturePrimitives(m_ScreenTextureRenderData.Shader.get());
	}

	void Renderer::BindScreenTexturePrimitives(Shader* customShader) const
	{
		TRACE_FUNCTION();

		ionassert(customShader);

		customShader->Bind();
		m_ScreenTextureRenderData.VertexBuffer->Bind();
		m_ScreenTextureRenderData.VertexBuffer->BindLayout();
		m_ScreenTextureRenderData.IndexBuffer->Bind();
	}

	void Renderer::InitShaders()
	{
		InitBasicShader();
		InitEditorObjectIDShader();
		InitEditorSelectedShader();
		InitEditorViewportShader();
	}

	void Renderer::InitBasicShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

		// @TODO: This needs a refactor
		if (RenderAPI::GetCurrent() == ERenderAPI::DX11)
		{
			File::ReadToString(shadersPath + L"BasicVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"BasicPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Basic.vert", vertexSrc);
			File::ReadToString(shadersPath + L"Basic.frag", pixelSrc);
		}

		m_BasicShader = Shader::Create();
		m_BasicShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_BasicShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		if (!m_BasicShader->Compile())
		{
			LOG_ERROR("Could not compile the Basic Shader.");
			debugbreak();
		}
	}

	void Renderer::InitEditorObjectIDShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

		// @TODO: This needs a refactor
		if (RenderAPI::GetCurrent() == ERenderAPI::DX11)
		{
			File::ReadToString(shadersPath + L"Editor/EditorObjectIDVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorObjectIDPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Editor/EditorObjectID.vert", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorObjectID.frag", pixelSrc);
		}

		m_EditorObjectIDShader = Shader::Create();
		m_EditorObjectIDShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_EditorObjectIDShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		if (!m_EditorObjectIDShader->Compile())
		{
			LOG_ERROR("Could not compile the EditorObjectID Shader.");
			debugbreak();
		}
	}

	void Renderer::InitEditorSelectedShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

		// @TODO: This needs a refactor
		if (RenderAPI::GetCurrent() == ERenderAPI::DX11)
		{
			File::ReadToString(shadersPath + L"Editor/EditorSelectedVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorSelectedPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Editor/EditorSelected.vert", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorSelected.frag", pixelSrc);
		}

		m_EditorSelectedShader = Shader::Create();
		m_EditorSelectedShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_EditorSelectedShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		if (!m_EditorSelectedShader->Compile())
		{
			LOG_ERROR("Could not compile the EditorSelected Shader.");
			debugbreak();
		}
	}

	void Renderer::InitEditorViewportShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

		// @TODO: This needs a refactor
		if (RenderAPI::GetCurrent() == ERenderAPI::DX11)
		{
			File::ReadToString(shadersPath + L"Editor/EditorViewportVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorViewportPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Editor/EditorViewport.vert", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorViewport.frag", pixelSrc);
		}

		m_EditorViewportShader = Shader::Create();
		m_EditorViewportShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_EditorViewportShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		if (!m_EditorViewportShader->Compile())
		{
			LOG_ERROR("Could not compile the EditorViewport Shader.");
			debugbreak();
		}
	}

	void Renderer::RenderEditorPass(const Scene* scene, const EditorPassData& data)
	{
		ionassert(scene);

		if (!data.RTObjectID || !data.RTSelection || data.Primitives.empty())
			return;

		// Set the alpha channel to 0
		RendererClearOptions clearOptions { };
		clearOptions.ClearColorValue = Vector4(0.0f, 0.0f, 0.0f, 0.0f);

		SetRenderTarget(data.RTObjectID);
		Clear(clearOptions);

		SceneUniforms& uniforms = scene->m_SceneUniformBuffer->DataRef<SceneUniforms>();
		uniforms.ViewMatrix = scene->m_RenderCamera.ViewMatrix;
		uniforms.ProjectionMatrix = scene->m_RenderCamera.ProjectionMatrix;
		uniforms.ViewProjectionMatrix = scene->m_RenderCamera.ViewProjectionMatrix;
		uniforms.CameraLocation = scene->m_RenderCamera.Location;
		scene->m_SceneUniformBuffer->UpdateData();
		scene->m_SceneUniformBuffer->Bind(0);

		for (const REditorPassPrimitive& editorPrim : data.Primitives)
		{
			RPrimitiveRenderProxy prim { };
			prim.VertexBuffer = editorPrim.VertexBuffer;
			prim.IndexBuffer = editorPrim.IndexBuffer;
			prim.UniformBuffer = editorPrim.UniformBuffer;
			prim.Material = nullptr;
			prim.Shader = GetEditorObjectIDShader().get();
			prim.Transform = editorPrim.Transform;

			// Load the GUID
			MeshUniforms& meshUniforms = prim.UniformBuffer->DataRef<MeshUniforms>();
			meshUniforms.RenderGuid = *(UVector4*)editorPrim.Guid.GetRawBytes().data();

			Draw(prim, scene);

			// Reset
			meshUniforms.RenderGuid = UVector4();
		}

		SetRenderTarget(data.RTSelection);
		Clear(clearOptions);

		if (!data.SelectedPrimitives.empty())
		{
			for (const REditorPassPrimitive& editorPrim : data.SelectedPrimitives)
			{
				RPrimitiveRenderProxy prim { };
				prim.VertexBuffer = editorPrim.VertexBuffer;
				prim.IndexBuffer = editorPrim.IndexBuffer;
				prim.UniformBuffer = editorPrim.UniformBuffer;
				prim.Material = nullptr;
				prim.Shader = GetEditorSelectedShader().get();
				prim.Transform = editorPrim.Transform;

				Draw(prim, scene);
			}
		}
	}

	Renderer* Renderer::s_Instance;
}
