#include "IonPCH.h"

#include "OpenGLRenderer.h"

namespace Ion
{
	OpenGLRenderer::OpenGLRenderer()
	{
	}

	OpenGLRenderer::~OpenGLRenderer()
	{
	}

	void OpenGLRenderer::Init()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Bind the default VAO and forget about it for the rest of the world.
		// Maybe I'll implement it in some way in the future but I don't think it's necessary.
		// @TODO: Think about it!
		uint vao;
		glCreateVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	void OpenGLRenderer::Clear() const
	{
		Clear(FVector4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void OpenGLRenderer::Clear(const FVector4& color) const
	{
		glClearColor(color.x, color.y, color.z, color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderer::Draw(const TShared<IDrawable>& drawable) const
	{
		drawable->PrepareForDraw();
		glDrawElements(GL_TRIANGLES, drawable->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderer::SetVSyncEnabled(bool bEnabled) const
	{
		OpenGL::SetSwapInterval((int)bEnabled);
	}

	bool OpenGLRenderer::GetVSyncEnabled() const
	{
		return (bool)OpenGL::GetSwapInterval();
	}

	void OpenGLRenderer::SetViewportDimensions(const SViewportDimensions& dimensions) const
	{
		glViewport(dimensions.X, dimensions.Y, dimensions.Width, dimensions.Height);
	}

	SViewportDimensions OpenGLRenderer::GetViewportDimensions() const
	{
		SViewportDimensions dimensions;
		glGetIntegerv(GL_VIEWPORT, (int*)&dimensions);
		return dimensions;
	}

	void OpenGLRenderer::SetPolygonDrawMode(EPolygonDrawMode drawMode) const
	{
		glPolygonMode(GL_FRONT_AND_BACK, PolygonDrawModeToGLPolygonMode(drawMode));
	}

	EPolygonDrawMode OpenGLRenderer::GetPolygonDrawMode() const
	{
		int polygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
		return GLPolygonModeToPolygonDrawMode(polygonMode);
	}
}
