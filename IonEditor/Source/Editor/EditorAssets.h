#pragma once

#include "EditorCore/EditorCore.h"
#include "Application/EnginePath.h"
#include "Engine/Components/Component.h"

namespace Ion::Editor
{
	class EDITOR_API EditorBillboards
	{
	public:
		static const inline FilePath PathCircle    = L"Icons/Circle.png";
		static const inline FilePath PathLightbulb = L"Icons/Lightbulb.png";
		static const inline FilePath PathSun       = L"Icons/Sun.png";
		static const inline FilePath PathNoMesh    = L"Icons/NoMesh.png";

		static inline TShared<Texture> BillboardCircle;
		static inline TShared<Texture> BillboardLightbulb;
		static inline TShared<Texture> BillboardSun;
		static inline TShared<Texture> BillboardNoMesh;

		static void LoadTextures();

		static const TShared<Texture>& GetComponentBillboardTexture(ComponentTypeID id);
	};
}
