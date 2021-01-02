#include "IonPCH.h"

#include "Event.h"

namespace Ion
{
	std::shared_ptr<Event> Event::MakeShared()
	{
		ASSERT(m_bDefer)
		return std::shared_ptr<Event>(this);
	}
}
