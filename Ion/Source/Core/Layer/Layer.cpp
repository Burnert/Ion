#include "IonPCH.h"

#include "Layer.h"

namespace Ion
{
	Layer::Layer(const char* name) :
		m_ID(s_LayerCount++),
		m_Name(name)
	{ }

	Layer::~Layer() { }

	void Layer::OnAttach()
	{
		ION_LOG_ENGINE_DEBUG("Layer {0} Attached", m_Name);
	}

	void Layer::OnDetach()
	{
		ION_LOG_ENGINE_DEBUG("Layer {0} Detached", m_Name);
	}

	void Layer::OnUpdate(float DeltaTime)
	{
		ION_LOG_ENGINE_DEBUG("Layer {0} Update", m_Name);
	}

	void Layer::OnRender()
	{
		ION_LOG_ENGINE_DEBUG("Layer {0} Render", m_Name);
	}

	bool Layer::OnEvent(Event& event)
	{
		return false;
	}

	void Layer::PropagateEvent()
	{
		m_bPropagateCurrentEvent = true;
	}

	void Layer::SetEnabled(bool bEnabled)
	{
		m_bEnabled = bEnabled;
	}

	uint Layer::s_LayerCount = 0;
}
