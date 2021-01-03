#pragma once

#include "Core/CoreTypes.h"

#include "Layer.h"

namespace Ion
{
	class ION_API LayerStack
	{
	public:
		using LayerPtr = std::shared_ptr<Layer>;
		using LayerVec = std::vector<LayerPtr>;

		using LayerIterator             = LayerVec::iterator;
		using LayerReverseIterator      = LayerVec::reverse_iterator;
		using LayerConstIterator        = LayerVec::const_iterator;
		using LayerConstReverseIterator = LayerVec::const_reverse_iterator;

		LayerStack();
		~LayerStack();

		/* Creates and pushes a layer object with specified name and parameters into the layer stack */
		template<typename LayerT, typename... Types>
		std::enable_if_t<std::is_base_of_v<Layer, LayerT>, LayerPtr>
			PushLayer(const CStr name, Types&&... args)
		{
			LayerPtr layer = std::make_shared<LayerT>(name, args...);
			layer->OnAttach();
			LayerIterator layerIt = m_Layers.insert(begin() + m_LayerInsertIndex, std::move(layer));
			m_LayerInsertIndex++;

			return *layerIt;
		}

		/* Creates and pushes a layer object (overlay) with specified name and parameters on top of other layers */
		template<typename LayerT, typename... Types>
		std::enable_if_t<std::is_base_of_v<Layer, LayerT>, LayerPtr>
			PushOverlayLayer(const CStr name, Types&&... args)
		{
			LayerPtr overlay = std::make_shared<LayerT>(name, args...);
			overlay->OnAttach();
			m_Layers.push_back(std::move(overlay));

			return *(end() - 1);
		}

		/* Removes a layer based on its name */
		void RemoveLayer(const CStr name);

		/* Finds a layer and returns a shared pointer to it.
		   Don't call this every frame. */
		LayerPtr GetLayer(const CStr name);

		/* Finds and sets layer enabled state.
		   A disabled layer will not get updated and rendered.
		   Also it will not receive any events.
		   Don't call this every frame. */
		void SetEnabled(const CStr name, bool bEnabled);

		void OnUpdate(float DeltaTime);
		void OnRender();
		void OnEvent(Event& event);

		// Iterators

		LayerIterator begin()                     { return m_Layers.begin();  }
		LayerIterator end()                       { return m_Layers.end();    }
		LayerReverseIterator rbegin()             { return m_Layers.rbegin(); }
		LayerReverseIterator rend()               { return m_Layers.rend();   }
												  
		LayerConstIterator begin() const          { return m_Layers.begin();  }
		LayerConstIterator end() const            { return m_Layers.end();    }
		LayerConstReverseIterator rbegin() const  { return m_Layers.rbegin(); }
		LayerConstReverseIterator rend() const    { return m_Layers.rend();   }

	private:
		LayerVec m_Layers;
		uint m_LayerInsertIndex;

		LayerIterator FindLayer(const CStr name);
	};
}