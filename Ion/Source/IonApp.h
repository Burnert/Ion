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

		Application* GetEngineApp() const;
		const std::shared_ptr<GenericWindow>& GetWindow() const;
		LayerStack* GetLayerStack() const;

		float GetGlobalDeltaTime() const;
	};

	FORCEINLINE void IApp::Exit()
	{
		g_pEngineApplication->Exit();
	}

	FORCEINLINE Application* IApp::GetEngineApp() const
	{
		return g_pEngineApplication;
	}

	FORCEINLINE float IApp::GetGlobalDeltaTime() const
	{
		return GetEngineApp()->GetGlobalDeltaTime();
	}

	FORCEINLINE const std::shared_ptr<GenericWindow>& IApp::GetWindow() const
	{
		return GetEngineApp()->GetWindow();
	}

	FORCEINLINE LayerStack* IApp::GetLayerStack() const
	{
		return GetEngineApp()->GetLayerStack();
	}
}
