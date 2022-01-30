#pragma once

#include "Core/Event/Event.h"

namespace Ion
{
	class ION_API Layer
	{
	public:
		using LayerPtr = TShared<Layer>;

		Layer(const char* name);
		virtual ~Layer();

		/* A disabled layer will not get updated and rendered.
		   Also it will not receive any events. */
		void SetEnabled(bool bEnabled);

		FORCEINLINE const char* GetName() const { return m_Name; }

	protected:
		/* Override */
		virtual void OnAttach() { }
		/* Override */
		virtual void OnDetach() { }
		/* Override */
		virtual void OnUpdate(float DeltaTime) { }
		/* Override */
		virtual void OnRender() { }
		/* Override */
		virtual void OnEvent(const Event& event) { }

		/* This function can be called only inside OnEvent's body.
		   Call this if you want to propagate the event down the layer stack
		   even if it was successfully handled. */
		void PropagateEvent();
		/* Call in OnEvent, if the event has been handled.
		   If the function is not called, the event will be propagated down the layer stack. */
		void EventHandled();

	private:
		// @TODO: Replace with a GUID
		static uint32 s_LayerIDCounter;

		const char* m_Name;
		uint32 m_ID;
		bool m_bEnabled;
		bool m_bPropagateCurrentEvent;
		bool m_bCurrentEventHandled;

		friend class LayerStack;
	};
}
