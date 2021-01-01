#pragma once

#include "Event.h"

namespace Ion
{
	// Engine Events

	class ION_API EngineTickEvent : public Event
	{
	public:
		FORCEINLINE float GetDeltaTime() const { return m_DeltaTime; }

		EngineTickEvent(float deltaTime) :
			m_DeltaTime(deltaTime) {}

		SET_EVENT_CATEGORY(EC_Application | EC_Engine)
		SET_EVENT_TYPE(EngineTick)
		SET_EVENT_TOSTRING_FORMAT("{ deltaTime: " << m_DeltaTime << " }")

	private:
		float m_DeltaTime;
	};


	class ION_API EngineRenderEvent : public Event
	{
	public:
		EngineRenderEvent() {}

		SET_EVENT_CATEGORY(EC_Application | EC_Engine)
		SET_EVENT_TYPE(EngineRender)
	};
}
