#include "EditorPCH.h"

#include "EditorViewport.h"
#include "EditorApplication/EditorApplication.h"
#include "Editor/UI/ViewportUI.h"
#include "Editor/EditorAssets.h"

#include "Engine/Components/SceneComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/World.h"

#include "Renderer/Renderer.h"

namespace Ion::Editor
{
	EditorViewport::EditorViewport(const UVector2& initialSize, int32 index) :
		m_UI(MakeShared<EditorUIViewport>(this, index)),
		m_Camera(Camera::Create()),
		m_CameraMoveSpeed(5.0f),
		m_ClickedPoint({ -1, -1 }),
		m_bCaptured(false),
		m_bEnableMSAA(false)
	{
		m_CameraTransform.SetLocation(Vector3(1.0f, 0.5f, 1.0f));
		m_CameraTransform.SetRotation(Rotator(Vector3(-10.0f, 45.0f, 0.0f)));
		m_Camera->SetFOV(Math::Radians(100.0f));
		m_Camera->SetNearClip(0.05f);
		m_Camera->SetFarClip(100.0f);

		CreateFramebuffers(initialSize);

		m_UI->SetOnResize([this](const UVector2& size)
		{
			TryResizeFramebuffers(size);
		});
	}

	void EditorViewport::Update(float deltaTime)
	{
		if (WasClickedLastFrame())
		{
			GUID clicked;
			if (ObtainClickedObject(clicked))
			{
				DispatchOnClicked(clicked);
			}
			m_ClickedPoint = { -1, -1 };
		}

		// Update the framebuffers when the MSAA mode changes
		if (m_ViewportTextures.SceneFinalColor->GetDescription().IsMultiSampled() != m_bEnableMSAA)
		{
			ResizeFramebuffers(m_ViewportSize);
		}
	}

	void EditorViewport::Render()
	{
		if (m_ViewportFinalColor)
		{
			// Assume other textures are initialized as well
			ionassert(m_ObjectIDColor);
			ionassert(m_ObjectIDDepthStencil);
			ionassert(m_ObjectIDStaging);
			ionassert(m_ViewportTextures.SceneFinalColor);
			ionassert(m_ViewportTextures.SceneFinalDepth);
			ionassert(m_ViewportTextures.SelectedDepth);

			Scene* editorScene = EditorApplication::Get()->GetEditorScene();
			ionassert(editorScene);

			m_Camera->SetTransform(m_CameraTransform.GetMatrix());
			editorScene->LoadCamera(m_Camera);

			TShared<EditorPassData> editorPassData = EditorApplication::Get()->GetEditorPassData();
			editorPassData->RTObjectID = m_ObjectIDColor;
			editorPassData->RTObjectIDDepth = m_ObjectIDDepthStencil;
			editorPassData->RTSelectionDepth = m_ViewportTextures.SelectedDepth;

			ViewportDescription viewport { };
			viewport.Width = GetSize().x;
			viewport.Height = GetSize().y;

			Renderer::Get()->SetViewport(viewport);

			// Render the scene

			Renderer::Get()->SetRenderTarget(m_ViewportTextures.SceneFinalColor);
			Renderer::Get()->SetDepthStencil(m_ViewportTextures.SceneFinalDepth);
			Renderer::Get()->Clear(Vector4(0.02f, 0.02f, 0.02f, 0.01f));
			Renderer::Get()->RenderScene(editorScene);
			RenderEditorBillboards();
			Renderer::Get()->SetBlendingEnabled(true);
			RenderEditorGrid();
			Renderer::Get()->SetBlendingEnabled(false);

			// Render editor pass

			Renderer::Get()->RenderEditorPass(editorScene, *editorPassData);

			// Render the viewport to the pre-fx viewport framebuffer

			Renderer::Get()->SetRenderTarget(m_ViewportPreFX);
			Renderer::Get()->Clear();
			m_ViewportTextures.SceneFinalDepth->Bind(1);
			m_ViewportTextures.SelectedDepth->Bind(2);
			TShared<Shader> viewportShader =
				// Select the proper shader
				m_ViewportTextures.SceneFinalColor->GetDescription().IsMultiSampled() ?
				Renderer::Get()->GetEditorViewportMSShader() :
				Renderer::Get()->GetEditorViewportShader();
			Renderer::Get()->DrawScreenTexture(m_ViewportTextures.SceneFinalColor, viewportShader.get());
			Renderer::Get()->UnbindResources();

			// Render to the final viewport framebuffer

			Renderer::Get()->SetRenderTarget(m_ViewportFinalColor);
			Renderer::Get()->Clear();
			if (m_bEnableFXAA)
			{
				Renderer::Get()->DrawScreenTexture(m_ViewportPreFX, Renderer::Get()->GetFXAAShader().get());
			}
			else
			{
				Renderer::Get()->DrawScreenTexture(m_ViewportPreFX);
			}
			Renderer::Get()->UnbindResources();
		}
	}

	void EditorViewport::Click(const IVector2& position)
	{
		m_ClickedPoint = position;
	}

