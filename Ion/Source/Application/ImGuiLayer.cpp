#include "IonPCH.h"

#include "ImGuiLayer.h"

namespace Ion
{
	void ImGuiLayer::OnAttach()
	{
	}

	void ImGuiLayer::OnDetach()
	{
	}

	void ImGuiLayer::OnUpdate(float DeltaTime)
	{
	}

	bool ImGuiLayer::OnEvent(const Event& event)
	{
		//m_EventDispatcher.Dispatch(event);
		return false;
	}
}
