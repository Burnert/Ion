#pragma once

#include "Core/Event/Event.h"

namespace Ion
{
	class ION_API Layer
	{
		friend class LayerStack;
	public:
		using LayerPtr = std::shared_ptr<Layer>;

		Layer(const CStr name);
		virtual ~Layer();

		/* A disabled layer will not get updated and rendered.
		   Also it will not receive any events. */
		void SetEnabled(bool bEnabled);

		FORCEINLINE const CStr GetName() const { return m_Name; }

	protected:
		virtual void OnAttach();
		virtual void OnDetach();

		virtual void OnUpdate(float DeltaTime);
		virtual void OnRender();

		/* Return false in the implementation if the event could not be handled.
		   The event will then be propagated down the layer stack.
		   Else return true. */
		virtual bool OnEvent(Event& event);

		/* This function can be called only inside OnEvent's body.
		   Call this if you want to propagate the event down the layer stack
		   even if it was successfully handled. */
		void PropagateEvent();

	private:
		static uint s_LayerCount;

		uint m_ID;
		const CStr m_Name;
		bool m_bEnabled = true;
		bool m_bPropagateCurrentEvent = false;
	};
}
