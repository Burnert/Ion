#include "IonPCH.h"

#include "Example.h"
#include "Core/Asset/AssetRegistry.h"

#include "Core/Task/EngineTaskQueue.h"
#include "Core/Task/Task.h"

IonExample::IonExample() :
	m_TextureFileNameBuffer(""),
	m_EventDispatcher(this)
{
}

void IonExample::CreateExampleAssets()
{
	m_4Pak.Name = "4Pak";
	m_4Pak.MeshAsset    = AssetFinder(FilePath(L"Assets/models/4pak.iasset")).Resolve();
	m_4Pak.TextureAsset = AssetFinder(FilePath(L"Assets/textures/4pak.iasset")).Resolve();

	m_Piwsko.Name = "Piwsko";
	m_Piwsko.MeshAsset    = AssetFinder(FilePath(L"Assets/models/piwsko.iasset")).Resolve();
	m_Piwsko.TextureAsset = AssetFinder(FilePath(L"Assets/textures/piwsko.iasset")).Resolve();

	m_Oscypek.Name = "Oscypek";
	m_Oscypek.MeshAsset    = AssetFinder(FilePath(L"Assets/models/oscypek.iasset")).Resolve();
	m_Oscypek.TextureAsset = AssetFinder(FilePath(L"Assets/textures/oscypek.iasset")).Resolve();

	m_Ciupaga.Name = "Ciupaga";
	m_Ciupaga.MeshAsset    = AssetFinder(FilePath(L"Assets/models/ciupaga.iasset")).Resolve();
	m_Ciupaga.TextureAsset = AssetFinder(FilePath(L"Assets/textures/ciupaga.iasset")).Resolve();

	m_Slovak.Name = "Slovak";
	m_Slovak.MeshAsset    = AssetFinder(FilePath(L"Assets/models/slovak.iasset")).Resolve();
	m_Slovak.TextureAsset = AssetFinder(FilePath(L"Assets/textures/slovak.iasset")).Resolve();

	m_Stress.Name = "Stress";
	m_Stress.MeshAsset    = AssetFinder(FilePath(L"spherestresstest_uv.iasset")).Resolve();
	m_Stress.TextureAsset = AssetFinder(FilePath(L"Assets/test_4k.iasset")).Resolve();

	//m_BigSphere.Name = "BigSphere";
	//m_BigSphere.MeshAsset = AssetManager::Get()->CreateAsset(EAssetType::Mesh, FilePath(L"Assets/big_sphere.dae"));
	//m_BigSphere.TextureAsset = AssetManager::Get()->CreateAsset(EAssetType::Texture, FilePath(L"Assets/test_4k.png"));

	//m_4Pak.MeshAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_4Pak); });
	//m_4Pak.TextureAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_4Pak); });

	//m_Piwsko.MeshAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Piwsko); });
	//m_Piwsko.TextureAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Piwsko); });

	//m_Oscypek.MeshAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Oscypek); });
	//m_Oscypek.TextureAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Oscypek); });

	//m_Ciupaga.MeshAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Ciupaga); });
	//m_Ciupaga.TextureAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Ciupaga); });

	//m_Slovak.MeshAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Slovak); });
	//m_Slovak.TextureAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Slovak); });

	//m_Stress.MeshAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Stress); });
	//m_Stress.TextureAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Stress); });

	//m_BigSphere.MeshAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_BigSphere); });
	//m_BigSphere.TextureAsset->AssignEvent(
	//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_BigSphere); });
}

static void LoadMesh(Asset& asset, TShared<Mesh>& mesh)
{
	ionassert(asset->GetType() == EAssetType::Mesh);

	TOptional<AssetData> data = asset->Load([mesh](const AssetData& data) mutable
	{
		TShared<MeshAssetData> meshData = VariantCast<TShared<MeshAssetData>>(data.Variant);

		mesh = Mesh::Create();

		TShared<RHIVertexBuffer> vb = RHIVertexBuffer::CreateShared(meshData->Vertices.Ptr, meshData->Vertices.Count);
		TShared<RHIIndexBuffer> ib = RHIIndexBuffer::CreateShared(meshData->Indices.Ptr, (uint32)meshData->Indices.Count);
		vb->SetLayout(meshData->Layout);

		mesh->SetVertexBuffer(vb);
		mesh->SetIndexBuffer(ib);

		LOG_TRACE("Loaded Mesh");
	});
	ionassert(!data);
}

