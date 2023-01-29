#pragma once

#include "Matter/Object.h"

namespace Ion
{
	REGISTER_LOGGER(MComponentLogger, "Engine::MComponent");

	class ION_API MComponent : public MObject
	{
		MCLASS(MComponent)
		using Super = MObject;

	public:
		MComponent();

	protected:
		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void Tick(float deltaTime) override;

	private:
	};
}
