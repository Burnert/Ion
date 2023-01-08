#include "EditorPCH.h"

#include "EditorViewport.h"
#include "EditorApplication/EditorApplication.h"
#include "Editor/EditorAssets.h"
#include "Editor/ContentBrowser/ContentBrowser.h"

#include "Engine/Components/SceneComponent.h"
#include "Engine/Components/MeshComponent.h"
#include "Engine/World.h"
#include "Engine/Entity/MeshEntity.h"

#include "Renderer/Renderer.h"

#include "UserInterface/ImGui.h"

namespace Ion::Editor
{
	// UIEditorViewport -------------------------------------------------------------------

	EditorUIViewport::EditorUIViewport(EditorViewport* owner, int32 index) :
		m_Owner(owner),
		m_WindowName("Viewport"),
		m_Size({ }),
		m_Rect({ }),
		m_Index(index),
		m_bWindowOpen(false),
		m_bHovered(false),
		m_bCaptured(false),
		m_bMSAA(false),
		m_bFXAA(false)
	{
	}

	void EditorUIViewport::Draw()
	{
		TRACE_FUNCTION();

		m_bHovered = false;

		if (m_bWindowOpen)
		{
			ImGuiWindowFlags windowFlags =
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoNavFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f, 1.0f));

			if (m_bCaptured)
				ImGui::SetNextWindowFocus();

