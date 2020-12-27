#pragma once

#include "Core/CoreTypes.h"
#include "Core/CoreApi.h"

#include "Core/Event/Event.h"

namespace Ion
{
	class ION_API GenericWindow
	{
	public:
		using EventPtr = std::shared_ptr<Event>;
		using EventCallback = std::function<void(EventPtr)>;

		virtual ~GenericWindow();

		virtual bool Initialize();

		virtual void Show();
		virtual void Hide();

		virtual void SetTitle(const WCStr title);

		virtual void SetEnabled(bool bEnabled);

		virtual void Resize();
		virtual bool GetDimensions(int& width, int& height) const;

		virtual void SetEventCallback(EventCallback callback);

		// Implemented per platform.
		static std::shared_ptr<GenericWindow> Create();

	protected:
		// Protected constructor: Only shared_ptrs of this class can be made.
		GenericWindow();

		// Function that gets called every time an event occurs.
		EventCallback m_EventCallback;
	};

	
}

