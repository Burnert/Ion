#include "EditorPCH.h"

#include "EditorAssets.h"
#include "Engine/Components/LightComponent.h"
#include "Engine/Components/DirectionalLightComponent.h"
#include "Engine/Components/SceneComponent.h"

#include "Renderer/Texture.h"

#include "Core/Asset/AssetManager.h"

namespace Ion::Editor
{
	static void LoadTexture(TShared<Texture>& texture, const FilePath& path)
	{
		AssetHandle textureAsset = AssetManager::CreateAsset(EAssetType::Texture, path);
		textureAsset->AssignEvent([&, path](const OnAssetLoadedMessage& msg)
		{
			AssetDescription::Texture* assetDesc = msg.RefPtr->GetDescription<EAssetType::Texture>();

			TextureDescription desc { };
			desc.Dimensions.Width = assetDesc->Width;
			desc.Dimensions.Height = assetDesc->Height;
			desc.bUseAsRenderTarget = true;
			desc.bCreateSampler = true;
			//desc.InitialData = msg.PoolLocation;
			desc.DebugName = StringConverter::WStringToString(path.LastElement());
			texture = Texture::Create(desc);

			Image texImage((uint8*)msg.PoolLocation, assetDesc->Width, assetDesc->Height, assetDesc->NumChannels);
			texture->UpdateSubresource(&texImage);
		});

		textureAsset->LoadAssetData();
	}

	void EditorBillboards::LoadTextures()
	{
		LoadTexture(BillboardCircle,    EnginePath::GetEditorContentPath() + PathCircle);
		LoadTexture(BillboardLightbulb, EnginePath::GetEditorContentPath() + PathLightbulb);
		LoadTexture(BillboardSun,       EnginePath::GetEditorContentPath() + PathSun);
	}

	const TShared<Texture>& EditorBillboards::GetComponentBillboardTexture(ComponentTypeID id)
	{
		if (id == LightComponent::GetTypeID())
			return BillboardLightbulb;
		if (id == DirectionalLightComponent::GetTypeID())
			return BillboardSun;

		return BillboardCircle;
	}
}