			String windowName = m_WindowName + "##" + ToString(m_Index);
			bool bOpen = ImGui::Begin(windowName.c_str(), &m_bWindowOpen, windowFlags);
			ImGui::PopStyleVar(2);
			if (bOpen)
			{
				// Save the viewport rect for later
				m_Rect = ImGui::GetWindowWorkRect();
				m_bHovered = ImGui::IsWindowHovered();

				UVector2 lastSize = m_Size;
				m_Size = UVector2(m_Rect.z - m_Rect.x, m_Rect.w - m_Rect.y);
				if (lastSize != m_Size)
				{
					DispatchOnResize(m_Size);
				}

				TRef<RHITexture> viewportFramebuffer = m_Owner->GetViewportFramebuffer();
				if (viewportFramebuffer)
				{
					ImVec2 startCursor = ImGui::GetCursorPos();

					const TextureDimensions& viewportDimensions = viewportFramebuffer->GetDimensions();
					ImGui::Image(viewportFramebuffer->GetNativeID(),
						ImVec2((float)viewportDimensions.Width, (float)viewportDimensions.Height));

					if (ImGui::BeginDragDropTarget())
					{
						ImGuiDragDropFlags dndFlags = ImGuiDragDropFlags_None;
						// From the Insert panel
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DNDID_InsertEntity, dndFlags))
						{
							ionassert(payload->DataSize == sizeof(DNDInsertEntityData));

							DNDInsertEntityData& data = *(DNDInsertEntityData*)payload->Data;
							data.Instantiate(EditorApplication::Get()->GetEditorWorld(), data.CustomData);
						}
						// From the Content Browser
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DNDID_MeshAsset, dndFlags))
						{
							ionassert(payload->DataSize == sizeof(DNDAssetData));

							DNDAssetData& data = *(DNDAssetData*)payload->Data;

							TSharedPtr<MeshResource> meshResource = MeshResource::Query(data.AssetHandle);
							std::shared_ptr<Mesh> mesh = Mesh::CreateFromResource(meshResource);

							World* world = EditorApplication::Get()->GetEditorWorld();
							TObjectPtr<MeshEntity> meshEntity = world->SpawnEntityOfClass<MeshEntity>();

							meshEntity->GetMeshComponent()->SetMeshResource(meshResource);
							meshEntity->GetMeshComponent()->SetMeshAsset(data.AssetHandle);
							meshEntity->SetMesh(mesh);
						}
						ImGui::EndDragDropTarget();
					}
					if (ImGui::IsItemHovered())
					{
						g_pEngineApplication->SetCursor(ECursorType::Cross);
					}
					if (ImGui::IsItemClicked())
					{
						ImVec2 mousePos = ImGui::GetMousePos();
						Vector4 workRect = ImGui::GetWindowWorkRect();
						Vector2 viewportPos = { mousePos.x - workRect.x, mousePos.y - workRect.y };
						m_Owner->Click(viewportPos);
					}
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						if (CanCapture())
							Capture();
					}
					if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
					{
						Release();
					}

					ImGui::SetCursorPos(startCursor);
					if (ImGui::Checkbox("MSAA", &m_bMSAA))
					{
						m_Owner->SetMSAAEnabled(m_bMSAA);
					}
					if (ImGui::Checkbox("FXAA", &m_bFXAA))
					{
						m_Owner->SetFXAAEnabled(m_bFXAA);
					}
				}
			}

			ImGui::End();
		}
	}

	void EditorUIViewport::SetOpen(bool bOpen)
	{
		m_bWindowOpen = bOpen;
	}

	void EditorUIViewport::SetWindowName(const String& name)
	{
		m_WindowName = name;
	}

	bool EditorUIViewport::CanCapture() const
	{
		return m_bHovered && IsMouseInRect();
	}

	bool EditorUIViewport::IsMouseInRect() const
	{
		IVector2 cursorPos = InputManager::GetCursorPosition();
		return
			// @TODO: Get some IsPointInRect function goin' (like PtInRect)
			cursorPos.x >= m_Rect.x && cursorPos.x <= m_Rect.z &&
			cursorPos.y >= m_Rect.y && cursorPos.y <= m_Rect.w;
	}

	void EditorUIViewport::Capture()
	{
		if (!m_bCaptured)
		{
			m_bCaptured = true;
			m_Owner->SetCaptureState(true);
		}
	}

	void EditorUIViewport::Release()
	{
		if (m_bCaptured)
		{
			m_bCaptured = false;
			m_Owner->SetCaptureState(false);
		}
	}

	void EditorUIViewport::DispatchOnResize(const UVector2& size)
	{
		if (m_OnResize)
			m_OnResize(m_Size);
	}

	// EditorViewport -------------------------------------------------------------------

	EditorViewport::EditorViewport(const UVector2& initialSize, int32 index) :
		m_UI(std::make_shared<EditorUIViewport>(this, index)),
		m_Camera(Camera::Create()),
		m_CameraMoveSpeed(5.0f),
		m_ClickedPoint({ -1, -1 }),
		m_bCaptured(false),
		m_bEnableMSAA(false),
		m_bEnableFXAA(false)
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

			std::shared_ptr<EditorPassData> editorPassData = EditorApplication::Get()->GetEditorPassData();
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
			TRef<RHIShader> viewportShader =
				// Select the proper shader
				m_ViewportTextures.SceneFinalColor->GetDescription().IsMultiSampled() ?
				Renderer::Get()->GetEditorViewportMSShader() :
				Renderer::Get()->GetEditorViewportShader();
			Renderer::Get()->DrawScreenTexture(m_ViewportTextures.SceneFinalColor, viewportShader.Raw());
			Renderer::Get()->UnbindResources();

			// Render to the final viewport framebuffer

			Renderer::Get()->SetRenderTarget(m_ViewportFinalColor);
			Renderer::Get()->Clear();
			if (m_bEnableFXAA)
			{
				Renderer::Get()->DrawScreenTexture(m_ViewportPreFX, Renderer::Get()->GetFXAAShader().Raw());
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
		ionassert(lineSize / GUID::Size >= m_ObjectIDStaging->GetDimensions().Width);

		// Get the pixel as GUID bytes
		GUIDBytesArray guidBytes;
		memcpy(&guidBytes, (GUIDBytesArray*)buffer + ((int32)m_ClickedPoint.y * (lineSize / GUID::Size) + (int32)m_ClickedPoint.x), GUID::Size);
		outGuid = GUID(guidBytes);

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
		grid.Shader        = EditorMeshes::ShaderGrid.Raw();
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
			billboard.Texture = EditorBillboards::GetComponentBillboardTexture(component->GetFinalTypeID()).Raw();
			billboard.LocationWS = sceneComponent->GetWorldTransform().GetLocation();
			billboard.Scale = 0.2f;

			const RHIShader* billboardShader = Renderer::Get()->GetBasicUnlitMaskedShader().Raw();
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

		m_ViewportFinalColor = RHITexture::Create(desc);

		desc = { };
		desc.DebugName = "ViewportPreFX";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsRenderTarget = true;
		desc.bCreateSampler = true;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;
		desc.SetFilterAll(ETextureFilteringMethod::Linear);

		m_ViewportPreFX = RHITexture::Create(desc);

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
		m_ViewportFinalColor = RHITexture::Create(desc);

		desc = m_ViewportPreFX->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ViewportPreFX = RHITexture::Create(desc);

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

		m_ViewportTextures.SceneFinalColor = RHITexture::Create(desc);

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

		m_ViewportTextures.SceneFinalDepth = RHITexture::Create(desc);
	}

	void EditorViewport::ResizeFinalSceneFramebuffer(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc = m_ViewportTextures.SceneFinalColor->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.MultiSampling = m_bEnableMSAA ? ETextureMSMode::X4 : ETextureMSMode::X1;
		m_ViewportTextures.SceneFinalColor = RHITexture::Create(desc);

		desc = m_ViewportTextures.SceneFinalDepth->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.MultiSampling = m_bEnableMSAA ? ETextureMSMode::X4 : ETextureMSMode::X1;
		m_ViewportTextures.SceneFinalDepth = RHITexture::Create(desc);
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

		m_ViewportTextures.SelectedDepth = RHITexture::Create(desc);

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

		m_ObjectIDColor = RHITexture::Create(desc);

		desc = { };
		desc.DebugName = "EditorObjectIDDepth";
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		desc.bUseAsDepthStencil = true;
		desc.Format = ETextureFormat::D24S8;
		desc.UWrapMode = ETextureWrapMode::Clamp;
		desc.VWrapMode = ETextureWrapMode::Clamp;
		desc.SetFilterAll(ETextureFilteringMethod::Linear);

		m_ObjectIDDepthStencil = RHITexture::Create(desc);

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

		m_ObjectIDStaging = RHITexture::Create(desc);
	}

	void EditorViewport::ResizeEditorPassFramebuffers(const UVector2& size)
	{
		TRACE_FUNCTION();

		TextureDescription desc = m_ViewportTextures.SelectedDepth->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ViewportTextures.SelectedDepth = RHITexture::Create(desc);

		desc = m_ObjectIDColor->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ObjectIDColor = RHITexture::Create(desc);

		desc = m_ObjectIDDepthStencil->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ObjectIDDepthStencil = RHITexture::Create(desc);

		desc = m_ObjectIDStaging->GetDescription();
		desc.Dimensions.Width = size.x;
		desc.Dimensions.Height = size.y;
		m_ObjectIDStaging = RHITexture::Create(desc);
	}
}