static void LoadTxtr(Asset& asset, TShared<RHITexture>& texture)
{
	ionassert(asset->GetType() == EAssetType::Image);

	TOptional<AssetData> data = asset->Load([texture](const AssetData& data) mutable
	{
		TShared<Image> image = VariantCast<TShared<Image>>(data.Variant);

		ionassert(image);

		TextureDescription desc { };
		desc.Dimensions.Width = image->GetWidth();
		desc.Dimensions.Height = image->GetHeight();
		desc.bGenerateMips = true;
		desc.bCreateSampler = true;
		desc.bUseAsRenderTarget = true;
		texture = RHITexture::CreateShared(desc);

		texture->UpdateSubresource(image.get());

		LOG_TRACE("Loaded Texture");
	});
	ionassert(!data);
}

static auto GetInitMeshFunc()
{
	return []
	{

	};
}

void IonExample::LoadExampleAssets()
{
	LoadMesh(m_4Pak.MeshAsset, m_4Pak.Mesh);
	LoadTxtr(m_4Pak.TextureAsset, m_4Pak.Texture);
	LoadMesh(m_Piwsko.MeshAsset, m_Piwsko.Mesh);
	LoadTxtr(m_Piwsko.TextureAsset, m_Piwsko.Texture);
	LoadMesh(m_Oscypek.MeshAsset, m_Oscypek.Mesh);
	LoadTxtr(m_Oscypek.TextureAsset, m_Oscypek.Texture);
	LoadMesh(m_Ciupaga.MeshAsset, m_Ciupaga.Mesh);
	LoadTxtr(m_Ciupaga.TextureAsset, m_Ciupaga.Texture);
	LoadMesh(m_Slovak.MeshAsset, m_Slovak.Mesh);
	LoadTxtr(m_Slovak.TextureAsset, m_Slovak.Texture);
	LoadMesh(m_Stress.MeshAsset, m_Stress.Mesh);
	LoadTxtr(m_Stress.TextureAsset, m_4Pak.Texture);

	// @FIXME: There once was a bug where one of the assets went into the wrong memory pool (I think?).
	// No idea what could cause it, I can't seem to reproduce it.
}

//static void BenchmarkMemory()
//{
//	char name[100];
//	int iters = 100;
//	size_t size = 1 << 21;
//
//	MemoryPool pool;
//	pool.AllocPool(1 << 30, 64); // 1GB
//
//	DebugTimer timer1;
//	for (int i = 0; i < iters; ++i)
//	{
//		void* ptr = malloc(size);
//	}
//	timer1.Stop();
//	sprintf_s(name, "malloc     (%d * %zuB)", iters, size);
//	timer1.PrintTimer(name, EDebugTimerTimeUnit::Millisecond);
//
//	DebugTimer timer2;
//	for (int i = 0; i < iters; ++i)
//	{
//		void* ptr = pool.Alloc(size);
//	}
//	timer2.Stop();
//	sprintf_s(name, "pool alloc (%d * %zuB)", iters, size);
//	timer2.PrintTimer(name, EDebugTimerTimeUnit::Millisecond);
//
//	pool.FreePool();
//}

TaskQueue Queue(4);

