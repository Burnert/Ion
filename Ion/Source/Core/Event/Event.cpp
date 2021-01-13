#include "IonPCH.h"

#include "Event.h"

namespace Ion
{
	Shared<Event> Event::AsShared()
	{
		ASSERT(m_bDefer)
		return Shared<Event>(this);
	}
}
