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
		virtual void OnRender() { };
		virtual void OnShutdown() { };
		virtual void OnEvent(const Event& event) { };

		void Exit();
		void SetApplicationTitle(const WString& title);

		void SetCursor(ECursorType cursor);
		ECursorType GetCurrentCursor() const;

		Application* GetEngineApplication() const;

		const TShared<GenericWindow>& GetWindow();
		const TShared<InputManager>& GetInputManager();
		LayerStack* GetLayerStack();

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

	inline const TShared<GenericWindow>& App::GetWindow()
	{
		return GetEngineApplication()->GetWindow();
	}

	inline const TShared<InputManager>& App::GetInputManager()
	{
		return GetEngineApplication()->GetInputManager();
	}

	inline LayerStack* App::GetLayerStack()
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
}
