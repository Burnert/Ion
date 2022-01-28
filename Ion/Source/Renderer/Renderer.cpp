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
		TRACE_FUNCTION();

		m_ScreenTextureRenderData.Shader->Bind();
		m_ScreenTextureRenderData.VertexBuffer->Bind();
		m_ScreenTextureRenderData.VertexBuffer->BindLayout();
		m_ScreenTextureRenderData.IndexBuffer->Bind();
	}

	void Renderer::InitShaders()
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

	Renderer* Renderer::s_Instance;
}
