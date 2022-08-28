#pragma once

#include "EditorCore/EditorCore.h"
#include "Application/EnginePath.h"
#include "Engine/Components/Component.h"

#include "Resource/TextureResource.h"

namespace Ion::Editor
{
	struct EDITOR_API EditorIcon
	{
		FilePath Path;
		TSharedPtr<TextureResource> Resource;
		TRef<RHITexture> Texture;
		String VirtualPath;

		EditorIcon(const FilePath& path) :
			Path(path)
		{
		}

		EditorIcon(const String& vp) :
			VirtualPath(vp)
		{
		}

		void Load();
	};

	class EDITOR_API EditorIcons
	{
	public:
		static inline EditorIcon IconTextureResource = "[Engine]/Editor/Icons/Resource/Texture";
		static inline EditorIcon IconMeshResource    = "[Engine]/Editor/Icons/Resource/Mesh";

		static inline EditorIcon IconAsset           = "[Engine]/Editor/Icons/Asset/Asset";
		static inline EditorIcon IconDataAsset       = "[Engine]/Editor/Icons/Asset/Data";
		static inline EditorIcon IconImageAsset      = "[Engine]/Editor/Icons/Asset/Image";
		static inline EditorIcon IconMeshAsset       = "[Engine]/Editor/Icons/Asset/Mesh";

		static void LoadTextures();
	};

	class EDITOR_API EditorBillboards
	{
	public:
		static const inline String VPCircle    = "[Engine]/Editor/Icons/Circle";
		static const inline String VPLightbulb = "[Engine]/Editor/Icons/Lightbulb";
		static const inline String VPSun       = "[Engine]/Editor/Icons/Sun";
		static const inline String VPNoMesh    = "[Engine]/Editor/Icons/NoMesh";

		static inline TSharedPtr<TextureResource> ResourceBillboardCircle;
		static inline TSharedPtr<TextureResource> ResourceBillboardLightbulb;
		static inline TSharedPtr<TextureResource> ResourceBillboardSun;
		static inline TSharedPtr<TextureResource> ResourceBillboardNoMesh;

		static inline TRef<RHITexture> BillboardCircle;
		static inline TRef<RHITexture> BillboardLightbulb;
		static inline TRef<RHITexture> BillboardSun;
		static inline TRef<RHITexture> BillboardNoMesh;

		static void LoadTextures();

		static const TRef<RHITexture>& GetComponentBillboardTexture(ComponentTypeID id);
	};

	class EDITOR_API EditorMeshes
	{
	public:
		static inline std::shared_ptr<Mesh> MeshGrid;

		static inline TRef<RHIShader> ShaderGrid;

		static void Init();
	};
}
