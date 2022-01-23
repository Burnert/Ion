#include "IonPCH.h"

#include "Renderer.h"

#include "RenderAPI/RenderAPI.h"
#include "RenderAPI/OpenGL/OpenGLRenderer.h"
#include "RenderAPI/DX11/DX11Renderer.h"

#include "Application/EnginePath.h"

namespace Ion
{
	TShared<Renderer> Renderer::Create()
	{
		switch (RenderAPI::GetCurrent())
		{
		case ERenderAPI::OpenGL:
			return MakeShared<OpenGLRenderer>();
		case ERenderAPI::DX11:
			return MakeShared<DX11Renderer>();
		default:
			return TShared<Renderer>(nullptr);
		}
	}

	void Renderer::CreateScreenTexturePrimitives()
	{
		// Setup quad shader

		String vertexSrc;
		String pixelSrc;
		File::ReadToString(EnginePath::GetCheckedShadersPath() + L"TextureRenderVS.hlsl", vertexSrc);
		File::ReadToString(EnginePath::GetCheckedShadersPath() + L"TextureRenderPS.hlsl", pixelSrc);

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
		CreateScreenTexturePrimitives();
	}

	void Renderer::BindScreenTexturePrimitives() const
	{
		m_ScreenTextureRenderData.Shader->Bind();
		m_ScreenTextureRenderData.VertexBuffer->Bind();
		m_ScreenTextureRenderData.VertexBuffer->BindLayout();
		m_ScreenTextureRenderData.IndexBuffer->Bind();
	}
}