	void EditorViewport::SetCaptureState(bool bCapture)
	{
		if (m_bCaptured != bCapture)
		{
			m_bCaptured = bCapture;

			EditorApplication::Get()->GetWindow()->ShowCursor(!bCapture);
			EditorApplication::Get()->GetWindow()->LockCursor(bCapture);

			if (m_bCaptured)
				DispatchOnCaptured();
			else
				DispatchOnReleased();
		}
	}

	void EditorViewport::DriveCameraRotation(const Vector2& axisValues)
	{
		float yawDelta = axisValues.y * 0.2f;
		float pitchDelta = axisValues.x * 0.2f;

		Rotator cameraRotation = m_CameraTransform.GetRotation();
		cameraRotation.SetPitch(Math::Clamp(cameraRotation.Pitch() - pitchDelta, -89.99f, 89.99f));
		cameraRotation.SetYaw(cameraRotation.Yaw() - yawDelta);
		m_CameraTransform.SetRotation(cameraRotation);
	}

	void EditorViewport::DriveCamera(const Vector3& axisValues, float deltaTime)
	{
		if (axisValues == Vector3(0.0f))
			return;

		Vector3 direction = Math::Normalize(
			m_CameraTransform.GetRightVector()   * axisValues.x +
			Vector3(0.0f, 1.0f, 0.0f)            * axisValues.y +
			m_CameraTransform.GetForwardVector() * axisValues.z);

		m_CameraTransform += direction * deltaTime * m_CameraMoveSpeed;
	}

	void EditorViewport::SetMSAAEnabled(bool bEnabled)
	{
		m_bEnableMSAA = bEnabled;
	}

	void EditorViewport::SetFXAAEnabled(bool bEnabled)
	{
		m_bEnableFXAA = bEnabled;
	}

	void EditorViewport::DispatchOnCaptured()
	{
		if (m_OnCaptured)
			m_OnCaptured(*this);
	}

	void EditorViewport::DispatchOnReleased()
	{
		if (m_OnReleased)
			m_OnReleased(*this);
	}

	bool EditorViewport::WasClickedLastFrame() const
	{
		return !(m_ClickedPoint.x == -1 || m_ClickedPoint.y == -1);
	}

	bool EditorViewport::ObtainClickedObject(GUID& outGuid)
	{
		if (!m_ObjectIDColor || !m_ObjectIDDepthStencil || !m_ObjectIDStaging)
			return false;

		// Copy the object ID to the staging texture
		m_ObjectIDColor->CopyTo(m_ObjectIDStaging);

		// Get the data on the CPU
		void* buffer = nullptr;
		int32 lineSize = 0;
		m_ObjectIDStaging->Map(buffer, lineSize, ETextureMapType::Read);
		ionassert(lineSize / sizeof(GUID) >= m_ObjectIDStaging->GetDimensions().Width);

		// Get the pixel as GUID bytes
		outGuid = GUID::Zero;
		memcpy(&outGuid, (GUID*)buffer + ((int32)m_ClickedPoint.y * (lineSize / sizeof(GUID)) + (int32)m_ClickedPoint.x), sizeof(GUID));

		// Cleanup
		m_ObjectIDStaging->Unmap();

		return true;
	}

	void EditorViewport::DispatchOnClicked(const GUID& clickedGuid) const
	{
		if (m_OnClicked)
			m_OnClicked(clickedGuid);
	}

	void EditorViewport::RenderEditorGrid()
	{
		RPrimitiveRenderProxy grid { };
		grid.Transform     = Transform().GetMatrix();
		grid.Material      = nullptr;
		grid.Shader        = EditorMeshes::ShaderGrid.get();
		grid.VertexBuffer  = EditorMeshes::MeshGrid->GetVertexBufferRaw();
		grid.IndexBuffer   = EditorMeshes::MeshGrid->GetIndexBufferRaw();
		grid.UniformBuffer = EditorMeshes::MeshGrid->GetUniformBufferRaw();

		Renderer::Get()->Draw(grid, EditorApplication::Get()->GetEditorScene());
	}

	void EditorViewport::RenderEditorBillboards()
	{
		World* editorWorld = EditorApplication::Get()->GetEditorWorld();

		ComponentRegistry& registry = editorWorld->GetComponentRegistry();
		registry.ForEachSceneComponent([editorWorld](SceneComponent* component)
		{
			if (component->IsOfType<MeshComponent>())
			{
				MeshComponent* meshComponent = (MeshComponent*)component;
				// Don't draw the billboard if the component has a mesh
				if (meshComponent->GetMesh())
				{
					return;
				}
			}

			SceneComponent* sceneComponent = (SceneComponent*)component;

			RBillboardRenderProxy billboard;
			billboard.Texture = EditorBillboards::GetComponentBillboardTexture(component->GetFinalTypeID()).get();
			billboard.LocationWS = sceneComponent->GetWorldTransform().GetLocation();
			billboard.Scale = 0.2f;

			const Shader* billboardShader = Renderer::Get()->GetBasicUnlitMaskedShader().get();
			Renderer::Get()->DrawBillboard(billboard, billboardShader, editorWorld->GetScene());
		});
	}

