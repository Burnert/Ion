#pragma once

#include "Core/Asset/AssetRegistry.h"

namespace Ion::Editor
{
	struct ExampleModelData
	{
		TShared<Mesh> Mesh;
		TShared<Material> Material;
		TShared<Texture> Texture;

		Asset MeshAsset;
		Asset TextureAsset;

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
		ionassert(data.TextureAsset->IsLoaded());
		ionassert(data.MeshAsset->IsLoaded());
		ionassert(data.Texture);
		ionassert(data.Mesh);

		data.Mesh->SetMaterial(data.Material);

		//Vector3 location;
		//location.x = Random::Float(-2.0f, 2.0f);
		//location.y = Random::Float(-0.5f, 0.5f);
		//location.z = Random::Float(-2.0f, 2.0f);
		//data.Mesh->SetTransform(Math::Translate(location) * Math::ToMat4(Quaternion(Math::Radians(Vector3(-90.0f, 90.0f, 0.0f)))));
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
			model.TextureAsset->Load([&model](const AssetData& asset)
			{
				TShared<Image> image = asset.Get<Image>();

				TextureDescription texDesc { };
				texDesc.Dimensions.Width = image->GetWidth();
				texDesc.Dimensions.Height = image->GetHeight();
				texDesc.bGenerateMips = true;
				texDesc.bUseAsRenderTarget = true;
				texDesc.bCreateSampler = true;
				model.Texture = Texture::Create(texDesc);

				model.Texture->UpdateSubresource(image.get());

				model.Material = Material::Create();
				model.Material->SetShader(Renderer::GetBasicShader());
				model.Material->CreateParameter("Texture", EMaterialParameterType::Texture2D);
				model.Material->SetParameter("Texture", model.Texture);

				model.TryInit();
			});
			model.MeshAsset->Load([&model](const AssetData& asset)
			{
				TShared<MeshAssetData> mesh = asset.Get<MeshAssetData>();

				model.Mesh = Mesh::Create();

				TShared<VertexBuffer> vb = VertexBuffer::Create(mesh->Vertices.Ptr, mesh->Vertices.Count);
				TShared<IndexBuffer> ib = IndexBuffer::Create(mesh->Indices.Ptr, (uint32)mesh->Indices.Count);
				vb->SetLayout(mesh->Layout);

				model.Mesh->SetVertexBuffer(vb);
				model.Mesh->SetIndexBuffer(ib);

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
		if (!m_bLoaded && MeshAsset->IsLoaded() && TextureAsset->IsLoaded())
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
