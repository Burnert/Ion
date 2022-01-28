#pragma once

namespace Ion
{
namespace Editor
{
	struct ExampleModelData
	{
		TShared<Mesh> Mesh;
		TShared<Material> Material;
		TShared<Texture> Texture;

		AssetHandle MeshAsset;
		AssetHandle TextureAsset;

		String Name;

	private:
		TFunction<void()> m_OnInit;

		bool m_bLoaded = false;

	public:
		void TryInit();
		template<typename Lambda>
		void SetOnInit(Lambda onInit)
		{
			m_OnInit = onInit;
		}
	};
	inline TArray<ExampleModelData> g_ExampleModels;

	static void InitExampleModel(ExampleModelData& data)
	{
		// Texture
		AssetDescription::Texture* texAssetDesc = data.TextureAsset->GetDescription<EAssetType::Texture>();
		TextureDescription texDesc { };
		texDesc.Dimensions.Width = texAssetDesc->Width;
		texDesc.Dimensions.Height = texAssetDesc->Height;
		texDesc.bGenerateMips = true;
		texDesc.bUseAsRenderTarget = true;
		data.Texture = Texture::Create(texDesc);

		Image texImage((uint8*)data.TextureAsset->Data(), texAssetDesc->Width, texAssetDesc->Height, texAssetDesc->NumChannels);
		data.Texture->UpdateSubresource(&texImage);

		// Material
		data.Material = Material::Create();
		data.Material->SetShader(Renderer::GetBasicShader());
		data.Material->CreateParameter("Texture", EMaterialParameterType::Texture2D);
		data.Material->SetParameter("Texture", data.Texture);

		// Mesh
		data.Mesh = Mesh::Create();
		data.Mesh->LoadFromAsset(data.MeshAsset);
		data.Mesh->SetMaterial(data.Material);
		data.Mesh->SetTransform(Math::Translate(Vector3(0.0f, 0.0f, -2.0f)) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 90.0f, 0.0f)))));
	}

	static void CreateExampleModels()
	{
		g_ExampleModels.resize(6);

		g_ExampleModels[0].Name            = "4Pak";
		g_ExampleModels[0].MeshAsset       = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"../IonExample/Assets/models/4pak.dae"));
		g_ExampleModels[0].TextureAsset    = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"../IonExample/Assets/textures/4pak.png"));

		g_ExampleModels[1].Name            = "Piwsko";
		g_ExampleModels[1].MeshAsset       = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"../IonExample/Assets/models/piwsko.dae"));
		g_ExampleModels[1].TextureAsset    = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"../IonExample/Assets/textures/piwsko.png"));

		g_ExampleModels[2].Name            = "Oscypek";
		g_ExampleModels[2].MeshAsset       = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"../IonExample/Assets/models/oscypek.dae"));
		g_ExampleModels[2].TextureAsset    = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"../IonExample/Assets/textures/oscypek.png"));

		g_ExampleModels[3].Name            = "Ciupaga";
		g_ExampleModels[3].MeshAsset       = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"../IonExample/Assets/models/ciupaga.dae"));
		g_ExampleModels[3].TextureAsset    = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"../IonExample/Assets/textures/ciupaga.png"));

		g_ExampleModels[4].Name            = "Slovak";
		g_ExampleModels[4].MeshAsset       = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"../IonExample/Assets/models/slovak.dae"));
		g_ExampleModels[4].TextureAsset    = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"../IonExample/Assets/textures/slovak.png"));

		g_ExampleModels[5].Name            = "Stress";
		g_ExampleModels[5].MeshAsset       = AssetManager::CreateAsset(EAssetType::Mesh,    FilePath(L"../IonExample/spherestresstest_uv.dae"));
		g_ExampleModels[5].TextureAsset    = AssetManager::CreateAsset(EAssetType::Texture, FilePath(L"../IonExample/Assets/test_4k.png"));

		for (ExampleModelData& model : g_ExampleModels)
		{
			model.MeshAsset->AssignEvent(
				[&model](const OnAssetLoadedMessage&) { model.TryInit(); });
			model.TextureAsset->AssignEvent(
				[&model](const OnAssetLoadedMessage&) { model.TryInit(); });
		}
	}

	static void LoadExampleModels()
	{
		for (ExampleModelData& model : g_ExampleModels)
		{
			model.MeshAsset->LoadAssetData();
			model.TextureAsset->LoadAssetData();
		}
	}

	inline void ExampleModelData::TryInit()
	{
		if (!m_bLoaded && MeshAsset->IsLoaded() && TextureAsset->IsLoaded())
		{
			InitExampleModel(*this);
			m_OnInit();
			m_bLoaded = true;
		}
	}
}
}