	void EditorViewport::CreateFramebuffers(const UVector2& size)
	{
		TRACE_FUNCTION();

		m_ViewportSize = size;

		TextureDescription desc { };
		desc.DebugName = "Viewport";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateSampler = true;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;
		desc.SetFilterAll(ETextureFilteringMethod::Linear);

		m_ViewportFinalColor = Texture::Create(desc);

		desc = { };
		desc.DebugName = "ViewportPreFX";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateSampler = true;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;
		desc.SetFilterAll(ETextureFilteringMethod::Linear);

		m_ViewportPreFX = Texture::Create(desc);

		CreateFinalSceneFramebuffer(size);
		CreateEditorPassFramebuffers(size);
	}

	void EditorViewport::ResizeFramebuffers(const UVector2& size)
	{
		TRACE_FUNCTION();

		m_ViewportSize = size;

		TextureDescription desc = m_ViewportFinalColor->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ViewportFinalColor = Texture::Create(desc);

		desc = m_ViewportPreFX->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ViewportPreFX = Texture::Create(desc);

		ResizeFinalSceneFramebuffer(size);
		ResizeEditorPassFramebuffers(size);

		m_Camera->SetAspectRatio((float)size.x / (float)size.y);
	}

	void EditorViewport::TryResizeFramebuffers(const UVector2& size)
	{
		if (size.x && size.y)
		{
			TextureDimensions viewportDimensions = m_ViewportFinalColor->GetDimensions();
			if (size != UVector2(viewportDimensions.Width, viewportDimensions.Height))
			{
				ResizeFramebuffers(size);
			}
		}
	}

	void EditorViewport::CreateFinalSceneFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc { };
		desc.DebugName = "FinalScene";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateSampler = true;
		desc.MultiSampling = m_bEnableMSAA ? ETextureMSMode::X4 : ETextureMSMode::X1;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;
		desc.SetFilterAll(ETextureFilteringMethod::Linear);

		m_ViewportTextures.SceneFinalColor = Texture::Create(desc);

		desc = { };
		desc.DebugName = "FinalSceneDepth";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsDepthStencil = true;
		desc.bCreateSampler = true;
		desc.MultiSampling = m_bEnableMSAA ? ETextureMSMode::X4 : ETextureMSMode::X1;
		desc.Format = ETextureFormat::D24S8;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;
		desc.SetFilterAll(ETextureFilteringMethod::Linear);

		m_ViewportTextures.SceneFinalDepth = Texture::Create(desc);
	}

	void EditorViewport::ResizeFinalSceneFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc = m_ViewportTextures.SceneFinalColor->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.MultiSampling = m_bEnableMSAA ? ETextureMSMode::X4 : ETextureMSMode::X1;
		m_ViewportTextures.SceneFinalColor = Texture::Create(desc);

		desc = m_ViewportTextures.SceneFinalDepth->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.MultiSampling = m_bEnableMSAA ? ETextureMSMode::X4 : ETextureMSMode::X1;
		m_ViewportTextures.SceneFinalDepth = Texture::Create(desc);
	}

	void EditorViewport::CreateEditorPassFramebuffers(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc { };
		desc.DebugName = "EditorSelectedDepth";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsDepthStencil = true;
		desc.bCreateSampler = true;
		desc.Format = ETextureFormat::D24S8;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;
		desc.SetFilterAll(ETextureFilteringMethod::Linear);

		m_ViewportTextures.SelectedDepth = Texture::Create(desc);

		desc = { };
		desc.DebugName = "EditorObjectID";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateSampler = true;
		desc.Format = ETextureFormat::UInt128GUID;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;
		desc.SetFilterAll(ETextureFilteringMethod::Linear);

		m_ObjectIDColor = Texture::Create(desc);

		desc = { };
		desc.DebugName = "EditorObjectIDDepth";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsDepthStencil = true;
		desc.Format = ETextureFormat::D24S8;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;
		desc.SetFilterAll(ETextureFilteringMethod::Linear);

		m_ObjectIDDepthStencil = Texture::Create(desc);

		desc = { };
		desc.DebugName = "EditorObjectIDStaging";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bAllowCPUReadAccess = true;
		desc.Usage = ETextureUsage::Staging;
		desc.Format = ETextureFormat::UInt128GUID;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;
		desc.SetFilterAll(ETextureFilteringMethod::Linear); 

		m_ObjectIDStaging = Texture::Create(desc);
	}

	void EditorViewport::ResizeEditorPassFramebuffers(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc = m_ViewportTextures.SelectedDepth->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ViewportTextures.SelectedDepth = Texture::Create(desc);

		desc = m_ObjectIDColor->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ObjectIDColor = Texture::Create(desc);

		desc = m_ObjectIDDepthStencil->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ObjectIDDepthStencil = Texture::Create(desc);

		desc = m_ObjectIDStaging->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ObjectIDStaging = Texture::Create(desc);
	}
}
