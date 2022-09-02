#pragma once

#include "Ion.h"
#include "Application/Application.h"

namespace Ion
{
	class ION_API NOVTABLE IApp
	{
	public:
		virtual void OnInit() { }
		virtual void OnUpdate(float deltaTime) { }
		virtual void PostUpdate() { }
		virtual void OnRender() { }
		virtual void OnShutdown() { }
		virtual void OnEvent(const Event& event) { }

		void Exit();
		void SetApplicationTitle(const WString& title);

		void SetCursor(ECursorType cursor);
		ECursorType GetCurrentCursor() const;

		EngineFonts& GetEngineFonts();

		Application* GetEngineApplication() const;

		const std::shared_ptr<GenericWindow>& GetWindow() const;
		const std::shared_ptr<InputManager>& GetInputManager() const;
		LayerStack* GetLayerStack() const;

		float GetGlobalDeltaTime() const;
	};

	inline void IApp::Exit()
	{
		g_pEngineApplication->Exit();
	}

	inline void IApp::SetApplicationTitle(const WString& title)
	{
		g_pEngineApplication->SetApplicationTitle(title);
	}

	inline Application* IApp::GetEngineApplication() const
	{
		return g_pEngineApplication;
	}

	inline float IApp::GetGlobalDeltaTime() const
	{
		return GetEngineApplication()->GetGlobalDeltaTime();
	}

	inline const std::shared_ptr<GenericWindow>& IApp::GetWindow() const
	{
		return GetEngineApplication()->GetWindow();
	}

	inline const std::shared_ptr<InputManager>& IApp::GetInputManager() const
	{
		return GetEngineApplication()->GetInputManager();
	}

	inline LayerStack* IApp::GetLayerStack() const
	{
		return GetEngineApplication()->GetLayerStack();
	}

	inline void IApp::SetCursor(ECursorType cursor)
	{
		GetEngineApplication()->SetCursor(cursor);
	}

	inline ECursorType IApp::GetCurrentCursor() const
	{
		return GetEngineApplication()->GetCurrentCursor();
	}

	inline EngineFonts& IApp::GetEngineFonts()
	{
		return GetEngineApplication()->GetEngineFonts();
	}
}
