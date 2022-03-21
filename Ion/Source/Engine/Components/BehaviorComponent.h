#pragma once

#include "Component.h"

namespace Ion
{
	class ION_API BehaviorComponent final : public Component
	{
		ENTITY_COMPONENT_CLASS_BODY("Behavior Component")

		//void COMPCALLBACKFUNC OnCreate();

	private:
		BehaviorComponent();
	};

	template<>
	struct ComponentTypeDefaults<BehaviorComponent>
	{
		static constexpr const char* Name = "BehaviorComponent";
	};
}
