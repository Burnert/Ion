#include "IonPCH.h"

#include "DX11Renderer.h"
#include "DX11Buffer.h"
#include "DX11Shader.h"
#include "DX11Texture.h"

#include "Renderer/Scene.h"

#include "Core/Platform/Windows/WindowsMacros.h"
#include "Core/Platform/Windows/WindowsUtility.h"

namespace Ion
{
	DX11Renderer::DX11Renderer() :
		m_CurrentScene({ })
	{
	}

	DX11Renderer::~DX11Renderer()
	{
	}

	void DX11Renderer::Init()
	{

	}

	void DX11Renderer::Clear() const
	{
		Clear(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void DX11Renderer::Clear(const Vector4& color) const
	{
		TRACE_FUNCTION();

		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall_v(context->ClearRenderTargetView(DX11::s_RTV, (float*)&color));
		dxcall_v(context->ClearDepthStencilView(DX11::s_DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0));
	}

	void DX11Renderer::Draw(const RPrimitiveRenderProxy& primitive, const TShared<Scene>& targetScene) const
	{
		TRACE_FUNCTION();

		ID3D11DeviceContext* context = DX11::GetContext();

		const Material* material = primitive.Material;
		const DX11VertexBuffer* vb = (DX11VertexBuffer*)primitive.VertexBuffer;
		const DX11IndexBuffer* ib = (DX11IndexBuffer*)primitive.IndexBuffer;
		const DX11UniformBuffer* ub = (DX11UniformBuffer*)primitive.UniformBuffer;
		const DX11Shader* shader = (DX11Shader*)primitive.Shader;

		shader->Bind();
		vb->Bind();
		vb->BindLayout();
		ib->Bind();

		const Matrix4& viewProjectionMatrix = targetScene->m_RenderCamera.ViewProjectionMatrix;
		const Matrix4& modelMatrix = primitive.Transform;

		MeshUniforms& uniformData = ub->DataRef<MeshUniforms>();
		uniformData.TransformMatrix = modelMatrix;
		uniformData.InverseTransposeMatrix = Math::InverseTranspose(modelMatrix);
		uniformData.ModelViewProjectionMatrix = viewProjectionMatrix * modelMatrix;

		ub->UpdateData();
		ub->Bind(1);

		if (material) material->BindTextures();
		//material->UpdateShaderUniforms();

		dxcall_v(context->DrawIndexed(ib->GetIndexCount(), 0, 0));
	}

	void DX11Renderer::RenderScene(const TShared<Scene>& scene)
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

	void DX11Renderer::SetCurrentScene(const TShared<Scene>& scene)
	{
		m_CurrentScene = scene;
	}

	const TShared<Scene>& DX11Renderer::GetCurrentScene() const
	{
		return m_CurrentScene;
	}

	void DX11Renderer::SetVSyncEnabled(bool bEnabled) const
	{
		DX11::SetSwapInterval((uint32)bEnabled);
	}

	bool DX11Renderer::IsVSyncEnabled() const
	{
		return DX11::GetSwapInterval();
	}

	void DX11Renderer::SetViewportDimensions(const ViewportDimensions& dimensions) const
	{
		TRACE_FUNCTION();

		HRESULT hResult = S_OK;

		D3D11_VIEWPORT viewport { };
		viewport.TopLeftX = (float)dimensions.X;
		viewport.TopLeftY = (float)dimensions.Y;
		viewport.Width = (float)dimensions.Width;
		viewport.Height = (float)dimensions.Height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		DX11::s_Context->RSSetViewports(1, &viewport);
		DX11::ResizeBuffers(dimensions.Width, dimensions.Height);
	}

	ViewportDimensions DX11Renderer::GetViewportDimensions() const
	{
		TRACE_FUNCTION();

		D3D11_VIEWPORT viewport { };
		uint32 nViewports = 0;

		DX11::s_Context->RSGetViewports(&nViewports, &viewport);

		ViewportDimensions dimensions { };
		dimensions.X = (int32)viewport.TopLeftX;
		dimensions.Y = (int32)viewport.TopLeftY;
		dimensions.Width = (int32)viewport.Width;
		dimensions.Height = (int32)viewport.Height;

		return dimensions;
	}

	void DX11Renderer::SetPolygonDrawMode(EPolygonDrawMode drawMode) const
	{

	}

	EPolygonDrawMode DX11Renderer::GetPolygonDrawMode() const
	{
		return EPolygonDrawMode();
	}
}
