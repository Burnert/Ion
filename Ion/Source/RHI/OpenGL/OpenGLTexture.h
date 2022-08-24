#pragma once

#include "OpenGL.h"
#include "RHI/Texture.h"

namespace Ion
{
	class ION_API OpenGLTexture : public RHITexture
	{
	public:
		virtual ~OpenGLTexture() override;

		virtual Result<void, RHIError> SetDimensions(TextureDimensions dimensions) override;
		virtual Result<void, RHIError> UpdateSubresource(Image* image) override;

		virtual Result<void, RHIError> Bind(uint32 slot = 0) const override;
		virtual Result<void, RHIError> Unbind() const override;

		virtual Result<void, RHIError> CopyTo(const TRef<RHITexture>& destination) const override;
		virtual Result<void, RHIError> Map(void*& outBuffer, int32& outLineSize, ETextureMapType mapType) override;
		virtual Result<void, RHIError> Unmap() override;

		virtual void* GetNativeID() const override;

		int32 GetBoundSlot() const { return m_BoundSlot; }

		inline static constexpr GLint SelectGLFilterType(ETextureFilteringMethod filter, bool bMips, ETextureFilteringMethod mipFilter)
		{
			switch (filter)
			{
				case ETextureFilteringMethod::Linear:
				{
					if (!bMips) return GL_LINEAR;

					switch (mipFilter)
					{
						case ETextureFilteringMethod::Linear:  return GL_LINEAR_MIPMAP_LINEAR;
						case ETextureFilteringMethod::Nearest: return GL_LINEAR_MIPMAP_NEAREST;
					}
					break;
				}
				case ETextureFilteringMethod::Nearest:
				{
					if (!bMips) return GL_NEAREST;

					switch (mipFilter)
					{
						case ETextureFilteringMethod::Linear:  return GL_NEAREST_MIPMAP_LINEAR;
						case ETextureFilteringMethod::Nearest: return GL_NEAREST_MIPMAP_NEAREST;
					}
					break;
				}
			}
			return GL_NEAREST;
		}

	protected:
		OpenGLTexture(const TextureDescription& desc);

	private:
		void CreateTexture(const TextureDescription& desc);
		void ReleaseTexture();

	private:
		uint32 m_ID;
		uint32 m_FramebufferID;
		mutable int32 m_BoundSlot;

		friend class RHITexture;
		friend class OpenGLRenderer;
		FRIEND_MAKE_REF;
	};
}
