#pragma once

#include "Core/CoreTypes.h"

#include "Layer.h"

namespace Ion
{
	class ION_API LayerStack
	{
	public:
		using LayerPtr = TShared<Layer>;
		using LayerVec = TArray<LayerPtr>;

		using LayerIterator             = LayerVec::iterator;
		using LayerReverseIterator      = LayerVec::reverse_iterator;
		using LayerConstIterator        = LayerVec::const_iterator;
		using LayerConstReverseIterator = LayerVec::const_reverse_iterator;

		LayerStack();
		~LayerStack();

		/* Creates and pushes a layer object with specified name and parameters into the layer stack */
		template<typename LayerT, typename... Types>
		TShared<LayerT> PushLayer(const char* name, Types&&... args)
		{
			LayerPtr layer = MakeShared<LayerT>(name, args...);
			layer->OnAttach();
			LayerIterator layerIt = m_Layers.emplace(begin() + m_LayerInsertIndex, Move(layer));
			m_LayerInsertIndex++;

			return TStaticCast<LayerT>(*layerIt);
		}

		/* Creates and pushes a layer object (overlay) with specified name and parameters on top of other layers */
		template<typename LayerT, typename... Types>
		TShared<LayerT> PushOverlayLayer(const char* name, Types&&... args)
		{
			LayerPtr overlay = MakeShared<LayerT>(name, args...);
			overlay->OnAttach();
			LayerPtr& layerPtr = m_Layers.emplace_back(Move(overlay));

			return TStaticCast<LayerT>(layerPtr);
		}

		/* Removes a layer based on its name */
		void RemoveLayer(const char* name);

		/* Finds a layer and returns a shared pointer to it.
		   Don't call this every frame. */
		LayerPtr GetLayer(const char* name);

		/* Finds and sets layer enabled state.
		   A disabled layer will not get updated and rendered.
		   Also it will not receive any events.
		   Don't call this every frame. */
		void SetEnabled(const char* name, bool bEnabled);

		void OnUpdate(float DeltaTime);
		void OnRender();
		void OnEvent(const Event& event);

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
		uint32 m_LayerInsertIndex;

		LayerIterator FindLayer(const char* name);
	};
}
