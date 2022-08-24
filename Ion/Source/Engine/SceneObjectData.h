#pragma once

namespace Ion
{
	struct SceneObjectData
	{
		uint8 bVisible : 1;
		uint8 bVisibleInGame : 1;

		inline SceneObjectData() :
			bVisible(true),
			bVisibleInGame(true)
		{
		}
	};
}
