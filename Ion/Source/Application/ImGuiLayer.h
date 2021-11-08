#pragma once

#include "Core/Layer/Layer.h"

namespace Ion
{
	class ION_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(const char* name) :
			Layer(name),
			m_EventDispatcher(this)
		{ }

	protected:
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(float DeltaTime) override;

		virtual bool OnEvent(const Event& event) override;

		using EventFunctions = TEventFunctionPack<>;

	private:
		EventDispatcher<EventFunctions, ImGuiLayer> m_EventDispatcher;
	};
}
