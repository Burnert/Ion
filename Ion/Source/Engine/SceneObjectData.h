#pragma once

#include "Core/Serialization/Archive.h"

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

		FORCEINLINE friend Archive& operator<<(Archive& ar, SceneObjectData& sod)
		{
			SERIALIZE_BIT_FIELD(ar, sod.bVisible);
			SERIALIZE_BIT_FIELD(ar, sod.bVisibleInGame);
			return ar;
		}
	};
}
