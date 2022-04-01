#pragma once

#include "Core/Core.h"

namespace Ion
{
	struct SceneObjectData
	{
		Transform RelativeTransform;
		Transform WorldTransformCache;
		uint8 bVisible : 1;
		uint8 bVisibleInGame : 1;

		inline SceneObjectData() :
			bVisible(true),
			bVisibleInGame(true)
		{
		}
	};
}
