#pragma once

namespace Ion
{
	enum class ETextureUsage : uint8
	{
		Default   = 0,
		Immutable = 1,
		Dynamic   = 2,
		Staging   = 3
	};

	enum class ETextureFilteringMethod : uint8
	{
		Nearest = 0,
		Linear  = 1
	};

	enum class ETextureWrapMode : uint8
	{
		Wrap   = 0,
		Clamp  = 1,
		Mirror = 2
	};

	enum class ETextureFormat : uint8
	{
		RGBA8 = 0, // Default
		RGBAFloat32,
		UInt32,
		UInt128GUID,
	};

	enum class ETextureMapType : uint8
	{
		Read,
		Write,
		ReadWrite,
	};

	struct TextureDimensions
	{
		uint32 Width;
		uint32 Height;
	};

	struct TextureDescription
	{
		String DebugName;
		TextureDimensions Dimensions;
		float LODBias;
		union
		{
			uint32 Flags;
			struct
			{
				uint32 bGenerateMips : 1;
				uint32 bUseAsRenderTarget : 1;
				uint32 bCreateColorAttachment : 1; // @TODO: Implement GL version
				uint32 bCreateDepthStencilAttachment : 1;
				uint32 bCreateDepthSampler : 1; // @TODO: Implement GL version
				uint32 bAllowCPUReadAccess : 1;
				uint32 bAllowCPUWriteAccess : 1;
			};
		};
		ETextureFormat Format;
		ETextureUsage Usage;
		ETextureFilteringMethod MinFilter;
		ETextureFilteringMethod MagFilter;
		ETextureFilteringMethod MipFilter;
		ETextureWrapMode UWrapMode;
		ETextureWrapMode VWrapMode;
		ETextureWrapMode WWrapMode;
	};

	class ION_API Texture
	{
	public:
		static TShared<Texture> Create(const TextureDescription& desc);

		virtual ~Texture();

		virtual void SetDimensions(TextureDimensions dimensions) = 0;
		virtual void UpdateSubresource(Image* image) = 0;

		virtual void Bind(uint32 slot = 0) const = 0;
		virtual void BindDepth(uint32 slot = 0) const = 0;
		virtual void Unbind() const = 0;

		virtual void CopyTo(const TShared<Texture>& destination) const = 0;
		virtual void Map(void*& outBuffer, int32& outLineSize, ETextureMapType mapType) = 0;
		virtual void Unmap() = 0;

		virtual void* GetNativeID() const = 0;

		FORCEINLINE TextureDimensions GetDimensions() const
		{
			return m_Description.Dimensions;
		}

		FORCEINLINE const TextureDescription& GetDescription() const
		{
			return m_Description;
		}

		FORCEINLINE bool IsRenderTarget() const
		{
			return m_Description.bUseAsRenderTarget;
		}

		FORCEINLINE bool HasColorAttachment() const
		{
			return m_Description.bCreateColorAttachment;
		}

		FORCEINLINE bool HasDepthStencilAttachment() const
		{
			return m_Description.bCreateDepthStencilAttachment;
		}

	protected:
		Texture(const TextureDescription& desc);

	protected:
		TextureDescription m_Description;
	};
}
