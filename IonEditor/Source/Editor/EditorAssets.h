#pragma once

#include "EditorCore/EditorCore.h"
#include "Application/EnginePath.h"
#include "Engine/Components/Component.h"

#include "Resource/TextureResource.h"

namespace Ion::Editor
{
	struct EditorIcon
	{
		FilePath Path;
		TResourcePtr<TextureResource> Resource;
		TShared<RHITexture> Texture;

		EditorIcon(const FilePath& path) :
			Path(path)
		{
		}

		void Load();
	};

	class EDITOR_API EditorIcons
	{
	public:
		static inline EditorIcon IconTextureResource = FilePath(L"Icons/Resource/Texture.iasset");
		static inline EditorIcon IconMeshResource    = FilePath(L"Icons/Resource/Mesh.iasset");

		static inline EditorIcon IconAsset           = FilePath(L"Icons/Asset/Asset.iasset");
		static inline EditorIcon IconDataAsset       = FilePath(L"Icons/Asset/Data.iasset");
		static inline EditorIcon IconImageAsset      = FilePath(L"Icons/Asset/Image.iasset");
		static inline EditorIcon IconMeshAsset       = FilePath(L"Icons/Asset/Mesh.iasset");

		static void LoadTextures();
	};

	class EDITOR_API EditorBillboards
	{
	public:
		static const inline FilePath PathCircle    = L"Icons/Circle.iasset";
		static const inline FilePath PathLightbulb = L"Icons/Lightbulb.iasset";
		static const inline FilePath PathSun       = L"Icons/Sun.iasset";
		static const inline FilePath PathNoMesh    = L"Icons/NoMesh.iasset";

		static inline TResourcePtr<TextureResource> ResourceBillboardCircle;
		static inline TResourcePtr<TextureResource> ResourceBillboardLightbulb;
		static inline TResourcePtr<TextureResource> ResourceBillboardSun;
		static inline TResourcePtr<TextureResource> ResourceBillboardNoMesh;

		static inline TShared<RHITexture> BillboardCircle;
		static inline TShared<RHITexture> BillboardLightbulb;
		static inline TShared<RHITexture> BillboardSun;
		static inline TShared<RHITexture> BillboardNoMesh;

		static void LoadTextures();

		static const TShared<RHITexture>& GetComponentBillboardTexture(ComponentTypeID id);
	};

	class EDITOR_API EditorMeshes
	{
	public:
		static inline TShared<Mesh> MeshGrid;

		static inline TShared<RHIShader> ShaderGrid;

		static void Init();
	};
}
