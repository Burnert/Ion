#pragma once

#include "RenderCommand.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "Light.h"
#include "Camera.h"
#include "Scene.h"

#include "Drawable.h"

namespace Ion
{
	struct ViewportDimensions
	{
		int32 X;
		int32 Y;
		int32 Width;
		int32 Height;
	};

	enum class EPolygonDrawMode : uint8
	{
		Fill,
		Lines,
		Points,
	};

	class ION_API RenderCommandQueue
	{
	public:
		void PushCommand(RenderCommand& command);
		void PushCommand(RenderCommandEx& command);

		void Flush();

	private:
		void ExecuteCommands();
		void Execute(RenderCommand& command);
		void FreeCommand(RenderCommand& command);

	private:
		TArray<RenderCommand> m_RenderCommands;
		TArray<RenderCommandEx> m_RenderCommandsEx;

		Mutex m_QueueMutex;
	};

	class ION_API Renderer
	{
	public:
		static Renderer* Create();

		static Renderer* Get()
		{
			return s_Instance;
		}

		virtual ~Renderer() { };

		virtual void Init() = 0;

		virtual void Clear() const = 0;
		virtual void Clear(const Vector4& color) const = 0;

		virtual void Draw(const RPrimitiveRenderProxy& primitive, const RSceneProxy& targetScene) const = 0;

		virtual void SetCurrentScene(const Scene* scene) = 0;
		virtual const Scene* GetCurrentScene() const = 0;

		virtual void RenderScene(const RSceneProxy& scene) = 0;

		virtual void SetVSyncEnabled(bool bEnabled) const = 0;
		virtual bool IsVSyncEnabled() const = 0;

		virtual void SetViewportDimensions(const ViewportDimensions& dimensions) const = 0;
		virtual ViewportDimensions GetViewportDimensions() const = 0;

		virtual void SetPolygonDrawMode(EPolygonDrawMode drawMode) const = 0;
		virtual EPolygonDrawMode GetPolygonDrawMode() const = 0;

		bool IsVSyncEnabledAtomic() const
		{
			return m_bVSyncEnabled;
		}
		ViewportDimensions GetViewportDimensionsAtomic() const
		{
			return m_ViewportDimensions;
		}
		EPolygonDrawMode GetPolygonDrawModeAtomic() const
		{
			return m_PolygonDrawMode;
		}

		void RenderFrame();

	protected:
		Renderer() { }

		friend struct RenderCommand;
		friend class RenderCommandQueue;

		static void PushRenderCommand(RenderCommand& command);
		static void PushRenderCommand(RenderCommandEx& command);

		friend class Application;
		void UpdateAtomicVariables();

	private:
		static Renderer* s_Instance;

		RenderCommandQueue m_RenderCommandQueue;

		TAtomic<bool> m_bVSyncEnabled;
		TAtomic<ViewportDimensions> m_ViewportDimensions;
		TAtomic<EPolygonDrawMode> m_PolygonDrawMode;
	};
}
