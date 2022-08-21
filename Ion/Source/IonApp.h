#pragma once

#include "Ion.h"
#include "Application/Application.h"

namespace Ion
{
	class ION_API App
	{
	public:
		App();

		virtual void OnInit() { };
		virtual void OnUpdate(float deltaTime) { };
		virtual void PostUpdate() { };
		virtual void OnRender() { };
		virtual void OnShutdown() { };
		virtual void OnEvent(const Event& event) { };

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

	private:
		Application* m_EngineApplication;

		friend Application* InstantiateApplication();
	};

	inline App::App() :
		m_EngineApplication(nullptr)
	{
	}

	inline void App::Exit()
	{
		m_EngineApplication->Exit();
	}

	inline void App::SetApplicationTitle(const WString& title)
	{
		m_EngineApplication->SetApplicationTitle(title);
	}

	inline Application* App::GetEngineApplication() const
	{
		return m_EngineApplication;
	}

	inline float App::GetGlobalDeltaTime() const
	{
		return GetEngineApplication()->GetGlobalDeltaTime();
	}

	inline const std::shared_ptr<GenericWindow>& App::GetWindow() const
	{
		return GetEngineApplication()->GetWindow();
	}

	inline const std::shared_ptr<InputManager>& App::GetInputManager() const
	{
		return GetEngineApplication()->GetInputManager();
	}

	inline LayerStack* App::GetLayerStack() const
	{
		return GetEngineApplication()->GetLayerStack();
	}

	inline void App::SetCursor(ECursorType cursor)
	{
		GetEngineApplication()->SetCursor(cursor);
	}

	inline ECursorType App::GetCurrentCursor() const
	{
		return GetEngineApplication()->GetCurrentCursor();
	}

	inline EngineFonts& App::GetEngineFonts()
	{
		return GetEngineApplication()->GetEngineFonts();
	}
}
