#pragma once

#include "Core/CoreApi.h"
#include "Application/Application.h"
#include "Core/Input/Input.h"

namespace Ion
{
	class ION_API WindowsApplication : public Application
	{
	public:
		void InitWindows(HINSTANCE hInstance);

		FORCEINLINE static HINSTANCE GetHInstance() { return m_HInstance; }

	protected:
		virtual void PollEvents() override;

		virtual void Update(float DeltaTime) override;

	private:
		static HINSTANCE m_HInstance;
	};
}

// Platform specific type of the Application class
using IonApplication = Ion::WindowsApplication;
