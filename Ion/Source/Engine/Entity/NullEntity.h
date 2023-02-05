#pragma once

#include "Entity.h"

namespace Ion
{
	class ION_API MNullEntity : public MEntity
	{
	public:
		MCLASS(MNullEntity)
		using Super = MEntity;

	public:
		MNullEntity();
	};
}
