/* 
* PBR Resources:
* https://learnopengl.com/PBR/Theory
* https://blog.selfshadow.com/publications/s2013-shading-course/hoffman/s2013_pbs_physics_math_notes.pdf
* https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
* https://www.shadertoy.com/view/4sSfzK
* http://www.codinglabs.net/article_physically_based_rendering.aspx
* http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx
* http://blog.wolfire.com/2015/10/Physically-based-rendering
*/

#include "IonPCH.h"

#include "IonApp.h"
#include "Renderer/Renderer.h"
#include "UserInterface/ImGui.h"

#include "Core/File/Collada.h"

#include "RenderAPI/DX11/DX11.h"
#include "RenderAPI/DX11/DX11Shader.h"
#include "RenderAPI/DX11/DX11Buffer.h"

#include "RenderAPI/RenderAPI.h"

class IonExample : public IonApplication
{
public:
	IonExample()
		: m_TextureFileNameBuffer(""),
		m_EventDispatcher(this)
	{
		ION_LOG_DEBUG("IonExample constructed.");
	}

	enum class EExampleModelName : uint8
	{
		FourPak,
		Piwsko,
		Oscypek,
		Ciupaga,
		Slovak,
		Stress,
		BigSphere,
	};

	template<EExampleModelName Type>
	struct ExampleModelData
	{
		static constexpr EExampleModelName ModelType = Type;

		TShared<Mesh> Mesh;
		TShared<Material> Material;
		TShared<Texture> Texture;

		AssetHandle MeshAsset;
		AssetHandle TextureAsset;

		String Name;
	};

	template<EExampleModelName Type>
	static void InitExampleModel(ExampleModelData<Type>& data, const TShared<Shader>& shader, TShared<Scene>& scene, const Matrix4& transform)
	{
		// Mesh

		const AssetDescription::Mesh* meshDesc = data.MeshAsset->GetDescription<EAssetType::Mesh>();
		float* vertexAttributesPtr = (float*)((uint8*)data.MeshAsset->Data() + meshDesc->VerticesOffset);
		uint32* indicesPtr = (uint32*)((uint8*)data.MeshAsset->Data() + meshDesc->IndicesOffset);

		TShared<VertexBuffer> vb = VertexBuffer::Create(vertexAttributesPtr, meshDesc->VertexCount);
		vb->SetLayout(meshDesc->VertexLayout);
		{
			DX11VertexBuffer* dx11VB = dynamic_cast<DX11VertexBuffer*>(vb.get());
			if (dx11VB)
				dx11VB->CreateDX11Layout(TStaticCast<DX11Shader>(shader));
		}
		TShared<IndexBuffer> ib = IndexBuffer::Create(indicesPtr, (uint32)meshDesc->IndexCount);
		
		// Texure

		data.Texture = Texture::Create(data.TextureAsset);

		// Other

		data.Material = Material::Create();
		data.Material->SetShader(shader);
		data.Material->CreateParameter("Texture", EMaterialParameterType::Texture2D);
		data.Material->SetParameter("Texture", data.Texture);

		data.Mesh = Mesh::Create();
		data.Mesh->SetVertexBuffer(vb);
		data.Mesh->SetIndexBuffer(ib);
		data.Mesh->SetMaterial(data.Material);
		data.Mesh->SetTransform(Math::Scale(Vector3(1.0f)) * transform);

		scene->AddDrawableObject(data.Mesh.get());
	}

