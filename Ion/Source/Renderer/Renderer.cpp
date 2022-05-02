#include "IonPCH.h"

#include "Renderer.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/OpenGLRenderer.h"
#include "RHI/DX11/DX11Renderer.h"

#include "Application/EnginePath.h"

namespace Ion
{
	Renderer* Renderer::Create()
	{
		TRACE_FUNCTION();

		ionassert(!s_Instance);

		switch (RHI::GetCurrent())
		{
			case ERHI::OpenGL:
			{
				s_Instance = new OpenGLRenderer;
				break;
			}
			case ERHI::DX11:
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

	void Renderer::Init()
	{
		InitScreenTextureRendering();
		InitShaders();
		InitUtilityPrimitives();
	}

	void Renderer::Clear() const
	{
		Clear(RendererClearOptions());
	}

	void Renderer::RenderScene(const Scene* scene) const
	{
		TRACE_FUNCTION();

		// Set global matrices and other scene data
		SceneUniforms& uniforms = scene->m_SceneUniformBuffer->DataRef<SceneUniforms>();
		uniforms.ViewMatrix = scene->m_RenderCamera.ViewMatrix;
		uniforms.ProjectionMatrix = scene->m_RenderCamera.ProjectionMatrix;
		uniforms.ViewProjectionMatrix = scene->m_RenderCamera.ViewProjectionMatrix;
		uniforms.CameraLocation = scene->m_RenderCamera.Location;

		// Set lights
		const RLightRenderProxy& dirLight = scene->GetRenderDirLight();
		const TArray<RLightRenderProxy>& lights = scene->GetRenderLights();
		uint32 lightNum = (int32)scene->GetRenderLights().size();

		if (dirLight.IsEnabled())
		{
			uniforms.DirLight.Direction = Vector4(dirLight.Direction, 0.0f);
			uniforms.DirLight.Color = Vector4(dirLight.Color, 1.0f);
			uniforms.DirLight.Intensity = dirLight.Intensity;
		}
		else
		{
			uniforms.DirLight.Intensity = 0.0f;
		}

		uniforms.AmbientLightColor = scene->m_RenderAmbientLight;
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

		{
			TRACE_SCOPE("Renderer::RenderScene - Draw Primitives");
			for (const RPrimitiveRenderProxy& primitive : scene->GetScenePrimitives())
			{
				Draw(primitive, scene);
			}
		}
	}

	void Renderer::Draw(const RPrimitiveRenderProxy& primitive, const Scene* targetScene) const
	{
		TRACE_FUNCTION();

		ionassert(targetScene);

		primitive.Shader->Bind();
		primitive.VertexBuffer->Bind();
		primitive.VertexBuffer->BindLayout();
		primitive.IndexBuffer->Bind();

		const Matrix4& viewProjectionMatrix = targetScene->m_RenderCamera.ViewProjectionMatrix;
		const Matrix4& modelMatrix = primitive.Transform;

		MeshUniforms& uniformData = primitive.UniformBuffer->DataRef<MeshUniforms>();
		uniformData.TransformMatrix = modelMatrix;
		uniformData.InverseTransposeMatrix = Math::InverseTranspose(modelMatrix);
		uniformData.ModelViewProjectionMatrix = viewProjectionMatrix * modelMatrix;

		primitive.UniformBuffer->UpdateData();
		primitive.UniformBuffer->Bind(1);

		const Material* material = primitive.Material;

		if (material)
			material->BindTextures();
		//material->UpdateShaderUniforms();

		DrawIndexed(primitive.IndexBuffer->GetIndexCount());
	}

	void Renderer::DrawBillboard(const RBillboardRenderProxy& billboard, const RHIShader* shader, const Scene* targetScene) const
	{
		TRACE_FUNCTION();

		ionassert(targetScene);

		TShared<Mesh> billboardMesh = GetBillboardMesh();
		ionassert(billboardMesh);

		RHIVertexBuffer* vb = billboardMesh->GetVertexBuffer();
		RHIIndexBuffer* ib = billboardMesh->GetIndexBuffer();

		shader->Bind();
		vb->Bind();
		vb->BindLayout();
		ib->Bind();

		// @TODO: All this cannot be done for each billbord to be drawn

		Transform cameraTransform = Transform(targetScene->GetCameraRenderProxy().Transform);

		Transform transform = Transform(billboard.LocationWS);
		transform.SetScale(Vector3(billboard.Scale));
		transform.SetRotation(cameraTransform.GetRotation());

		MeshUniforms& uniforms = billboardMesh->GetUniformsDataRef();
		uniforms.TransformMatrix = transform.GetMatrix();
		uniforms.ModelViewProjectionMatrix = targetScene->m_RenderCamera.ViewProjectionMatrix * transform.GetMatrix();

		billboardMesh->GetUniformBufferRaw()->UpdateData();
		billboardMesh->GetUniformBufferRaw()->Bind(1);

		billboard.Texture->Bind(0);

		DrawIndexed(ib->GetIndexCount());
	}

	void Renderer::DrawScreenTexture(const TShared<RHITexture>& texture) const
	{
		TRACE_FUNCTION();

		BindScreenTexturePrimitives();
		texture->Bind(0);

		// Index count is always 6 (2 triangles)
		DrawIndexed(6);
	}

	void Renderer::DrawScreenTexture(const TShared<RHITexture>& texture, const RHIShader* shader) const
	{
		TRACE_FUNCTION();

		BindScreenTexturePrimitives(shader);
		texture->Bind(0);

		// Index count is always 6 (2 triangles)
		DrawIndexed(6);
	}

	void Renderer::CreateScreenTexturePrimitives()
	{
		TRACE_FUNCTION();

		// Setup quad shader

		String vertexSrc;
		String pixelSrc;
		if (RHI::GetCurrent() == ERHI::DX11)
		{
			File::ReadToString(EnginePath::GetCheckedShadersPath() + L"TextureRenderVS.hlsl", vertexSrc);
			File::ReadToString(EnginePath::GetCheckedShadersPath() + L"TextureRenderPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(EnginePath::GetCheckedShadersPath() + L"TextureRender.vert", vertexSrc);
			File::ReadToString(EnginePath::GetCheckedShadersPath() + L"TextureRender.frag", pixelSrc);
		}

		m_ScreenTextureRenderData.Shader = RHIShader::Create();
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

		TShared<RHIVertexLayout> quadLayout = MakeShared<RHIVertexLayout>(2);
		quadLayout->AddAttribute(EVertexAttributeSemantic::Position, EVertexAttributeType::Float, 3, false);
		quadLayout->AddAttribute(EVertexAttributeSemantic::TexCoord, EVertexAttributeType::Float, 2, false);

		m_ScreenTextureRenderData.VertexBuffer = RHIVertexBuffer::Create(quadVertices, sizeof(quadVertices) / sizeof(float));
		m_ScreenTextureRenderData.VertexBuffer->SetLayout(quadLayout);
		m_ScreenTextureRenderData.VertexBuffer->SetLayoutShader(m_ScreenTextureRenderData.Shader);

		m_ScreenTextureRenderData.IndexBuffer = RHIIndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32));
	}

	void Renderer::InitUtilityPrimitives()
	{
		// Quad mesh

		float quadVertices[] = {
		/*   location           texcoord    normal       */
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		};

		uint32 quadIndices[] = {
			0, 2, 1,
			2, 0, 3,
		};

		TShared<RHIVertexLayout> quadLayout = MakeShared<RHIVertexLayout>(2);
		quadLayout->AddAttribute(EVertexAttributeSemantic::Position, EVertexAttributeType::Float, 3, false);
		quadLayout->AddAttribute(EVertexAttributeSemantic::TexCoord, EVertexAttributeType::Float, 2, false);
		quadLayout->AddAttribute(EVertexAttributeSemantic::Normal,   EVertexAttributeType::Float, 3, true);

		RHIVertexBuffer* vb = RHIVertexBuffer::Create(quadVertices, sizeof(quadVertices) / sizeof(float));
		vb->SetLayout(quadLayout);
		vb->SetLayoutShader(m_BasicUnlitMaskedShader);

		RHIIndexBuffer* ib = RHIIndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32));

