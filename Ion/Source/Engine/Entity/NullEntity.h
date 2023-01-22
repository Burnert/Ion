#pragma once

#include "Entity.h"

namespace Ion
{
	class ION_API MNullEntity : public MEntity
	{
		MCLASS(MNullEntity)
		using Super = MEntity;

	public:
		MNullEntity();
	};
}
