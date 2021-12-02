#pragma once

#include "OpenGL.h"
#include "Renderer/Texture.h"

namespace Ion
{
	class ION_API OpenGLTexture : public Texture
	{
	public:
		virtual ~OpenGLTexture() override;

		virtual void Bind(uint32 slot = 0) const override;
		virtual void Unbind() const override;

		int32 GetBoundSlot() const { return m_BoundSlot; }

	protected:
		OpenGLTexture(FileOld* file);
		OpenGLTexture(Image* image);
		OpenGLTexture(AssetHandle asset);

	private:
		void CreateTexture(const void* const pixelData, uint32 width, uint32 height);

	private:
		uint32 m_ID;
		mutable int32 m_BoundSlot;

		friend class Texture;
		friend class OpenGLRenderer;
	};
}
