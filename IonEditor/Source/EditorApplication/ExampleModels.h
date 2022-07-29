#pragma once

#include "Core/Asset/AssetRegistry.h"

#include "Resource/MeshResource.h"
#include "Resource/TextureResource.h"

namespace Ion::Editor
{
	struct ExampleModelData
	{
		TShared<Mesh> Mesh;
		TShared<MaterialOld> Material;
		TShared<RHITexture> Texture;

		Asset MeshAsset;
		Asset TextureAsset;
		Asset MaterialAsset;

		TResourcePtr<MeshResource> MeshResource;
		TResourcePtr<TextureResource> TextureResource;

		String Name;

	private:
		TArray<TFunction<void(ExampleModelData&)>> m_OnInit;

		bool m_bLoaded = false;

	public:
		void TryInit();
		template<typename Lambda>
		void SetOnInit(Lambda onInit)
		{
			m_OnInit.push_back(onInit);
		}
		bool IsLoaded() const
		{
			return MeshResource->IsLoaded() && TextureResource->IsLoaded();
		}
	};
	inline TArray<ExampleModelData> g_ExampleModels;
	inline DirectionalLight* g_ExampleLight;

	static void InitExampleModel(ExampleModelData& data)
	{
		ionassert(data.Texture);
		ionassert(data.Mesh);
	}

	static void CreateExampleModels()
	{
		g_ExampleModels.resize(5);

		g_ExampleModels[0].Name            = "4Pak";
		g_ExampleModels[0].MeshAsset       = Asset::Resolve("[Engine]/../../IonExample/Assets/models/4pak");
		g_ExampleModels[0].TextureAsset    = Asset::Resolve("[Engine]/../../IonExample/Assets/textures/4pak");
		g_ExampleModels[0].MaterialAsset   = Asset::Resolve("[Engine]/../../IonExample/Assets/Materials/4pak");

		g_ExampleModels[1].Name            = "Piwsko";
		g_ExampleModels[1].MeshAsset       = Asset::Resolve("[Engine]/../../IonExample/Assets/models/piwsko");
		g_ExampleModels[1].TextureAsset    = Asset::Resolve("[Engine]/../../IonExample/Assets/textures/piwsko");
		g_ExampleModels[1].MaterialAsset   = Asset::Resolve("[Engine]/../../IonExample/Assets/Materials/piwsko");

		g_ExampleModels[2].Name            = "Oscypek";
		g_ExampleModels[2].MeshAsset       = Asset::Resolve("[Engine]/../../IonExample/Assets/models/oscypek");
		g_ExampleModels[2].TextureAsset    = Asset::Resolve("[Engine]/../../IonExample/Assets/textures/oscypek");
		g_ExampleModels[2].MaterialAsset   = Asset::Resolve("[Engine]/../../IonExample/Assets/Materials/oscypek");

		g_ExampleModels[3].Name            = "Ciupaga";
		g_ExampleModels[3].MeshAsset       = Asset::Resolve("[Engine]/../../IonExample/Assets/models/ciupaga");
		g_ExampleModels[3].TextureAsset    = Asset::Resolve("[Engine]/../../IonExample/Assets/textures/ciupaga");
		g_ExampleModels[3].MaterialAsset   = Asset::Resolve("[Engine]/../../IonExample/Assets/Materials/ciupaga");

		g_ExampleModels[4].Name            = "Slovak";
		g_ExampleModels[4].MeshAsset       = Asset::Resolve("[Engine]/../../IonExample/Assets/models/slovak");
		g_ExampleModels[4].TextureAsset    = Asset::Resolve("[Engine]/../../IonExample/Assets/textures/slovak");
		g_ExampleModels[4].MaterialAsset   = Asset::Resolve("[Engine]/../../IonExample/Assets/Materials/slovak");

		//g_ExampleModels[5].Name            = "Stress";
		//g_ExampleModels[5].MeshAsset       = AssetFinder(FilePath(L"../IonExample/spherestresstest_uv.iasset")).Resolve();
		//g_ExampleModels[5].TextureAsset    = AssetFinder(FilePath(L"../IonExample/Assets/test_4k.iasset")).Resolve();
	}

	static void LoadExampleModels()
	{
		for (ExampleModelData& model : g_ExampleModels)
		{
			//model.MeshResource = MeshResource::Query(model.MeshAsset);
			//model.TextureResource = TextureResource::Query(model.TextureAsset);

			//model.Mesh = Mesh::CreateFromResource(model.MeshResource);

			//model.Material = Material::Create();
			//model.Material->SetShader(Renderer::GetBasicShader());
			//model.Material->CreateParameter("Texture", EMaterialParameterType::Texture2D);

			//model.Mesh->SetMaterial(model.Material);

			//model.TextureResource->Take([&model](const TextureResourceRenderData& renderData)
			//{
			//	model.Texture = renderData.Texture;

			//	model.Material->SetParameter("Texture", model.Texture);
			//});
		}
	}

	static void AddModelsToSceneOnInit(Scene* scene)
	{
		for(ExampleModelData& model : g_ExampleModels)
		{
			model.SetOnInit([scene](ExampleModelData& model)
			{
				//scene->AddDrawableObject(model.Mesh.get());
			});
		}
	}

	static void InitLights(Scene* scene)
	{
		scene->SetAmbientLightColor(Vector4(1.0f, 1.0f, 0.8f, 0.2f));

		g_ExampleLight = new DirectionalLight;
		g_ExampleLight->m_LightDirection = Math::Normalize(Vector3(1.0f, -1.0f, 1.0f));
		g_ExampleLight->m_Color = Vector3(1.0f, 0.97f, 0.92f);
		g_ExampleLight->m_Intensity = 1.0f;
		scene->SetActiveDirectionalLight(g_ExampleLight);
	}

	static void InitExample(Scene* scene)
	{
		CreateExampleModels();
		//AddModelsToSceneOnInit(scene);
		LoadExampleModels();
		if (scene)
			InitLights(scene);
	}

	template<typename Lambda>
	static void GetModelDeferred(ExampleModelData& model, Lambda onInit)
	{
		if (model.IsLoaded())
		{
			onInit(model);
		}
		else
		{
			model.SetOnInit(onInit);
		}
	}

	inline void ExampleModelData::TryInit()
	{
		if (!m_bLoaded && Mesh->GetVertexBufferRaw() && Mesh->GetIndexBufferRaw() && Texture)
		{
			InitExampleModel(*this);
			for (auto& func : m_OnInit)
			{
				checked_call(func, *this);
			}
			m_bLoaded = true;
		}
	}
}
