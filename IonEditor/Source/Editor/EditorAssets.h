#pragma once

#include "EditorCore/EditorCore.h"
#include "Application/EnginePath.h"
#include "Engine/Components/Component.h"

namespace Ion::Editor
{
	class EDITOR_API EditorBillboards
	{
	public:
		static const inline FilePath PathCircle    = L"Icons/Circle.iasset";
		static const inline FilePath PathLightbulb = L"Icons/Lightbulb.iasset";
		static const inline FilePath PathSun       = L"Icons/Sun.iasset";
		static const inline FilePath PathNoMesh    = L"Icons/NoMesh.iasset";

		static inline TShared<Texture> BillboardCircle;
		static inline TShared<Texture> BillboardLightbulb;
		static inline TShared<Texture> BillboardSun;
		static inline TShared<Texture> BillboardNoMesh;

		static void LoadTextures();

		static const TShared<Texture>& GetComponentBillboardTexture(ComponentTypeID id);
	};

	class EDITOR_API EditorMeshes
	{
	public:
		static inline TShared<Mesh> MeshGrid;

		static inline TShared<Shader> ShaderGrid;

		static void Init();
	};
}
