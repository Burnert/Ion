#pragma once

#include "Component.h"

namespace Ion
{
	ENTITY_COMPONENT_CLASS_HEADER(BehaviorComponent);

	class ION_API BehaviorComponent final : public Component
	{
		ENTITY_COMPONENT_CLASS_BODY(BehaviorComponent, "Behavior")

	private:
		BehaviorComponent();
	};
}
