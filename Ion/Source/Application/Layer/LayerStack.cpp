#include "IonPCH.h"

#include "LayerStack.h"

namespace Ion
{
	LayerStack::LayerStack() :
		m_LayerInsertIndex(0)
	{ }

	LayerStack::~LayerStack()
	{
		for (auto it = begin(); it != end();)
			(*(it++))->OnDetach();
	}

	void LayerStack::RemoveLayer(const char* name)
	{
		auto layerIt = FindLayer(name);
		ptrdiff_t layerIndex = layerIt - begin();
		if (layerIt != end())
		{
			LayerPtr layer = (*layerIt);
			layer->OnDetach();
			m_Layers.erase(layerIt);

			if (layerIndex < m_LayerInsertIndex)
			{
				m_LayerInsertIndex--;
			}
		}
	}

	LayerStack::LayerPtr LayerStack::GetLayer(const char* name)
	{
		auto layerIt = FindLayer(name);
		if (layerIt != end())
			return *layerIt;
		else
			return std::shared_ptr<Layer>(nullptr);
	}

	void LayerStack::SetEnabled(const char* name, bool bEnabled)
	{
		if (LayerPtr layer = GetLayer(name))
		{
			layer->SetEnabled(bEnabled);
		}
	}

	void LayerStack::OnUpdate(float DeltaTime)
	{
		TRACE_FUNCTION();

		for (LayerPtr layer : m_Layers)
		{
			if (layer->m_bEnabled)
			{
				layer->OnUpdate(DeltaTime);
			}
		}
	}

	void LayerStack::OnRender()
	{
		TRACE_FUNCTION();

		for (LayerPtr layer : m_Layers)
		{
			if (layer->m_bEnabled)
			{
				layer->OnRender();
			}
		}
	}

	void LayerStack::OnEvent(const Event& event)
	{
		TRACE_FUNCTION();

		for (auto it = rbegin(); it != rend();)
		{
			LayerPtr layer = (*(it++));
			if (layer->m_bEnabled)
			{
				// Layer::PropagateEvent() cannot be called from outside of the OnEvent method's scope.
				ionassert(!layer->m_bPropagateCurrentEvent);
				ionassert(!layer->m_bCurrentEventHandled);

				layer->OnEvent(event);
				// Don't propagate if handled, unless specifically told to.
				if (layer->m_bCurrentEventHandled && !layer->m_bPropagateCurrentEvent)
					break;

				layer->m_bPropagateCurrentEvent = false;
			}
		}
	}

	LayerStack::LayerIterator LayerStack::FindLayer(const char* name)
	{
		return std::find_if(begin(), end(), [name](LayerPtr& layer)
		{
			return strcmp(layer->GetName(), name) == 0;
		});
	}
}
