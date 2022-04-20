#include "EditorPCH.h"

#include "EditorAssets.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/DirectionalLightComponent.h"
#include "Engine/Components/SceneComponent.h"
#include "Engine/Components/MeshComponent.h"

#include "Renderer/Texture.h"
#include "Renderer/Mesh.h"
#include "Renderer/Shader.h"

#include "RHI/RHI.h"

#include "Core/Asset/AssetRegistry.h"

namespace Ion::Editor
{
	static void LoadTexture(TShared<Texture>& texture, const FilePath& path)
	{
		//AssetHandle textureAsset = AssetManager::CreateAsset(EAssetType::Texture, path);
		Asset textureAsset = AssetFinder(path).Resolve();
		textureAsset->Load([&, path](const AssetData& asset)
		{
			TShared<Image> image = asset.Get<Image>();

			TextureDescription desc { };
			desc.Dimensions.Width = image->GetWidth();
			desc.Dimensions.Height = image->GetHeight();
			desc.bUseAsRenderTarget = true;
			desc.bCreateSampler = true;
			//desc.InitialData = msg.PoolLocation;
			desc.DebugName = StringConverter::WStringToString(path.LastElement());
			desc.MagFilter = ETextureFilteringMethod::Linear;
			desc.MinFilter = ETextureFilteringMethod::Linear;
			texture = Texture::Create(desc);

			texture->UpdateSubresource(image.get());
		});
	}

	void EditorBillboards::LoadTextures()
	{
		LoadTexture(BillboardCircle,    EnginePath::GetEditorContentPath() + PathCircle);
		LoadTexture(BillboardLightbulb, EnginePath::GetEditorContentPath() + PathLightbulb);
		LoadTexture(BillboardSun,       EnginePath::GetEditorContentPath() + PathSun);
		LoadTexture(BillboardNoMesh,    EnginePath::GetEditorContentPath() + PathNoMesh);
	}

	const TShared<Texture>& EditorBillboards::GetComponentBillboardTexture(ComponentTypeID id)
	{
		if (id == LightComponent::GetTypeID())
			return BillboardLightbulb;
		if (id == DirectionalLightComponent::GetTypeID())
			return BillboardSun;
		if (id == MeshComponent::GetTypeID())
			return BillboardNoMesh;

		return BillboardCircle;
	}

	static void CreateGridMesh()
	{
		float gridVertices[] = {
			/*   location               texcoord    normal       */
				-100.0f, 0.0f, -100.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
				 100.0f, 0.0f, -100.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
				 100.0f, 0.0f,  100.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
				-100.0f, 0.0f,  100.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		};

		uint32 gridIndices[] = {
			0, 2, 1,
			2, 0, 3,
			0, 1, 2,
			2, 3, 0,
		};

		TShared<VertexLayout> gridLayout = MakeShared<VertexLayout>(2);
		gridLayout->AddAttribute(EVertexAttributeSemantic::Position, EVertexAttributeType::Float, 3, false);
		gridLayout->AddAttribute(EVertexAttributeSemantic::TexCoord, EVertexAttributeType::Float, 2, false);
		gridLayout->AddAttribute(EVertexAttributeSemantic::Normal, EVertexAttributeType::Float, 3, true);

		TShared<VertexBuffer> gridVB = VertexBuffer::Create(gridVertices, sizeof(gridVertices) / sizeof(float));
		gridVB->SetLayout(gridLayout);
		gridVB->SetLayoutShader(EditorMeshes::ShaderGrid);

		TShared<IndexBuffer> gridIB = IndexBuffer::Create(gridIndices, sizeof(gridIndices) / sizeof(uint32));

		EditorMeshes::MeshGrid = Mesh::Create();
		EditorMeshes::MeshGrid->SetVertexBuffer(gridVB);
		EditorMeshes::MeshGrid->SetIndexBuffer(gridIB);
	}

	static void CreateGridShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetCheckedShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() == ERHI::DX11)
		{
			File::ReadToString(shadersPath + L"Editor/EditorGridVS.hlsl", vertexSrc);
			File::ReadToString(shadersPath + L"Editor/EditorGridPS.hlsl", pixelSrc);
		}
		else
		{
			File::ReadToString(shadersPath + L"Basic.vert", vertexSrc);
			File::ReadToString(shadersPath + L"Basic.frag", pixelSrc);
		}

		EditorMeshes::ShaderGrid = Shader::Create();
		EditorMeshes::ShaderGrid->AddShaderSource(EShaderType::Vertex, vertexSrc);
		EditorMeshes::ShaderGrid->AddShaderSource(EShaderType::Pixel, pixelSrc);

		if (!EditorMeshes::ShaderGrid->Compile())
		{
			LOG_ERROR("Could not compile the Editor Grid Shader.");
			debugbreak();
		}
	}

	void EditorMeshes::Init()
	{
		CreateGridShader();
		CreateGridMesh();
	}
}
