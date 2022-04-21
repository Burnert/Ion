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