	void CreateExampleAssets()
	{
		m_4Pak.Name            = "4Pak";
		m_4Pak.MeshAsset       = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"Assets/models/4pak.dae"));
		m_4Pak.TextureAsset    = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"Assets/textures/4pak.png"));

		m_Piwsko.Name          = "Piwsko";
		m_Piwsko.MeshAsset     = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"Assets/models/piwsko.dae"));
		m_Piwsko.TextureAsset  = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"Assets/textures/piwsko.png"));

		m_Oscypek.Name         = "Oscypek";
		m_Oscypek.MeshAsset    = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"Assets/models/oscypek.dae"));
		m_Oscypek.TextureAsset = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"Assets/textures/oscypek.png"));

		m_Ciupaga.Name         = "Ciupaga";
		m_Ciupaga.MeshAsset    = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"Assets/models/ciupaga.dae"));
		m_Ciupaga.TextureAsset = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"Assets/textures/ciupaga.png"));

		m_Slovak.Name          = "Slovak";
		m_Slovak.MeshAsset     = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"Assets/models/slovak.dae"));
		m_Slovak.TextureAsset  = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"Assets/textures/slovak.png"));

		m_Stress.Name          = "Stress";
		m_Stress.MeshAsset     = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"spherestresstest_uv.dae"));
		m_Stress.TextureAsset  = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"Assets/test_4k.png"));

		//m_BigSphere.Name = "BigSphere";
		//m_BigSphere.MeshAsset = AssetManager::Get()->CreateAsset(EAssetType::Mesh, FilePath(L"Assets/big_sphere.dae"));
		//m_BigSphere.TextureAsset = AssetManager::Get()->CreateAsset(EAssetType::Texture, FilePath(L"Assets/test_4k.png"));

		m_4Pak.MeshAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_4Pak); });
		m_4Pak.TextureAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_4Pak); });

		m_Piwsko.MeshAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Piwsko); });
		m_Piwsko.TextureAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Piwsko); });

		m_Oscypek.MeshAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Oscypek); });
		m_Oscypek.TextureAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Oscypek); });

		m_Ciupaga.MeshAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Ciupaga); });
		m_Ciupaga.TextureAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Ciupaga); });

		m_Slovak.MeshAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Slovak); });
		m_Slovak.TextureAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Slovak); });

		m_Stress.MeshAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Stress); });
		m_Stress.TextureAsset->AssignEvent(
			[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_Stress); });

		//m_BigSphere.MeshAsset->AssignEvent(
		//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_BigSphere); });
		//m_BigSphere.TextureAsset->AssignEvent(
		//	[this](const OnAssetLoadedMessage&) { InitModelIfReady(m_BigSphere); });
	}

	void LoadExampleAssets()
	{
		m_4Pak.MeshAsset->LoadAssetData();
		m_4Pak.TextureAsset->LoadAssetData();
		m_Piwsko.MeshAsset->LoadAssetData();
		m_Piwsko.TextureAsset->LoadAssetData();
		m_Oscypek.MeshAsset->LoadAssetData();
		m_Oscypek.TextureAsset->LoadAssetData();
		m_Ciupaga.MeshAsset->LoadAssetData();
		m_Ciupaga.TextureAsset->LoadAssetData();
		m_Slovak.MeshAsset->LoadAssetData();
		m_Slovak.TextureAsset->LoadAssetData();
		m_Stress.MeshAsset->LoadAssetData();
		m_Stress.TextureAsset->LoadAssetData();

		// @FIXME: There once was a bug where one of the assets went into the wrong memory pool (I think?).
		// No idea what could cause it, I can't seem to reproduce it.
	}

	// Template:

	//if constexpr (N == EExampleModelName::FourPak)
	//{

	//}
	//if constexpr (N == EExampleModelName::Piwsko)
	//{

	//}
	//if constexpr (N == EExampleModelName::Oscypek)
	//{

	//}
	//if constexpr (N == EExampleModelName::Ciupaga)
	//{

	//}
	//if constexpr (N == EExampleModelName::Slovak)
	//{

	//}
	//if constexpr (N == EExampleModelName::Stress)
	//{

	//}

	template<EExampleModelName N>
	void InitModelIfReady(ExampleModelData<N>& data)
	{
		if (!(data.MeshAsset->IsLoaded() && data.TextureAsset->IsLoaded()) || data.Mesh) return;

		if constexpr (N == EExampleModelName::FourPak)
		{
			// 4Pak
			InitExampleModel(data, m_Shader, m_Scene,
				Math::Translate(Vector3(2.0f, 1.0f, 0.0f)) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 0.0f, 0.0f)))));
		}
		if constexpr (N == EExampleModelName::Piwsko)
		{
			// Piwsko
			InitExampleModel(data, m_Shader, m_Scene,
				Math::Translate(Vector3(-2.0f, 1.0f, 0.0f)) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 0.0f, 0.0f)))));
		}
		if constexpr (N == EExampleModelName::Oscypek)
		{
			// Oscypek
			InitExampleModel(data, m_Shader, m_Scene,
				Math::Translate(Vector3(-1.0f, 1.0f, 1.0f)) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 0.0f, 0.0f)))));
		}
		if constexpr (N == EExampleModelName::Ciupaga)
		{
			// Ciupaga
			InitExampleModel(data, m_Shader, m_Scene,
				Math::Translate(Vector3(1.0f, 1.0f, 1.0f)) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 90.0f, 0.0f)))));
		}
		if constexpr (N == EExampleModelName::Slovak)
		{
			// Slovak
			InitExampleModel(data, m_Shader, m_Scene,
				Math::Translate(Vector3(1.0f, 0.0f, -2.0f)) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 180.0f, 0.0f)))));
		}
		if constexpr (N == EExampleModelName::Stress)
		{
			// Kula
			InitExampleModel(data, m_Shader, m_Scene,
				Math::Translate(Vector3(-1.0f, 0.0f, -2.0f))* Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 180.0f, 0.0f)))));
		}
		//if constexpr (N == EExampleModelName::BigSphere)
		//{
		//	// Kula
		//	InitExampleModel(data, m_Shader, m_Scene,
		//		Math::Translate(Vector3(0.0f, 5.0f, -2.0f)) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 180.0f, 0.0f)))));
		//}
	}

	template<EExampleModelName N>
	void DeleteModel()
	{
		if constexpr (N == EExampleModelName::FourPak)
		{
			
		}
		if constexpr (N == EExampleModelName::Piwsko)
		{
			
		}
		if constexpr (N == EExampleModelName::Oscypek)
		{
			
		}
		if constexpr (N == EExampleModelName::Ciupaga)
		{
			
		}
		if constexpr (N == EExampleModelName::Slovak)
		{
			
		}
		if constexpr (N == EExampleModelName::Stress)
		{
			
		}
	}

	MeshUniforms m_TriangleUniforms;

	struct Triangle
	{
		TShared<VertexBuffer> VB;
		TShared<IndexBuffer> IB;
		TShared<UniformBuffer> UB;
		TShared<Shader> Shader;
		TShared<Texture> Texture;
	} m_Triangle;

	template<EExampleModelName Type>
	void ImGuiExampleModelOps(ExampleModelData<Type>& model)
	{
		ImGui::PushID(model.Name.c_str());

		if (!model.MeshAsset->IsLoaded() && !model.MeshAsset->IsLoading())
			ImGui::TextDisabled(model.Name.c_str());
		else
			ImGui::TextColored(model.MeshAsset->IsLoading() ? ImVec4(1.0f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), model.Name.c_str());

		ImGui::SameLine(70);
		if (ImGui::Button("Load"))
		{
			model.MeshAsset->LoadAssetData();
			model.TextureAsset->LoadAssetData();
		} ImGui::SameLine();
		if (ImGui::Button("Unload"))
		{
			model.MeshAsset->UnloadAssetData();
			model.TextureAsset->UnloadAssetData();
		}
		ImGui::PopID();
	}

	template<EAssetType Type>
	void ImGuiAssetMemoryPoolControls()
	{
		ImGui::PushID(AssetTypeToString(Type));
		ImGui::Text("%s Pool", AssetTypeToString(Type));
		ImGui::SameLine(100);
		if (ImGui::Button("Print"))
		{
			AssetManager::PrintAssetPool<Type>();
		}
		ImGui::SameLine();
		if (ImGui::Button("Defragment"))
		{
			AssetManager::DefragmentAssetPool<Type>();
		}
		ImGui::SameLine();
		ImGui::Text("Size: %.2f MB", AssetManager::GetAssetMemoryPool<Type>().GetSize() / (float)(1 << 20));
		ImGui::PopID();
	}

	virtual void OnInit() override
	{
#pragma warning(disable:6001)

		CreateExampleAssets();
		LoadExampleAssets();

		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

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

		bool bResult;

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

		m_Camera = Camera::Create();
		m_Camera->SetTransform(Math::Translate(Vector3(0.0f, 0.0f, 2.0f)));
		m_Camera->SetFOV(Math::Radians(90.0f));
		m_Camera->SetNearClip(0.1f);
		m_Camera->SetFarClip(100.0f);

		m_Scene = Scene::Create();
		m_Scene->SetActiveCamera(m_Camera);

		m_Shader = Shader::Create();
		m_Shader->AddShaderSource(EShaderType::Vertex, vertexSrc);
		m_Shader->AddShaderSource(EShaderType::Pixel, pixelSrc);

		bResult = m_Shader->Compile();
		ionassert(bResult);
		m_AuxCamera = Camera::Create();
		m_AuxCamera->SetTransform(Math::Translate(Vector3(0.0f, 0.0f, 4.0f)));
		m_AuxCamera->SetFOV(Math::Radians(66.0f));
		m_AuxCamera->SetNearClip(0.1f);
		m_AuxCamera->SetFarClip(10.0f);

		m_DirectionalLight = MakeShared<DirectionalLight>();
		m_Scene->SetActiveDirectionalLight(m_DirectionalLight.get());

		m_Light0->m_Location = Vector3(2.0f, 3.0f, -1.0f);
		m_Light0->m_Color = Vector3(1.0f, 1.0f, 1.0f);
		m_Light0->m_Intensity = 4.0f;
		m_Light0->m_Falloff = 5.0f;

		m_Light1->m_Location = Vector3(-1.0f, 1.0f, 2.0f);
		m_Light1->m_Color = Vector3(0.3f, 0.6f, 1.0f);
		m_Light1->m_Intensity = 3.0f;
		m_Light1->m_Falloff = 4.0f;

		m_Light2->m_Location = Vector3(2.0f, 1.0f, 4.0f);
		m_Light2->m_Color = Vector3(1.0f, 0.1f, 0.0f);
		m_Light2->m_Intensity = 2.0f;
		m_Light2->m_Falloff = 7.0f;

		m_Scene->AddLight(m_Light0.get());
		m_Scene->AddLight(m_Light1.get());
		m_Scene->AddLight(m_Light2.get());

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
		ionassertnd(textureImage->Load(textureFile));
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

	virtual void OnUpdate(float deltaTime) override
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

		m_Scene->SetAmbientLightColor(m_AmbientLightColor);
		m_DirectionalLightRotation = Quaternion(Math::Radians(m_DirectionalLightAngles));
		m_DirectionalLight->m_LightDirection = Math::Rotate(m_DirectionalLightRotation, Vector3(0.0f, 0.0f, -1.0f));
		m_DirectionalLight->m_Color = m_DirectionalLightColor;
		m_DirectionalLight->m_Intensity = m_DirectionalLightIntensity;
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
		m_Scene->UpdateRenderData();

		// ImGui:

		if (m_bDrawImGui)
		{
			//ImGui::ShowDemoWindow();
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
						ionassertnd(textureImage->Load(textureFile));
						m_TextureCollada = Texture::Create(textureImage);
						m_MeshCollada->GetMaterial().lock()->SetParameter("Texture", m_TextureCollada);
					}
					delete textureFile;
				}
				ImGui::DragFloat3("Aux Camera Location", &m_AuxCameraLocation.x, 0.01f, -FLT_MAX, FLT_MAX);
				if (ImGui::Button("Change Camera"))
				{
					m_Scene->SetActiveCamera(m_Scene->GetActiveCamera() == m_Camera ? m_AuxCamera : m_Camera);
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

				ImGui::DragFloat3("Light0 Location", &m_Light0->m_Location.x, 0.01f, -FLT_MAX, FLT_MAX);
				ImGui::DragFloat3("Light1 Location", &m_Light1->m_Location.x, 0.01f, -FLT_MAX, FLT_MAX);
				ImGui::DragFloat3("Light2 Location", &m_Light2->m_Location.x, 0.01f, -FLT_MAX, FLT_MAX);
			}
			ImGui::End();

			ImGui::Begin("Asset Manager");
			{
				ImGuiAssetMemoryPoolControls<EAssetType::Mesh>();
				ImGuiAssetMemoryPoolControls<EAssetType::Texture>();

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
								GetRenderer()->SetPolygonDrawMode(EPolygonDrawMode::Fill);
							}
							else if (currentDrawMode == m_DrawModes[1])
							{
								GetRenderer()->SetPolygonDrawMode(EPolygonDrawMode::Lines);
							}
							else if (currentDrawMode == m_DrawModes[2])
							{
								GetRenderer()->SetPolygonDrawMode(EPolygonDrawMode::Points);
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
					GetRenderer()->SetVSyncEnabled(!GetRenderer()->IsVSyncEnabled());
				}
			}
			ImGui::End();
		}
	}

	virtual void OnRender() override
	{
		GetRenderer()->Clear(Vector4(0.1f, 0.1f, 0.1f, 1.0f));
		GetRenderer()->RenderScene(m_Scene);

		//RPrimitiveRenderProxy triangle { };
		//triangle.VertexBuffer = m_Triangle.VB.get();
		//triangle.IndexBuffer = m_Triangle.IB.get();
		//triangle.UniformBuffer = m_Triangle.UB.get();
		//triangle.Shader = m_Triangle.Shader.get();
		//triangle.Transform = Matrix4(1.0f);
		//m_Triangle.Texture->Bind();

		//GetRenderer()->Draw(triangle, m_Scene);
	}

	virtual void OnShutdown() override
	{
	}

	virtual void OnEvent(const Event& event) override
	{
		m_EventDispatcher.Dispatch(event);
	}

	void OnKeyPressedEvent(const KeyPressedEvent& event)
	{
		if (event.GetKeyCode() == Key::F4)
		{
			m_bDrawImGui = !m_bDrawImGui;
		}
	}

	void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event)
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

	void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event)
	{
		if (event.GetMouseButton() == Mouse::Right)
		{
			ImGui::SetWindowFocus();

			GetWindow()->LockCursor();
			GetWindow()->ShowCursor(false);
		}
	}

	void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event)
	{
		if (event.GetMouseButton() == Mouse::Right)
		{
			GetWindow()->UnlockCursor();
			GetWindow()->ShowCursor(true);
			std::pair s(1, 4.0);
		}
	}

	using ExampleEventFunctions = TEventFunctionPack <
		TMemberEventFunction<IonExample, KeyPressedEvent, &OnKeyPressedEvent>,
		TMemberEventFunction<IonExample, RawInputMouseMovedEvent, &OnRawInputMouseMovedEvent>,
		TMemberEventFunction<IonExample, MouseButtonPressedEvent, &OnMouseButtonPressedEvent>,
		TMemberEventFunction<IonExample, MouseButtonReleasedEvent, &OnMouseButtonReleasedEvent>
	>;

