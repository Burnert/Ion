#include "IonPCH.h"

#include "Event.h"

namespace Ion
{
	Shared<Event> Event::MakeShared()
	{
		ASSERT(m_bDefer)
		return Shared<Event>(this);
	}
}
