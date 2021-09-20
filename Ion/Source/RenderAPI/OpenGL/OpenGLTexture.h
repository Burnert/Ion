#pragma once

#include "OpenGL.h"
#include "Renderer/Texture.h"

namespace Ion
{
	class ION_API OpenGLTexture : public Texture
	{
		friend class Texture;
	public:
		virtual ~OpenGLTexture() override;

		virtual void Bind(uint slot = 0) const override;
		virtual void Unbind() const override;

	protected:
		OpenGLTexture(File* file);
		OpenGLTexture(Image* image);

	private:
		void CreateTexture();

	private:
		uint m_ID;
	};
}