void IonExample::OnInit()
{
#pragma warning(disable:6001)

	//BenchmarkMemory();

	WorldInitializer worldInitializer { };
	m_World = g_Engine->CreateWorld(worldInitializer);

	CreateExampleAssets();
	LoadExampleAssets();

	EngineTaskQueue::Schedule(FTaskWork([](IMessageQueueProvider& queue)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		queue.PushMessage(FTaskMessage([]
		{
			LOG_INFO("Message!");
		}));
	}));

	AsyncTask task([](IMessageQueueProvider& queue)
	{
		LOG_INFO("AsyncTask!");
		std::this_thread::sleep_for(std::chrono::milliseconds(1300));
		queue.PushMessage(FTaskMessage([]
		{
			// Not printed by Queue because Queue.DispatchMessages() is not called
			LOG_INFO("AsyncMessage!");
		}));
	});

	task.Schedule();
	task.Schedule(Queue);
	std::this_thread::sleep_for(std::chrono::milliseconds(400));
	task.Schedule();
	task.Schedule(Queue);

	//m_Triangle.Shader = Shader::Create();
	//m_Triangle.Shader->AddShaderSource(EShaderType::Vertex, vertexSrc);
	//m_Triangle.Shader->AddShaderSource(EShaderType::Pixel, pixelSrc);

	//bResult = m_Triangle.Shader->Compile();
	//ionassert(bResult);

	//float vertices[] = {
	//	 0.0f,  0.5f, 0.0f,
	//	 0.5f, -0.5f, 0.0f,
	//	-0.5f, -0.5f, 0.0f,
	//};

	//uint32 indices[] = {
	//	0, 1, 2
	//};

	//float quad[] = {
	//	-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	//	 0.5f,  0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	//	 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
	//	-0.5f, -0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	//};

	//uint32 quadIndices[] = {
	//	0, 2, 1,
	//	2, 0, 3,
	//};

	//TShared<VertexLayout> layout = MakeShareable(new VertexLayout(1));
	//layout->AddAttribute(EVertexAttributeSemantic::Position, EVertexAttributeType::Float, 3, false);
	//layout->AddAttribute(EVertexAttributeSemantic::Normal, EVertexAttributeType::Float, 3, true);
	//layout->AddAttribute(EVertexAttributeSemantic::TexCoord, EVertexAttributeType::Float, 2, false);

	//m_Triangle.VB = VertexBuffer::Create(quad, sizeof(quad) / sizeof(float));
	//m_Triangle.VB->SetLayout(layout);
	//{
	//	DX11VertexBuffer* dx11VB = dynamic_cast<DX11VertexBuffer*>(m_Triangle.VB.get());
	//	if (dx11VB)
	//		dx11VB->CreateDX11Layout(std::static_pointer_cast<DX11Shader>(m_Triangle.Shader));
	//}

	//m_Triangle.IB = IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32));

	//ZeroMemory(&m_TriangleUniforms, sizeof(m_TriangleUniforms));
	////m_TriangleUniforms.Color = Vector4(1.0f, 0.5f, 0.3f, 1.0f);

	//UniformBufferFactory ubFactory;
	//ubFactory.Add("Color", EUniformType::Float4);
	////ubFactory.Construct(m_Triangle.UB);

	//m_Triangle.UB = MakeShareable(UniformBuffer::Create(m_TriangleUniforms));

	//File fImage(L"Assets/test.png");
	//Image* tImage = new Image;
	//tImage->Load(fImage);
	//m_Triangle.Texture = Texture::Create(tImage);

	WindowDimensions windowDimensions = GetWindow()->GetDimensions();
	CreateRenderTargetTexture(TextureDimensions { (uint32)windowDimensions.Width, (uint32)windowDimensions.Height });

	m_Camera = Camera::Create();
	m_Camera->SetTransform(Math::Translate(Vector3(0.0f, 0.0f, 2.0f)));
	m_Camera->SetFOV(Math::Radians(90.0f));
	m_Camera->SetNearClip(0.1f);
	m_Camera->SetFarClip(100.0f);

	m_World->GetScene()->SetActiveCamera(m_Camera);

	//m_Scene = Scene::Create();
	//m_Scene->SetActiveCamera(m_Camera);

	//m_AuxCamera = Camera::Create();
	//m_AuxCamera->SetTransform(Math::Translate(Vector3(0.0f, 0.0f, 4.0f)));
	//m_AuxCamera->SetFOV(Math::Radians(66.0f));
	//m_AuxCamera->SetNearClip(0.1f);
	//m_AuxCamera->SetFarClip(10.0f);

	//m_DirectionalLight = MakeShared<DirectionalLight>();
	//m_Scene->SetActiveDirectionalLight(m_DirectionalLight.get());

	//m_Light0->m_Location = Vector3(2.0f, 3.0f, -1.0f);
	//m_Light0->m_Color = Vector3(1.0f, 1.0f, 1.0f);
	//m_Light0->m_Intensity = 4.0f;
	//m_Light0->m_Falloff = 5.0f;

	//m_Light1->m_Location = Vector3(-1.0f, 1.0f, 2.0f);
	//m_Light1->m_Color = Vector3(0.3f, 0.6f, 1.0f);
	//m_Light1->m_Intensity = 3.0f;
	//m_Light1->m_Falloff = 4.0f;

	//m_Light2->m_Location = Vector3(2.0f, 1.0f, 4.0f);
	//m_Light2->m_Color = Vector3(1.0f, 0.1f, 0.0f);
	//m_Light2->m_Intensity = 2.0f;
	//m_Light2->m_Falloff = 7.0f;

	//m_Scene->AddLight(m_Light0.get());
	//m_Scene->AddLight(m_Light1.get());
	//m_Scene->AddLight(m_Light2.get());

	// @TODO: Create an asset manager for textures, meshes and other files that can be imported

