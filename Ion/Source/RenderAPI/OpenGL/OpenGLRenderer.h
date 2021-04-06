#pragma once

#include "Renderer/Renderer.h"
#include "OpenGL.h"

namespace Ion
{
	class ION_API OpenGLRenderer : public Renderer
	{
	public:
		OpenGLRenderer();
		virtual ~OpenGLRenderer() override;

		virtual void Init() override;

		virtual void Clear() const override;
		virtual void Clear(const FVector4& color) const override;

		virtual void SetVSyncEnabled(bool bEnabled) const override;
	};
}