		m_BillboardMesh = Mesh::Create();
		m_BillboardMesh->SetVertexBuffer(vb);
		m_BillboardMesh->SetIndexBuffer(ib);

		// White Texture

		uint8 whiteTex[] = { 0xFF, 0xFF, 0xFF, 0xFF };

		TextureDescription whiteDesc { };
		whiteDesc.Dimensions.Width = 1;
		whiteDesc.Dimensions.Height = 1;
		whiteDesc.Usage = ETextureUsage::Immutable;
		whiteDesc.bCreateSampler = true;
		whiteDesc.DebugName = "WhiteTex";
		whiteDesc.InitialData = whiteTex;
		m_WhiteTexture = RHITexture::Create(whiteDesc);
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

	void Renderer::BindScreenTexturePrimitives(const RHIShader* customShader) const
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
		InitBasicUnlitMaskedShader();
		InitFXAAShader();
		InitEditorObjectIDShader();
		InitEditorSelectedShader();
		InitEditorViewportShader();
		InitEditorViewportMSShader();
	}

	void Renderer::InitBasicShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() == ERHI::DX11)
		{
			File::ReadToString(shadersPath + L"BasicVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"BasicPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Basic.vert", vertexSrc);
			File::ReadToString(shadersPath + L"Basic.frag", pixelSrc);
		}

		m_BasicShader = RHIShader::Create();
		m_BasicShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_BasicShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		if (!m_BasicShader->Compile())
		{
			LOG_ERROR("Could not compile the Basic Shader.");
			debugbreak();
		}
	}

	void Renderer::InitBasicUnlitMaskedShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() == ERHI::DX11)
		{
			File::ReadToString(shadersPath + L"BasicVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"BasicUnlitMaskedPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Basic.vert", vertexSrc);
			File::ReadToString(shadersPath + L"BasicUnlitMasked.frag", pixelSrc);
		}

		m_BasicUnlitMaskedShader = RHIShader::Create();
		m_BasicUnlitMaskedShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_BasicUnlitMaskedShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		if (!m_BasicUnlitMaskedShader->Compile())
		{
			LOG_ERROR("Could not compile the Basic Unlit Masked Shader.");
			debugbreak();
		}
	}

	void Renderer::InitFXAAShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() == ERHI::DX11)
		{
			File::ReadToString(shadersPath + L"TextureRenderVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"PP_FXAAPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"TextureRender.vert", vertexSrc);
			File::ReadToString(shadersPath + L"PP_FXAA.frag", pixelSrc);
		}

		m_PPFXAAShader = RHIShader::Create();
		m_PPFXAAShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_PPFXAAShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		if (!m_PPFXAAShader->Compile())
		{
			LOG_ERROR("Could not compile the PostProcess FXAA Shader.");
			debugbreak();
		}
	}

	void Renderer::InitEditorObjectIDShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() == ERHI::DX11)
		{
			File::ReadToString(shadersPath + L"Editor/EditorObjectIDVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorObjectIDPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Editor/EditorObjectID.vert", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorObjectID.frag", pixelSrc);
		}

		m_EditorObjectIDShader = RHIShader::Create();
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
		if (RHI::GetCurrent() == ERHI::DX11)
		{
			File::ReadToString(shadersPath + L"Editor/EditorSelectedVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"BasicUnlitMaskedPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Editor/EditorSelected.vert", vertexSrc);
		}

		m_EditorSelectedShader = RHIShader::Create();
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
		if (RHI::GetCurrent() == ERHI::DX11)
		{
			File::ReadToString(shadersPath + L"Editor/EditorViewportVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorViewportPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Editor/EditorViewport.vert", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorViewport.frag", pixelSrc);
		}

		m_EditorViewportShader = RHIShader::Create();
		m_EditorViewportShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_EditorViewportShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		if (!m_EditorViewportShader->Compile())
		{
			LOG_ERROR("Could not compile the EditorViewport Shader.");
			debugbreak();
		}
	}

	void Renderer::InitEditorViewportMSShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() == ERHI::DX11)
		{
			File::ReadToString(shadersPath + L"Editor/EditorViewportVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorViewportMSPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Editor/EditorViewport.vert", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorViewport.frag", pixelSrc);
		}

		m_EditorViewportMSShader = RHIShader::Create();
		m_EditorViewportMSShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_EditorViewportMSShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		if (!m_EditorViewportMSShader->Compile())
		{
			LOG_ERROR("Could not compile the EditorViewportMS Shader.");
			debugbreak();
		}
	}

	void Renderer::RenderEditorPass(const Scene* scene, const EditorPassData& data)
	{
		ionassert(scene);

		if (!data.RTObjectID || !data.RTObjectIDDepth || !data.RTSelectionDepth)
			return;

		// Set the alpha channel to 0
		RendererClearOptions clearOptions { };
		clearOptions.ClearColorValue = Vector4(0.0f, 0.0f, 0.0f, 0.0f);

		// ObjectID for viewport click selection

		SetRenderTarget(data.RTObjectID);
		SetDepthStencil(data.RTObjectIDDepth);
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

			m_WhiteTexture->Bind(0);

			Draw(prim, scene);

			// Reset
			meshUniforms.RenderGuid = UVector4();
		}

		for (const REditorPassBillboardPrimitive& billboardPrim : data.Billboards)
		{
			TShared<Mesh> billboardMesh = GetBillboardMesh();

			// Load the GUID
			MeshUniforms& meshUniforms = billboardMesh->GetUniformsDataRef();
			meshUniforms.RenderGuid = *(UVector4*)billboardPrim.Guid.GetRawBytes().data();

			DrawBillboard(billboardPrim.BillboardRenderProxy, GetEditorObjectIDShader().get(), scene);

			// Reset
			meshUniforms.RenderGuid = UVector4();
		}

		// Selection outline

		SetRenderTarget(nullptr);
		SetDepthStencil(data.RTSelectionDepth);
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

				m_WhiteTexture->Bind(0);

				Draw(prim, scene);
			}
		}

		if (!data.SelectedBillboards.empty())
		{
			for (const REditorPassBillboardPrimitive& billboardPrim : data.SelectedBillboards)
			{
				DrawBillboard(billboardPrim.BillboardRenderProxy, GetEditorSelectedShader().get(), scene);
			}
		}
	}

	Renderer* Renderer::s_Instance;
}
