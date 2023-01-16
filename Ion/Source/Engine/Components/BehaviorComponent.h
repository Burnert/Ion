#pragma once

#include "ComponentOld.h"

namespace Ion
{
	ENTITY_COMPONENT_CLASS_HEADER(BehaviorComponent);

	class ION_API BehaviorComponent final : public ComponentOld
	{
		ENTITY_COMPONENT_CLASS_BODY(BehaviorComponent, "Behavior")

	private:
		BehaviorComponent();
	};
}
