#include "IonPCH.h"

#include "Renderer.h"

#include "RHI/RHI.h"
#include "RHI/OpenGL/OpenGLRenderer.h"
#include "RHI/DX10/DX10Renderer.h"
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
			case ERHI::DX10:
			{
				s_Instance = new DX10Renderer;
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

		const MaterialInstance* materialInstance = primitive.MaterialInstance;
		if (materialInstance)
		{
			const Material* material = materialInstance->GetBaseMaterial().get();
			if (material->BindShader(EShaderUsage::StaticMesh))
			{
				materialInstance->TransferParameters();
				material->UpdateConstantBuffer();

				materialInstance->BindTextures();
			}
			else
			{
				m_BasicShader->Bind();
			}
		}
		else
		{
			primitive.Shader->Bind();
		}

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

		DrawIndexed(primitive.IndexBuffer->GetIndexCount());
	}

	void Renderer::DrawBillboard(const RBillboardRenderProxy& billboard, const RHIShader* shader, const Scene* targetScene) const
	{
		TRACE_FUNCTION();

		ionassert(targetScene);

		std::shared_ptr<Mesh> billboardMesh = GetBillboardMesh();
		ionassert(billboardMesh);

		const RHIVertexBuffer* vb = billboardMesh->GetVertexBufferRaw();
		const RHIIndexBuffer* ib = billboardMesh->GetIndexBufferRaw();

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

	void Renderer::DrawScreenTexture(const TRef<RHITexture>& texture) const
	{
		TRACE_FUNCTION();

		BindScreenTexturePrimitives();
		texture->Bind(0);

		// Index count is always 6 (2 triangles)
		DrawIndexed(6);
	}

	void Renderer::DrawScreenTexture(const TRef<RHITexture>& texture, const RHIShader* shader) const
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
		if (RHI::GetCurrent() != ERHI::OpenGL)
		{
			vertexSrc = File::ReadToString(EnginePath::GetShadersPath() + "TextureRenderVS.hlsl").Unwrap();
			pixelSrc  = File::ReadToString(EnginePath::GetShadersPath() + "TextureRenderPS.hlsl").Unwrap();
		}
		else
		{
			vertexSrc = File::ReadToString(EnginePath::GetShadersPath() + "TextureRender.vert").Unwrap();
			pixelSrc  = File::ReadToString(EnginePath::GetShadersPath() + "TextureRender.frag").Unwrap();
		}

		m_ScreenTextureRenderData.Shader = RHIShader::Create();
		m_ScreenTextureRenderData.Shader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_ScreenTextureRenderData.Shader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		m_ScreenTextureRenderData.Shader->Compile().Unwrap();

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

		std::shared_ptr<RHIVertexLayout> quadLayout = std::make_shared<RHIVertexLayout>(2);
		quadLayout->AddAttribute(EVertexAttributeSemantic::Position, EVertexAttributeType::Float, 3, false);
		quadLayout->AddAttribute(EVertexAttributeSemantic::TexCoord, EVertexAttributeType::Float, 2, false);

		m_ScreenTextureRenderData.VertexBuffer = RHIVertexBuffer::Create(quadVertices, sizeof(quadVertices) / sizeof(float));
		m_ScreenTextureRenderData.VertexBuffer->SetLayout(quadLayout);
		m_ScreenTextureRenderData.VertexBuffer->SetLayoutShader(m_ScreenTextureRenderData.Shader);

		m_ScreenTextureRenderData.IndexBuffer = RHIIndexBuffer::CreateShared(quadIndices, sizeof(quadIndices) / sizeof(uint32));
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

		std::shared_ptr<RHIVertexLayout> quadLayout = std::make_shared<RHIVertexLayout>(2);
		quadLayout->AddAttribute(EVertexAttributeSemantic::Position, EVertexAttributeType::Float, 3, false);
		quadLayout->AddAttribute(EVertexAttributeSemantic::TexCoord, EVertexAttributeType::Float, 2, false);
		quadLayout->AddAttribute(EVertexAttributeSemantic::Normal,   EVertexAttributeType::Float, 3, true);

		TRef<RHIVertexBuffer> vb = RHIVertexBuffer::Create(quadVertices, sizeof(quadVertices) / sizeof(float));
		vb->SetLayout(quadLayout);
		vb->SetLayoutShader(m_BasicUnlitMaskedShader);

		std::shared_ptr<RHIIndexBuffer> ib = RHIIndexBuffer::CreateShared(quadIndices, sizeof(quadIndices) / sizeof(uint32));

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
		m_WhiteTexture = RHITexture::CreateRef(whiteDesc);
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

		FilePath shadersPath = EnginePath::GetShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() != ERHI::OpenGL)
		{
			vertexSrc = File::ReadToString(shadersPath + "BasicVS.hlsl").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "BasicPS.hlsl").Unwrap();
		}
		else
		{
			vertexSrc = File::ReadToString(shadersPath + "Basic.vert").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "Basic.frag").Unwrap();
		}

		m_BasicShader = RHIShader::Create();
		m_BasicShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_BasicShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		m_BasicShader->Compile()
			.Err<ShaderCompilationError>([](auto& err) { RendererLogger.Error("Could not compile the Basic Shader."); })
			.Unwrap();
	}

	void Renderer::InitBasicUnlitMaskedShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() != ERHI::OpenGL)
		{
			vertexSrc = File::ReadToString(shadersPath + "BasicVS.hlsl").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "BasicUnlitMaskedPS.hlsl").Unwrap();
		}
		else
		{
			vertexSrc = File::ReadToString(shadersPath + "Basic.vert").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "BasicUnlitMasked.frag").Unwrap();
		}

		m_BasicUnlitMaskedShader = RHIShader::Create();
		m_BasicUnlitMaskedShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_BasicUnlitMaskedShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		m_BasicUnlitMaskedShader->Compile()
			.Err<ShaderCompilationError>([](auto& err) { RendererLogger.Error("Could not compile the Basic Unlit Masked Shader."); })
			.Unwrap();
	}

	void Renderer::InitFXAAShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() != ERHI::OpenGL)
		{
			vertexSrc = File::ReadToString(shadersPath + "TextureRenderVS.hlsl").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "PP_FXAAPS.hlsl").Unwrap();
		}
		else
		{
			vertexSrc = File::ReadToString(shadersPath + "TextureRender.vert").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "PP_FXAA.frag").Unwrap();
		}

		m_PPFXAAShader = RHIShader::Create();
		m_PPFXAAShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_PPFXAAShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		m_PPFXAAShader->Compile()
			.Err<ShaderCompilationError>([](auto& err) { RendererLogger.Error("Could not compile the PostProcess FXAA Shader."); })
			.Unwrap();
	}

	void Renderer::InitEditorObjectIDShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() != ERHI::OpenGL)
		{
			vertexSrc = File::ReadToString(shadersPath + "Editor/EditorObjectIDVS.hlsl").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "Editor/EditorObjectIDPS.hlsl").Unwrap();
		}
		else
		{
			vertexSrc = File::ReadToString(shadersPath + "Editor/EditorObjectID.vert").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "Editor/EditorObjectID.frag").Unwrap();
		}

		m_EditorObjectIDShader = RHIShader::Create();
		m_EditorObjectIDShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_EditorObjectIDShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		m_EditorObjectIDShader->Compile()
			.Err<ShaderCompilationError>([](auto& err) { RendererLogger.Error("Could not compile the EditorObjectID Shader."); })
			.Unwrap();
	}

	void Renderer::InitEditorSelectedShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() != ERHI::OpenGL)
		{
			vertexSrc = File::ReadToString(shadersPath + "Editor/EditorSelectedVS.hlsl").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "BasicUnlitMaskedPS.hlsl").Unwrap();
		}
		else
		{
			vertexSrc = File::ReadToString(shadersPath + "Editor/EditorSelected.vert").Unwrap();
		}

		m_EditorSelectedShader = RHIShader::Create();
		m_EditorSelectedShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_EditorSelectedShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		m_EditorSelectedShader->Compile()
			.Err<ShaderCompilationError>([](auto& err) { RendererLogger.Error("Could not compile the EditorSelected Shader."); })
			.Unwrap();
	}

	void Renderer::InitEditorViewportShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() != ERHI::OpenGL)
		{
			vertexSrc = File::ReadToString(shadersPath + "Editor/EditorViewportVS.hlsl").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "Editor/EditorViewportPS.hlsl").Unwrap();
		}
		else
		{
			vertexSrc = File::ReadToString(shadersPath + "Editor/EditorViewport.vert").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "Editor/EditorViewport.frag").Unwrap();
		}

		m_EditorViewportShader = RHIShader::Create();
		m_EditorViewportShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_EditorViewportShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		m_EditorViewportShader->Compile()
			.Err<ShaderCompilationError>([](auto& err) { RendererLogger.Error("Could not compile the EditorViewport Shader."); })
			.Unwrap();
	}

	void Renderer::InitEditorViewportMSShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() != ERHI::OpenGL)
		{
			vertexSrc = File::ReadToString(shadersPath + "Editor/EditorViewportVS.hlsl").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "Editor/EditorViewportMSPS.hlsl").Unwrap();
		}
		else
		{
			vertexSrc = File::ReadToString(shadersPath + "Editor/EditorViewport.vert").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "Editor/EditorViewport.frag").Unwrap();
		}

		m_EditorViewportMSShader = RHIShader::Create();
		m_EditorViewportMSShader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_EditorViewportMSShader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		m_EditorViewportMSShader->Compile()
			.Err<ShaderCompilationError>([](auto& err) { RendererLogger.Error("Could not compile the EditorViewportMS Shader."); })
			.Unwrap();
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
			std::shared_ptr<Mesh> billboardMesh = GetBillboardMesh();

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
