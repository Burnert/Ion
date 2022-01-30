#include "IonPCH.h"

#include "Layer.h"

namespace Ion
{
	Layer::Layer(const char* name) :
		m_Name(name),
		m_ID(s_LayerIDCounter++),
		m_bEnabled(true),
		m_bCurrentEventHandled(false),
		m_bPropagateCurrentEvent(false)
	{ }

	Layer::~Layer() { }

	void Layer::PropagateEvent()
	{
		m_bPropagateCurrentEvent = true;
	}

	void Layer::SetEnabled(bool bEnabled)
	{
		m_bEnabled = bEnabled;
	}

	uint32 Layer::s_LayerIDCounter = 0;
}