private:
	TShared<Mesh> m_MeshCollada;
	TShared<Camera> m_Camera;
	TShared<Camera> m_AuxCamera;
	TShared<Material> m_Material;
	TShared<Texture> m_TextureCollada;
	TShared<DirectionalLight> m_DirectionalLight;
	TShared<Scene> m_Scene;
	TShared<Shader> m_Shader;

	ExampleModelData<EExampleModelName::FourPak> m_4Pak;
	ExampleModelData<EExampleModelName::Piwsko>  m_Piwsko;
	ExampleModelData<EExampleModelName::Oscypek> m_Oscypek;
	ExampleModelData<EExampleModelName::Ciupaga> m_Ciupaga;
	ExampleModelData<EExampleModelName::Slovak>  m_Slovak;
	ExampleModelData<EExampleModelName::Stress>  m_Stress;
	//ExampleModelData<EExampleModelName::BigSphere> m_BigSphere;

	Vector4 m_CameraLocation = { 0.0f, 0.0f, 2.0f, 1.0f };
	Vector3 m_CameraRotation = { 0.0f, 0.0f, 0.0f };
	Transform m_CameraTransform = { m_CameraLocation, m_CameraRotation, Vector3(1.0f) };

	Vector3 m_MeshLocation = { 0.0f, 0.0f, 0.0f };
	Vector3 m_MeshRotation = { -90.0f, 0.0f, 0.0f };
	Vector3 m_MeshScale = Vector3(1.0f);
	Transform m_MeshTransform = { m_MeshLocation, m_MeshRotation, m_MeshScale };

	Vector3 m_AuxCameraLocation = { 0.0f, 0.0f, 4.0f };

	Vector4 m_AmbientLightColor = Vector4(0.1f, 0.11f, 0.14f, 1.0f);

	Vector3 m_DirectionalLightAngles = Vector3(-30.0f, 30.0f, 0.0f);
	Vector3 m_DirectionalLightColor = Vector3(1.0f, 1.0f, 1.0f);
	float m_DirectionalLightIntensity = 1.0f;
	Quaternion m_DirectionalLightRotation = Quaternion(Math::Radians(m_DirectionalLightAngles));

	TShared<Light> m_Light0 = MakeShared<Light>();
	TShared<Light> m_Light1 = MakeShared<Light>();
	TShared<Light> m_Light2 = MakeShared<Light>();

	const char* m_DrawModes[3] = { "Triangles", "Lines", "Points" };

	char m_TextureFileNameBuffer[MAX_PATH + 1];

	bool m_bDrawImGui = true;

	EventDispatcher<ExampleEventFunctions, IonExample> m_EventDispatcher;
};

USE_APPLICATION_CLASS(IonExample);
