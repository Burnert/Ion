#pragma once

#include "Matter/Object.h"

namespace Ion
{
	class ION_API MComponent : public MObject
	{
		MCLASS(MComponent)
		using Super = MObject;

	public:
		MComponent();

		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void Tick(float deltaTime) override;

	private:
	};
}
