#include "EditorPCH.h"

#include "EditorAssets.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/DirectionalLightComponent.h"
#include "Engine/Components/SceneComponent.h"
#include "Engine/Components/MeshComponent.h"

#include "RHI/Texture.h"
#include "RHI/Shader.h"
#include "RHI/RHI.h"
#include "Renderer/Mesh.h"

#include "Asset/AssetRegistry.h"

#include "Resource/TextureResource.h"

namespace Ion::Editor
{
	static void LoadTexture(TResourceRef<TextureResource>& outResource, TRef<RHITexture>& texture, const String& vp)
	{
		Asset asset = Asset::Resolve(vp).UnwrapOr(Asset::None);
		
		outResource = TextureResource::Query(asset);

		outResource->Take([&texture](const TextureResourceRenderData& data)
		{
			texture = data.Texture;
		});
	}

	void EditorIcon::Load()
	{
		LoadTexture(Resource, Texture, VirtualPath);
	}

	void EditorIcons::LoadTextures()
	{
		IconTextureResource.Load();
		IconMeshResource.Load();

		IconAsset.Load();
		IconDataAsset.Load();
		IconImageAsset.Load();
		IconMeshAsset.Load();
	}

	void EditorBillboards::LoadTextures()
	{
		LoadTexture(ResourceBillboardCircle,    BillboardCircle,    VPCircle);
		LoadTexture(ResourceBillboardLightbulb, BillboardLightbulb, VPLightbulb);
		LoadTexture(ResourceBillboardSun,       BillboardSun,       VPSun);
		LoadTexture(ResourceBillboardNoMesh,    BillboardNoMesh,    VPNoMesh);
	}

	const TRef<RHITexture>& EditorBillboards::GetComponentBillboardTexture(ComponentTypeID id)
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

		std::shared_ptr<RHIVertexLayout> gridLayout = std::make_shared<RHIVertexLayout>(2);
		gridLayout->AddAttribute(EVertexAttributeSemantic::Position, EVertexAttributeType::Float, 3, false);
		gridLayout->AddAttribute(EVertexAttributeSemantic::TexCoord, EVertexAttributeType::Float, 2, false);
		gridLayout->AddAttribute(EVertexAttributeSemantic::Normal, EVertexAttributeType::Float, 3, true);

		std::shared_ptr<RHIVertexBuffer> gridVB = RHIVertexBuffer::CreateShared(gridVertices, sizeof(gridVertices) / sizeof(float));
		gridVB->SetLayout(gridLayout);
		gridVB->SetLayoutShader(EditorMeshes::ShaderGrid);

		std::shared_ptr<RHIIndexBuffer> gridIB = RHIIndexBuffer::CreateShared(gridIndices, sizeof(gridIndices) / sizeof(uint32));

		EditorMeshes::MeshGrid = Mesh::Create();
		EditorMeshes::MeshGrid->SetVertexBuffer(gridVB);
		EditorMeshes::MeshGrid->SetIndexBuffer(gridIB);
	}

	static void CreateGridShader()
	{
		String vertexSrc;
		String pixelSrc;

		FilePath shadersPath = EnginePath::GetShadersPath();

		// @TODO: This needs a refactor
		if (RHI::GetCurrent() != ERHI::OpenGL)
		{
			vertexSrc = File::ReadToString(shadersPath + "Editor/EditorGridVS.hlsl").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "Editor/EditorGridPS.hlsl").Unwrap();
		}
		else
		{
			vertexSrc = File::ReadToString(shadersPath + "Basic.vert").Unwrap();
			pixelSrc  = File::ReadToString(shadersPath + "Basic.frag").Unwrap();
		}

		EditorMeshes::ShaderGrid = RHIShader::Create();
		EditorMeshes::ShaderGrid->AddShaderSource(EShaderType::Vertex, vertexSrc);
		EditorMeshes::ShaderGrid->AddShaderSource(EShaderType::Pixel, pixelSrc);

		EditorMeshes::ShaderGrid->Compile()
			.Err<ShaderCompilationError>([](auto& err) { EditorLogger.Error("Could not compile the Editor Grid Shader."); })
			.Unwrap();
	}

	void EditorMeshes::Init()
	{
		CreateGridShader();
		CreateGridMesh();
	}
}