#if 0
	String collada;
	File::ReadToString(L"char.dae", collada);

	TUnique<ColladaDocument> colladaDoc = MakeUnique<ColladaDocument>(collada);

	const ColladaData& colladaData = colladaDoc->GetData();

	TShared<VertexBuffer> colladaVertexBuffer = VertexBuffer::Create(colladaData.VertexAttributes, colladaData.VertexAttributeCount);
	colladaVertexBuffer->SetLayout(colladaData.Layout);

	TShared<IndexBuffer> colladaIndexBuffer = IndexBuffer::Create(colladaData.Indices, (uint32)colladaData.IndexCount);

	WString textureFileName = L"char.png";
	memset(m_TextureFileNameBuffer, 0, sizeof(m_TextureFileNameBuffer));
	StringConverter::WCharToChar(textureFileName.c_str(), m_TextureFileNameBuffer);

	FileOld* textureFile = FileOld::Create(textureFileName);
	Image* textureImage = new Image;
	ionverify(textureImage->Load(textureFile));
	delete textureFile;

	m_TextureCollada = Texture::Create(textureImage);

	m_Material = Material::Create();
	m_Material->SetShader(shader);
	m_Material->CreateParameter("Texture", EMaterialParameterType::Texture2D);
	m_Material->SetParameter("Texture", m_TextureCollada);
	m_Material->CreateParameter("Texture1", EMaterialParameterType::Texture2D);
	m_Material->SetParameter("Texture1", m_TextureCollada);

	m_Material->RemoveParameter("Texture1");

	m_MeshCollada = Mesh::Create();
	m_MeshCollada->SetVertexBuffer(colladaVertexBuffer);
	m_MeshCollada->SetIndexBuffer(colladaIndexBuffer);
	m_MeshCollada->SetMaterial(m_Material);
	m_MeshCollada->SetTransform(Math::Rotate(Math::Radians(-90.0f), Vector3(1.0f, 0.0f, 0.0f)));

	m_Scene->AddDrawableObject(m_MeshCollada.get());

#endif
	//for (int32 i = 0; i < 5; ++i) {
		//LOG_WARN("Init {0}", i);
	//}
	// 
	//FileOld* dirTest = FileOld::Create();
	//dirTest->SetFilename(L"Assets");
	//ionassert(dirTest->IsDirectory());
	//std::vector<FileInfo> files = dirTest->GetFilesInDirectory();
	//for (FileInfo& info : files)
	//{
	//	LOG_INFO(L"{0}, {1}, {2}, {3}", info.Filename, info.FullPath, info.Size, info.bDirectory ? L"Dir" : L"File");
	//}

	LOG_DEBUG("Model Init finished.");
}

