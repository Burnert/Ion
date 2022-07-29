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
		Linear  = 1,

		Default = 0,
	};

	template<>
	struct TEnumParser<ETextureFilteringMethod>
	{
		ENUM_PARSER_TO_STRING_BEGIN(ETextureFilteringMethod)
		ENUM_PARSER_TO_STRING_HELPER(Nearest)
		ENUM_PARSER_TO_STRING_HELPER(Linear)
		ENUM_PARSER_TO_STRING_END()

		ENUM_PARSER_FROM_STRING_BEGIN(ETextureFilteringMethod)
		ENUM_PARSER_FROM_STRING_HELPER(Nearest)
		ENUM_PARSER_FROM_STRING_HELPER(Linear)
		ENUM_PARSER_FROM_STRING_END()
	};

	enum class ETextureWrapMode : uint8
	{
		Wrap   = 0,
		Clamp  = 1,
		Mirror = 2
	};

	enum class ETextureFormat : uint8
	{
		RGBA8 = 0,    // Default - Four 8-bit channels - UNorm
		RGBA10,       // Three 10-bit channels and 2-bit Alpha - UNorm For HDR display
		RGBAFloat32,  // Four 32-bit float channels - For HDR rendering
		UInt32,       // Single channel - UInt
		Float32,      // Single channel - Float
		D24S8,        // Depth Stencil
		UInt128GUID,  // GUID - For editor
	};

	/** For MSAA multisampling */
	enum class ETextureMSMode : uint8
	{
		Default = 0, // x1 - no multisampling
		X1 = 1,      // Default
		X2 = 2,
		X4 = 4,
		X8 = 8,
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

		inline operator UVector2() const
		{
			return UVector2(Width, Height);
		}

		inline operator IVector2() const
		{
			return IVector2(Width, Height);
		}
	};

	struct TextureDescription
	{
		String DebugName;
		void* InitialData;
		TextureDimensions Dimensions;
		float LODBias;
		union
		{
			uint32 Flags;
			struct
			{
				uint32 bGenerateMips : 1;
				uint32 bUseAsRenderTarget : 1;
				uint32 bUseAsDepthStencil : 1;
				uint32 bCreateSampler : 1;
				uint32 bAllowCPUReadAccess : 1;
				uint32 bAllowCPUWriteAccess : 1;
			};
		};
		ETextureFormat Format;
		ETextureUsage Usage;
		ETextureMSMode MultiSampling;
		ETextureFilteringMethod MinFilter;
		ETextureFilteringMethod MagFilter;
		ETextureFilteringMethod MipFilter;
		ETextureWrapMode UWrapMode;
		ETextureWrapMode VWrapMode;
		ETextureWrapMode WWrapMode;

		inline void SetFilterAll(ETextureFilteringMethod filter)
		{
			MinFilter = filter;
			MagFilter = filter;
			MipFilter = filter;
		}

		inline bool IsMultiSampled() const
		{
			return !(MultiSampling == ETextureMSMode::Default || MultiSampling == ETextureMSMode::X1);
		}
	};

	class ION_API RHITexture
	{
	public:
		static RHITexture* Create(const TextureDescription& desc);
		static TShared<RHITexture> CreateShared(const TextureDescription& desc);

		virtual ~RHITexture();

		virtual void SetDimensions(TextureDimensions dimensions) = 0;
		virtual void UpdateSubresource(Image* image) = 0;

		virtual void Bind(uint32 slot = 0) const = 0;
		virtual void Unbind() const = 0;

		virtual void CopyTo(const TShared<RHITexture>& destination) const = 0;
		virtual void Map(void*& outBuffer, int32& outLineSize, ETextureMapType mapType) = 0;
		virtual void Unmap() = 0;

		virtual void* GetNativeID() const = 0;

		FORCEINLINE TextureDimensions GetDimensions() const;
		FORCEINLINE const TextureDescription& GetDescription() const;
		FORCEINLINE bool IsRenderTarget() const;
		FORCEINLINE bool IsDepthStencil() const;

	protected:
		RHITexture(const TextureDescription& desc);

	protected:
		TextureDescription m_Description;
	};

	FORCEINLINE TextureDimensions RHITexture::GetDimensions() const
	{
		return m_Description.Dimensions;
	}

	FORCEINLINE const TextureDescription& RHITexture::GetDescription() const
	{
		return m_Description;
	}

	FORCEINLINE bool RHITexture::IsRenderTarget() const
	{
		return m_Description.bUseAsRenderTarget;
	}

	FORCEINLINE bool RHITexture::IsDepthStencil() const
	{
		return m_Description.bUseAsDepthStencil;
	}
}
