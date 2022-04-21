#pragma once

#include "Core/Asset/AssetRegistry.h"
#include "Core/Resource/MeshResource.h"
#include "Core/Resource/TextureResource.h"

namespace Ion::Editor
{
	struct ExampleModelData
	{
		TShared<Mesh> Mesh;
		TShared<Material> Material;
		TShared<RHITexture> Texture;

		Asset MeshAsset;
		Asset TextureAsset;

		TShared<MeshResource> MeshResource;
		TShared<TextureResource> TextureResource;

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
			return m_bLoaded;
		}
	};
	inline TArray<ExampleModelData> g_ExampleModels;
	inline DirectionalLight* g_ExampleLight;

	static void InitExampleModel(ExampleModelData& data)
	{
		ionassert(data.Texture);
		ionassert(data.Mesh);

		data.Mesh->SetMaterial(data.Material);
	}

	static void CreateExampleModels()
	{
		g_ExampleModels.resize(6);

		g_ExampleModels[0].Name            = "4Pak";
		g_ExampleModels[0].MeshAsset       = AssetFinder(FilePath(L"../IonExample/Assets/models/4pak.iasset")).Resolve();
		g_ExampleModels[0].TextureAsset    = AssetFinder(FilePath(L"../IonExample/Assets/textures/4pak.iasset")).Resolve();

		g_ExampleModels[1].Name            = "Piwsko";
		g_ExampleModels[1].MeshAsset       = AssetFinder(FilePath(L"../IonExample/Assets/models/piwsko.iasset")).Resolve();
		g_ExampleModels[1].TextureAsset    = AssetFinder(FilePath(L"../IonExample/Assets/textures/piwsko.iasset")).Resolve();

		g_ExampleModels[2].Name            = "Oscypek";
		g_ExampleModels[2].MeshAsset       = AssetFinder(FilePath(L"../IonExample/Assets/models/oscypek.iasset")).Resolve();
		g_ExampleModels[2].TextureAsset    = AssetFinder(FilePath(L"../IonExample/Assets/textures/oscypek.iasset")).Resolve();

		g_ExampleModels[3].Name            = "Ciupaga";
		g_ExampleModels[3].MeshAsset       = AssetFinder(FilePath(L"../IonExample/Assets/models/ciupaga.iasset")).Resolve();
		g_ExampleModels[3].TextureAsset    = AssetFinder(FilePath(L"../IonExample/Assets/textures/ciupaga.iasset")).Resolve();

		g_ExampleModels[4].Name            = "Slovak";
		g_ExampleModels[4].MeshAsset       = AssetFinder(FilePath(L"../IonExample/Assets/models/slovak.iasset")).Resolve();
		g_ExampleModels[4].TextureAsset    = AssetFinder(FilePath(L"../IonExample/Assets/textures/slovak.iasset")).Resolve();

		g_ExampleModels[5].Name            = "Stress";
		g_ExampleModels[5].MeshAsset       = AssetFinder(FilePath(L"../IonExample/spherestresstest_uv.iasset")).Resolve();
		g_ExampleModels[5].TextureAsset    = AssetFinder(FilePath(L"../IonExample/Assets/test_4k.iasset")).Resolve();
	}

	static void LoadExampleModels()
	{
		for (ExampleModelData& model : g_ExampleModels)
		{
			model.MeshResource = MeshResource::Query(model.MeshAsset);
			model.TextureResource = TextureResource::Query(model.TextureAsset);

			model.Mesh = Mesh::Create();

			model.Material = Material::Create();
			model.Material->SetShader(Renderer::GetBasicShader());
			model.Material->CreateParameter("Texture", EMaterialParameterType::Texture2D);

			model.MeshResource->Take([&model](const MeshResourceRenderData& renderData)
			{
				model.Mesh->SetVertexBuffer(renderData.VertexBuffer);
				model.Mesh->SetIndexBuffer(renderData.IndexBuffer);

				model.TryInit();
			});

			model.TextureResource->Take([&model](const TextureResourceRenderData& renderData)
			{
				model.Texture = renderData.Texture;

				model.Material->SetParameter("Texture", model.Texture);

				model.TryInit();
			});
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
