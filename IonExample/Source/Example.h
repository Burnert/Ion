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

#include "IonApp.h"
#include "Renderer/Renderer.h"
#include "UserInterface/ImGui.h"

#include "Core/File/Collada.h"
#include "Core/Asset/Asset.h"

#include "Engine/Engine.h"
#include "Engine/Entity/MeshEntity.h"

#include "RHI/RHI.h"

using namespace Ion;

class IonExample : public Ion::App
{
public:
	IonExample();

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
		TShared<MaterialOld> Material;
		TShared<RHITexture> Texture;

		Asset MeshAsset;
		Asset TextureAsset;

		String Name;
	};

	template<EExampleModelName Type>
	static void InitExampleModel(ExampleModelData<Type>& data, const TShared<RHIShader>& shader, World* world, const Matrix4& transform)
	{
		// Texture
		AssetDescription::Texture* texAssetDesc = data.TextureAsset->GetDescription<EAssetType::Texture>();
		TextureDescription texDesc { };
		texDesc.Dimensions.Width = texAssetDesc->Width;
		texDesc.Dimensions.Height = texAssetDesc->Height;
		texDesc.bGenerateMips = true;
		texDesc.bCreateSampler = true;
		texDesc.bUseAsRenderTarget = true;
		data.Texture = RHITexture::Create(texDesc);

		Image texImage((uint8*)data.TextureAsset->Data(), texAssetDesc->Width, texAssetDesc->Height, texAssetDesc->NumChannels);
		data.Texture->UpdateSubresource(&texImage);

		// Material
		data.Material = MaterialOld::Create();
		data.Material->SetShader(shader);
		data.Material->CreateParameter("Texture", EMaterialParameterType::Texture2D);
		data.Material->SetParameter("Texture", data.Texture);

		// Mesh
		data.Mesh = Mesh::Create();
		data.Mesh->LoadFromAsset(data.MeshAsset);
		data.Mesh->SetMaterial(data.Material);
		data.Mesh->SetTransform(transform);

		MeshEntity* entity = world->SpawnEntityOfClass<MeshEntity>();
		entity->SetMesh(data.Mesh);
	}

	void CreateExampleAssets();

	void LoadExampleAssets();

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

		TShared<RHIShader> shader = Renderer::Get()->GetBasicShader();

		if constexpr (N == EExampleModelName::FourPak)
		{
			// 4Pak
			InitExampleModel(data, shader, m_World,
				Math::Translate(Vector3(2.0f, 1.0f, 0.0f)) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 0.0f, 0.0f)))));
		}
		if constexpr (N == EExampleModelName::Piwsko)
		{
			// Piwsko
			InitExampleModel(data, shader, m_World,
				Math::Translate(Vector3(-2.0f, 1.0f, 0.0f))* Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 0.0f, 0.0f)))));
		}
		if constexpr (N == EExampleModelName::Oscypek)
		{
			// Oscypek
			InitExampleModel(data, shader, m_World,
				Math::Translate(Vector3(-1.0f, 1.0f, 1.0f))* Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 0.0f, 0.0f)))));
		}
		if constexpr (N == EExampleModelName::Ciupaga)
		{
			// Ciupaga
			InitExampleModel(data, shader, m_World,
				Math::Translate(Vector3(1.0f, 1.0f, 1.0f))* Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 90.0f, 0.0f)))));
		}
		if constexpr (N == EExampleModelName::Slovak)
		{
			// Slovak
			InitExampleModel(data, shader, m_World,
				Math::Translate(Vector3(1.0f, 0.0f, -2.0f))* Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 180.0f, 0.0f)))));
		}
		if constexpr (N == EExampleModelName::Stress)
		{
			// Kula
			InitExampleModel(data, shader, m_World,
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
		TShared<RHIVertexBuffer> VB;
		TShared<IndexBuffer> IB;
		TShared<RHIUniformBuffer> UB;
		TShared<RHIShader> Shader;
		TShared<RHITexture> Texture;
	} m_Triangle;

	template<EExampleModelName Type>
	void ImGuiExampleModelOps(ExampleModelData<Type>& model)
	{
		//ImGui::PushID(model.Name.c_str());

		//if (!model.MeshAsset->IsLoaded() && !model.MeshAsset->IsLoading())
		//	ImGui::TextDisabled(model.Name.c_str());
		//else
		//	ImGui::TextColored(model.MeshAsset->IsLoading() ? ImVec4(1.0f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), model.Name.c_str());

		//ImGui::SameLine(70);
		//if (ImGui::Button("Load"))
		//{
		//	model.MeshAsset->LoadAssetData();
		//	model.TextureAsset->LoadAssetData();
		//} ImGui::SameLine();
		//if (ImGui::Button("Unload"))
		//{
		//	model.MeshAsset->UnloadAssetData();
		//	model.TextureAsset->UnloadAssetData();
		//}
		//ImGui::PopID();
	}

	//template<EAssetType Type>
	//void ImGuiAssetMemoryPoolControls()
	//{
	//	ImGui::PushID(AssetTypeToString(Type));
	//	ImGui::Text("%s Pool", AssetTypeToString(Type));
	//	ImGui::SameLine(100);
	//	if (ImGui::Button("Print"))
	//	{
	//		AssetManager::PrintAssetPool<Type>();
	//	}
	//	ImGui::SameLine();
	//	if (ImGui::Button("Defragment"))
	//	{
	//		AssetManager::DefragmentAssetPool<Type>();
	//	}
	//	ImGui::SameLine();
	//	ImGui::Text("Size: %.2f MB", AssetManager::GetAssetMemoryPool<Type>().GetSize() / (float)(1 << 20));
	//	ImGui::PopID();
	//}

	virtual void OnInit() override;

	virtual void OnUpdate(float deltaTime) override;

	virtual void OnRender() override;

	virtual void OnShutdown() override;

	virtual void OnEvent(const Event& event) override;

	void OnKeyPressedEvent(const KeyPressedEvent& event);
	void OnRawInputMouseMovedEvent(const RawInputMouseMovedEvent& event);
	void OnMouseButtonPressedEvent(const MouseButtonPressedEvent& event);
	void OnMouseButtonReleasedEvent(const MouseButtonReleasedEvent& event);
	void OnWindowResizeEvent(const WindowResizeEvent& event);

	void CreateRenderTargetTexture(TextureDimensions dimensions);

	using ExampleEventFunctions = TEventFunctionPack <
		TMemberEventFunction<IonExample, KeyPressedEvent, &OnKeyPressedEvent>,
		TMemberEventFunction<IonExample, RawInputMouseMovedEvent, &OnRawInputMouseMovedEvent>,
		TMemberEventFunction<IonExample, MouseButtonPressedEvent, &OnMouseButtonPressedEvent>,
		TMemberEventFunction<IonExample, MouseButtonReleasedEvent, &OnMouseButtonReleasedEvent>,
		TMemberEventFunction<IonExample, WindowResizeEvent, &OnWindowResizeEvent>
	>;

private:
	TShared<Mesh> m_MeshCollada;
	TShared<Camera> m_Camera;
	TShared<Camera> m_AuxCamera;
	TShared<MaterialOld> m_Material;
	TShared<RHITexture> m_TextureCollada;
	World* m_World;

	ExampleModelData<EExampleModelName::FourPak> m_4Pak;
	ExampleModelData<EExampleModelName::Piwsko>  m_Piwsko;
	ExampleModelData<EExampleModelName::Oscypek> m_Oscypek;
	ExampleModelData<EExampleModelName::Ciupaga> m_Ciupaga;
	ExampleModelData<EExampleModelName::Slovak>  m_Slovak;
	ExampleModelData<EExampleModelName::Stress>  m_Stress;
	//ExampleModelData<EExampleModelName::BigSphere> m_BigSphere;

	TShared<RHITexture> m_RenderTarget;
	TShared<RHITexture> m_DepthStencil;

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

	const char* m_DrawModes[3] = { "Triangles", "Lines", "Points" };

	char m_TextureFileNameBuffer[261];

	bool m_bDrawImGui = true;

	EventDispatcher<ExampleEventFunctions, IonExample> m_EventDispatcher;
};
