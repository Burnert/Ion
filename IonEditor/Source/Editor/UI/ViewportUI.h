#pragma once

#include "EditorCore/CoreApi.h"

namespace Ion::Editor
{
	class EditorViewport;

	class EDITOR_API EditorUIViewport
	{
	public:
		EditorUIViewport(EditorViewport* owner, int32 index);

		void Draw();

		void SetOpen(bool bOpen);

		void SetWindowName(const String& name);

		const UVector2& GetSize() const;
		bool CanCapture() const;
		bool IsMouseInRect() const;

		bool& GetWindowOpenFlagRef();

	private:
		void Capture();
		void Release();

		template<typename Lambda>
		void SetOnResize(Lambda onResize);
		void DispatchOnResize(const UVector2& size);

	private:
		EditorViewport* m_Owner;

		TFunction<void(const UVector2&)> m_OnResize;

		String m_WindowName;

		UVector2 m_Size;
		Vector4 m_Rect;

		uint32 m_Index;

		bool m_bWindowOpen;
		bool m_bHovered;
		bool m_bCaptured;

		friend class EditorViewport;
	};

	inline const UVector2& EditorUIViewport::GetSize() const
	{
		return m_Size;
	}

	inline bool& EditorUIViewport::GetWindowOpenFlagRef()
	{
		return m_bWindowOpen;
	}

	template<typename Lambda>
	inline void EditorUIViewport::SetOnResize(Lambda onResize)
	{
		m_OnResize = onResize;
	}
}