void IonExample::OnUpdate(float deltaTime)
{
	float cameraMoveSpeed = 5.0f;

	if (GetInputManager()->IsMouseButtonPressed(Mouse::Right))
	{
		if (GetInputManager()->IsKeyPressed(Key::W))
		{
			m_CameraTransform += m_CameraTransform.GetForwardVector() * deltaTime * cameraMoveSpeed;
		}
		if (GetInputManager()->IsKeyPressed(Key::S))
		{
			m_CameraTransform += -m_CameraTransform.GetForwardVector() * deltaTime * cameraMoveSpeed;
		}
		if (GetInputManager()->IsKeyPressed(Key::A))
		{
			m_CameraTransform += -m_CameraTransform.GetRightVector() * deltaTime * cameraMoveSpeed;
		}
		if (GetInputManager()->IsKeyPressed(Key::D))
		{
			m_CameraTransform += m_CameraTransform.GetRightVector() * deltaTime * cameraMoveSpeed;
		}
		if (GetInputManager()->IsKeyPressed(Key::Q))
		{
			m_CameraTransform += Vector3(0.0f, -1.0f, 0.0f) * deltaTime * cameraMoveSpeed;
		}
		if (GetInputManager()->IsKeyPressed(Key::E))
		{
			m_CameraTransform += Vector3(0.0f, 1.0f, 0.0f) * deltaTime * cameraMoveSpeed;
		}
	}

	m_Camera->SetTransform(m_CameraTransform.GetMatrix());
	const Vector3& Location = m_Camera->GetLocation();

	//float& r = m_Triangle.UB->Data<TriangleUniforms>()->Color.r;
	//r = glm::mod(r + deltaTime, 1.0f);

	// Update the aspect ratio when the viewport size changes
	WindowDimensions dimensions = GetWindow()->GetDimensions();
	float aspectRatio = dimensions.GetAspectRatio();
	m_Camera->SetAspectRatio(aspectRatio);
	//m_AuxCamera->SetAspectRatio(aspectRatio);

	//m_Scene->SetAmbientLightColor(m_AmbientLightColor);
	//m_DirectionalLightRotation = Quaternion(Math::Radians(m_DirectionalLightAngles));
	//m_DirectionalLight->m_LightDirection = Math::Rotate(m_DirectionalLightRotation, Vector3(0.0f, 0.0f, -1.0f));
	//m_DirectionalLight->m_Color = m_DirectionalLightColor;
	//m_DirectionalLight->m_Intensity = m_DirectionalLightIntensity;
#if 0
	TShared<Material> meshMaterial = m_MeshCollada->GetMaterial().lock();
	TShared<Shader> meshShader = meshMaterial->GetShader();

	static float c_Angle = 0.0f;
	static Vector4 c_Tint(1.0f, 0.0f, 1.0f, 1.0f);

	m_MeshRotation.y = std::fmodf(m_MeshRotation.y + deltaTime * 90, 360);
	m_MeshTransform = { m_MeshLocation, m_MeshRotation, m_MeshScale };
	m_MeshCollada->SetTransform(m_MeshTransform.GetMatrix());

	c_Angle += deltaTime;
	c_Tint.y = (((c_Tint.y + deltaTime) >= 2.0f) ? 0.0f : (c_Tint.y + deltaTime));

	m_AuxCamera->SetTransform(Math::Translate(m_AuxCameraLocation));
#endif
	//m_Scene->UpdateRenderData();

	// ImGui:

	if (m_bDrawImGui)
	{
		ImGui::ShowDemoWindow();
		//ImGui::ShowAboutWindow();
		//ImGui::ShowFontSelector("Fonts");
		//ImGui::ShowStyleEditor();
		//ImGui::Begin("Guide");
		//ImGui::ShowUserGuide();
		//ImGui::End();

		ImGui::ShowMetricsWindow();

		ImGui::Begin("Diagnostics");
		{
			if (ImGui::Button("Start profiling"))
			{
				TRACE_RECORD_START();
			}
			if (ImGui::Button("Stop profiling"))
			{
				TRACE_RECORD_STOP();
			}
		}
		ImGui::End();

		ImGui::Begin("Texture settings");
		{
			ImGui::InputText("Texture file", m_TextureFileNameBuffer, sizeof(m_TextureFileNameBuffer));
			if (ImGui::Button("Set Texture"))
			{
				FileOld* textureFile = FileOld::Create(StringConverter::StringToWString(m_TextureFileNameBuffer));
				if (textureFile->Exists())
				{
					Image* textureImage = new Image;
					ionverify(textureImage->Load(textureFile));
					m_TextureCollada->UpdateSubresource(textureImage);
					//m_MeshCollada->GetMaterial().lock()->SetParameter("Texture", m_TextureCollada);
				}
				delete textureFile;
			}
			ImGui::DragFloat3("Aux Camera Location", &m_AuxCameraLocation.x, 0.01f, -FLT_MAX, FLT_MAX);
			if (ImGui::Button("Change Camera"))
			{
				//m_Scene->SetActiveCamera(m_Scene->GetActiveCamera() == m_Camera ? m_AuxCamera : m_Camera);
			}
		}
		ImGui::End();

		ImGui::Begin("Mesh settings");
		{
			ImGui::DragFloat3("Location", &m_MeshLocation.x, 0.01f, -FLT_MAX, FLT_MAX);
			ImGui::DragFloat3("Rotation", &m_MeshRotation.x, 1.0f, -FLT_MAX, FLT_MAX);
			ImGui::DragFloat3("Scale", &m_MeshScale.x, 0.01f, -FLT_MAX, FLT_MAX);
		}
		ImGui::End();

		ImGui::Begin("Scene settings");
		{
			ImGui::DragFloat3("Directional Light Rotation", &m_DirectionalLightAngles.x, 1.0f, -180.0f, 180.0f);
			ImGui::DragFloat3("Directional Light Color", &m_DirectionalLightColor.x, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Directional Light Intensity", &m_DirectionalLightIntensity, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat4("Ambient Light Color", &m_AmbientLightColor.x, 0.01f, 0.0f, 1.0f);

			//ImGui::DragFloat3("Light0 Location", &m_Light0->m_Location.x, 0.01f, -FLT_MAX, FLT_MAX);
			//ImGui::DragFloat3("Light1 Location", &m_Light1->m_Location.x, 0.01f, -FLT_MAX, FLT_MAX);
			//ImGui::DragFloat3("Light2 Location", &m_Light2->m_Location.x, 0.01f, -FLT_MAX, FLT_MAX);
		}
		ImGui::End();

		ImGui::Begin("Asset Manager");
		{
			//ImGuiAssetMemoryPoolControls<EAssetType::Mesh>();
			//ImGuiAssetMemoryPoolControls<EAssetType::Texture>();

			ImGui::Separator();

			ImGuiExampleModelOps(m_4Pak);
			ImGuiExampleModelOps(m_Piwsko);
			ImGuiExampleModelOps(m_Oscypek);
			ImGuiExampleModelOps(m_Ciupaga);
			ImGuiExampleModelOps(m_Slovak);
			ImGuiExampleModelOps(m_Stress);
			//ImGuiExampleModelOps(m_BigSphere);
		}
		ImGui::End();

		ImGui::Begin("Renderer Settings");
		{
			static const char* currentDrawMode = m_DrawModes[0];
			if (ImGui::BeginCombo("Draw Mode", currentDrawMode))
			{
				for (int32 i = 0; i < 3; ++i)
				{
					bool selected = currentDrawMode == m_DrawModes[i];
					if (ImGui::Selectable(m_DrawModes[i], selected))
					{
						currentDrawMode = m_DrawModes[i];
						if (currentDrawMode == m_DrawModes[0])
						{
							Renderer::Get()->SetPolygonDrawMode(EPolygonDrawMode::Fill);
						}
						else if (currentDrawMode == m_DrawModes[1])
						{
							Renderer::Get()->SetPolygonDrawMode(EPolygonDrawMode::Lines);
						}
						else if (currentDrawMode == m_DrawModes[2])
						{
							Renderer::Get()->SetPolygonDrawMode(EPolygonDrawMode::Points);
						}
					}
					if (selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("Toggle VSync"))
			{
				Renderer::Get()->SetVSyncEnabled(!Renderer::Get()->IsVSyncEnabled());
			}
		}
		ImGui::End();
	}
}

void IonExample::OnRender()
{
	Renderer::Get()->UnbindResources();
	// @TODO: Resize the render target on window resize
	Renderer::Get()->SetRenderTarget(m_RenderTarget);
	Renderer::Get()->SetDepthStencil(m_DepthStencil);

	Renderer::Get()->Clear(Vector4(0.1f, 0.1f, 0.1f, 1.0f));
	Renderer::Get()->RenderScene(m_World->GetScene());

	Renderer::Get()->SetRenderTarget(GetWindow()->GetWindowColorTexture());
	Renderer::Get()->SetDepthStencil(GetWindow()->GetWindowDepthStencilTexture());

	Renderer::Get()->Clear(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	Renderer::Get()->DrawScreenTexture(m_RenderTarget);
}

void IonExample::OnShutdown()
{
}

void IonExample::OnEvent(const Event& event)
{
	m_EventDispatcher.Dispatch(event);
}

void IonExample::OnKeyPressedEvent(const KeyPressedEvent& event)
{
	if (event.GetKeyCode() == Key::F4)
	{
		m_bDrawImGui = !m_bDrawImGui;
	}
}

void IonExample::OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event)
{
	if (GetInputManager()->IsMouseButtonPressed(Mouse::Right))
	{
		float yawDelta = event.GetX() * 0.2f;
		float pitchDelta = event.GetY() * 0.2f;

		Rotator cameraRotation = m_CameraTransform.GetRotation();
		cameraRotation.SetPitch(Math::Clamp(cameraRotation.Pitch() - pitchDelta, -89.99f, 89.99f));
		cameraRotation.SetYaw(cameraRotation.Yaw() - yawDelta);
		m_CameraTransform.SetRotation(cameraRotation);
	}
}

void IonExample::OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event)
{
	if (event.GetMouseButton() == Mouse::Right)
	{
		ImGui::SetWindowFocus();

		GetWindow()->LockCursor();
		GetWindow()->ShowCursor(false);
	}
}

void IonExample::OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event)
{
	if (event.GetMouseButton() == Mouse::Right)
	{
		GetWindow()->UnlockCursor();
		GetWindow()->ShowCursor(true);
	}
}

void IonExample::OnWindowResizeEvent(const WindowResizeEvent& event)
{
	CreateRenderTargetTexture(TextureDimensions { event.GetWidth(), event.GetHeight() });
}

void IonExample::CreateRenderTargetTexture(TextureDimensions dimensions)
{
	TextureDescription renderTargetDesc { };
	renderTargetDesc.Dimensions = dimensions;
	renderTargetDesc.bUseAsRenderTarget = true;
	renderTargetDesc.bCreateSampler = true;
	m_RenderTarget = RHITexture::CreateShared(renderTargetDesc);

	TextureDescription depthStencilDesc { };
	depthStencilDesc.Dimensions = dimensions;
	depthStencilDesc.bUseAsDepthStencil = true;
	depthStencilDesc.Format = ETextureFormat::D24S8;
	m_DepthStencil = RHITexture::CreateShared(depthStencilDesc);
}
