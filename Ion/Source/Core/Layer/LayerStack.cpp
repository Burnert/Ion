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

	void LayerStack::RemoveLayer(const CStr name)
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

	LayerStack::LayerPtr LayerStack::GetLayer(const CStr name)
	{
		auto layerIt = FindLayer(name);
		if (layerIt != end())
			return *layerIt;
		else
			return Shared<Layer>(nullptr);
	}

	void LayerStack::SetEnabled(const CStr name, bool bEnabled)
	{
		if (LayerPtr layer = GetLayer(name))
		{
			layer->SetEnabled(bEnabled);
		}
	}

	void LayerStack::OnUpdate(float DeltaTime)
	{
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
		for (LayerPtr layer : m_Layers)
		{
			if (layer->m_bEnabled)
			{
				layer->OnRender();
			}
		}
	}

	void LayerStack::OnEvent(Event& event)
	{
		for (auto it = rbegin(); it != rend();)
		{
			LayerPtr layer = (*(it++));
			if (layer->m_bEnabled)
			{
				// Layer::PropagateEvent() cannot be called from outside of the OnEvent method's scope.
				ASSERT(!layer->m_bPropagateCurrentEvent)

				// Don't propagate if handled, unless specifically told to.
				if (layer->OnEvent(event) && !layer->m_bPropagateCurrentEvent)
					break;

				layer->m_bPropagateCurrentEvent = false;
			}
		}
	}

	LayerStack::LayerIterator LayerStack::FindLayer(const CStr name)
	{
		return std::find_if(begin(), end(), [name](LayerPtr& layer)
		{
			return strcmp(layer->GetName(), name) == 0;
		});
	}
}
