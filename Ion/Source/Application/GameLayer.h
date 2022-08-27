#pragma once

#include "Application/Layer/Layer.h"
#include "Application/Event/EventDispatcher.h"

namespace Ion
{
	class ION_API GameLayer : public Layer
	{
	public:
		GameLayer(const char* name) :
			Layer(name), 
			m_EventDispatcher(this)
		{ }

	protected:
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(float DeltaTime) override;
		virtual void OnRender() override;

		virtual void OnEvent(const Event& event) override;

	private:
		TEventDispatcher<GameLayer> m_EventDispatcher;
	};
}
