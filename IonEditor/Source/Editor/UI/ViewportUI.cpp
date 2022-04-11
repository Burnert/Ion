#include "EditorPCH.h"

#include "ViewportUI.h"
#include "UserInterface/ImGui.h"

#include "Renderer/Renderer.h"

#include "EditorApplication/EditorApplication.h"
#include "Editor/Viewport/EditorViewport.h"

namespace Ion::Editor
{
	EditorUIViewport::EditorUIViewport(EditorViewport* owner, int32 index) :
		m_Owner(owner),
		m_WindowName("Viewport"),
		m_Size({ }),
		m_Rect({ }),
		m_Index(index),
		m_bWindowOpen(false),
		m_bHovered(false),
		m_bCaptured(false)
	{
	}

	void EditorUIViewport::Draw()
	{
		TRACE_FUNCTION();

		m_bHovered = false;

		if (m_bWindowOpen)
		{
			ImGuiWindowFlags windowFlags =
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoNavFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f, 1.0f));

			if (m_bCaptured)
				ImGui::SetNextWindowFocus();

			String windowName = m_WindowName + "##" + ToString(m_Index);
			bool bOpen = ImGui::Begin(windowName.c_str(), &m_bWindowOpen, windowFlags);
			ImGui::PopStyleVar(2);
			if (bOpen)
			{
				// Save the viewport rect for later
				m_Rect = ImGui::GetWindowWorkRect();
				m_bHovered = ImGui::IsWindowHovered();

				UVector2 lastSize = m_Size;
				m_Size = UVector2(m_Rect.z - m_Rect.x, m_Rect.w - m_Rect.y);
				if (lastSize != m_Size)
				{
					DispatchOnResize(m_Size);
				}

				TShared<Texture> viewportFramebuffer = m_Owner->GetViewportFramebuffer();
				if (viewportFramebuffer)
				{
					const TextureDimensions& viewportDimensions = viewportFramebuffer->GetDimensions();
					ImGui::Image(viewportFramebuffer->GetNativeID(),
						ImVec2((float)viewportDimensions.Width, (float)viewportDimensions.Height));

					if (ImGui::BeginDragDropTarget())
					{
						ImGuiDragDropFlags dndFlags = ImGuiDragDropFlags_None;
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Ion_DND_InsertEntity", dndFlags))
						{
							ionassert(payload->DataSize == sizeof(DNDInsertEntityData));

							DNDInsertEntityData& data = *(DNDInsertEntityData*)payload->Data;
							data.Instantiate(EditorApplication::Get()->GetEditorWorld());
						}
						ImGui::EndDragDropTarget();
					}
					if (ImGui::IsItemHovered())
					{
						EditorApplication::Get()->SetCursor(ECursorType::Cross);
					}
					if (ImGui::IsItemClicked())
					{
						ImVec2 mousePos = ImGui::GetMousePos();
						Vector4 workRect = ImGui::GetWindowWorkRect();
						Vector2 viewportPos = { mousePos.x - workRect.x, mousePos.y - workRect.y };
						m_Owner->Click(viewportPos);
					}
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						if (CanCapture())
							Capture();
					}
					if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
					{
						Release();
					}
				}
			}

			ImGui::End();
		}
	}

	void EditorUIViewport::SetOpen(bool bOpen)
	{
		m_bWindowOpen = bOpen;
	}

	void EditorUIViewport::SetWindowName(const String& name)
	{
		m_WindowName = name;
	}

	bool EditorUIViewport::CanCapture() const
	{
		return m_bHovered && IsMouseInRect();
	}

	bool EditorUIViewport::IsMouseInRect() const
	{
		IVector2 cursorPos = InputManager::GetCursorPosition();
		return
			// @TODO: Get some IsPointInRect function goin' (like PtInRect)
			cursorPos.x >= m_Rect.x && cursorPos.x <= m_Rect.z &&
			cursorPos.y >= m_Rect.y && cursorPos.y <= m_Rect.w;
	}

	void EditorUIViewport::Capture()
	{
		if (!m_bCaptured)
		{
			m_bCaptured = true;
			m_Owner->SetCaptureState(true);
		}
	}

	void EditorUIViewport::Release()
	{
		if (m_bCaptured)
		{
			m_bCaptured = false;
			m_Owner->SetCaptureState(false);
		}
	}

	void EditorUIViewport::DispatchOnResize(const UVector2& size)
	{
		if (m_OnResize)
			m_OnResize(m_Size);
	}
}
