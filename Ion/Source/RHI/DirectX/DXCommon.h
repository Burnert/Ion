#pragma once

#include "Core/Platform/Windows.h"

#include "RHI/Texture.h"
#include "RHI/VertexAttribute.h"
#include "RHI/Shader.h"

#include <d3dcommon.h>
#include <dxgiformat.h>

#include "DXDebug.h"

namespace Ion
{
	struct DXShaderSource
	{
		String Code;
		FilePath Path;
		bool bFromFile;

		DXShaderSource(
			const String& code,
			const FilePath& path
		) : Code(code),
			Path(path),
			bFromFile(!path.IsEmpty())
		{
		}
	};

	struct DXShader
	{
		void* ShaderPtr;
		ID3DBlob* ShaderBlob;
		DXShaderSource Source;
		EShaderType Type;

		DXShader(const DXShaderSource& source, EShaderType type) :
			ShaderPtr(nullptr),
			ShaderBlob(nullptr),
			Source(source),
			Type(type)
		{
		}
	};

	struct DXVertexAttributeFormat
	{
		EVertexAttributeType Type;
		uint8 ElementCount;
	};

	enum class EDXTextureFormatUsage
	{
		Resource,
		RTV,
		DSV,
		SRV,
		AuxSRV,
	};

	class DXCommon
	{
	public:
		static constexpr const char* D3DFeatureLevelToString(D3D_FEATURE_LEVEL level);

		// Shader:

		static constexpr const char* GetShaderModelString(D3D_FEATURE_LEVEL level);
		static constexpr const char* ShaderTypeToTarget(EShaderType type);
		static String FormatShaderTarget(D3D_FEATURE_LEVEL featureLevel, EShaderType type);
		static constexpr const char* ShaderTypeToEntryPoint(EShaderType type);

		// Texture:

		static constexpr DXGI_FORMAT TextureFormatToDXGIFormat(ETextureFormat format, EDXTextureFormatUsage usage);
		/* Returns bytes per pixel for a format. */
		static constexpr int32 GetTextureFormatPixelSize(ETextureFormat format);

		// Vertex Buffer:

		static constexpr const char* GetSemanticName(const EVertexAttributeSemantic semantic);
		static constexpr DXGI_FORMAT VertexAttributeToDXGIFormat(const DXVertexAttributeFormat attribute);
	};

	inline constexpr const char* DXCommon::D3DFeatureLevelToString(D3D_FEATURE_LEVEL level)
	{
		switch (level)
		{
		case D3D_FEATURE_LEVEL_1_0_CORE: return "1.0 Core";
		case D3D_FEATURE_LEVEL_9_1:      return "9.1";
		case D3D_FEATURE_LEVEL_9_2:      return "9.2";
		case D3D_FEATURE_LEVEL_9_3:      return "9.3";
		case D3D_FEATURE_LEVEL_10_0:     return "10.0";
		case D3D_FEATURE_LEVEL_10_1:     return "10.1";
		case D3D_FEATURE_LEVEL_11_0:     return "11.0";
		case D3D_FEATURE_LEVEL_11_1:     return "11.1";
		case D3D_FEATURE_LEVEL_12_0:     return "12.0";
		case D3D_FEATURE_LEVEL_12_1:     return "12.1";
		default:                         return "Unknown Version";
		}
	}

	inline constexpr const char* DXCommon::GetShaderModelString(D3D_FEATURE_LEVEL level)
	{
		switch (level)
		{
		case D3D_FEATURE_LEVEL_1_0_CORE: return "1.0 Core";
		case D3D_FEATURE_LEVEL_9_1:      return "3.0";
		case D3D_FEATURE_LEVEL_9_2:      return "3.0";
		case D3D_FEATURE_LEVEL_9_3:      return "3.0";
		case D3D_FEATURE_LEVEL_10_0:     return "4.0";
		case D3D_FEATURE_LEVEL_10_1:     return "4.1";
		case D3D_FEATURE_LEVEL_11_0:     return "5.0";
		case D3D_FEATURE_LEVEL_11_1:     return "5.0";
		case D3D_FEATURE_LEVEL_12_0:     return "5.1";
		case D3D_FEATURE_LEVEL_12_1:     return "5.1";
		default:                         return "Unknown Version";
		}
	}

	inline constexpr const char* DXCommon::ShaderTypeToTarget(EShaderType type)
	{
		switch (type)
		{
		case EShaderType::Vertex:  return "vs_{}_{}";
		case EShaderType::Pixel:   return "ps_{}_{}";
		default:                   return "";
		}
	}

	inline String DXCommon::FormatShaderTarget(D3D_FEATURE_LEVEL featureLevel, EShaderType type)
	{
		int32 modelMajor = 0;
		int32 modelMinor = 0;

		switch (featureLevel)
		{
		case D3D_FEATURE_LEVEL_11_1: modelMajor = 5; modelMinor = 0; break;
		case D3D_FEATURE_LEVEL_11_0: modelMajor = 5; modelMinor = 0; break;
		case D3D_FEATURE_LEVEL_10_1: modelMajor = 4; modelMinor = 1; break;
		case D3D_FEATURE_LEVEL_10_0: modelMajor = 4; modelMinor = 0; break;
		}

		return fmt::format(ShaderTypeToTarget(type), modelMajor, modelMinor);
	}

	inline constexpr const char* DXCommon::ShaderTypeToEntryPoint(EShaderType type)
	{
		switch (type)
		{
		case EShaderType::Vertex:  return "VSMain";
		case EShaderType::Pixel:   return "PSMain";
		default:                   return "main";
		}
	}

	inline constexpr DXGI_FORMAT DXCommon::TextureFormatToDXGIFormat(ETextureFormat format, EDXTextureFormatUsage usage)
	{
		switch (format)
		{
		case ETextureFormat::RGBA8:       return DXGI_FORMAT_R8G8B8A8_UNORM;
		case ETextureFormat::RGBA10:      return DXGI_FORMAT_R10G10B10A2_UNORM;
		case ETextureFormat::RGBAFloat32: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ETextureFormat::Float32:     return DXGI_FORMAT_R32_FLOAT;
		case ETextureFormat::UInt32:      return DXGI_FORMAT_R32_UINT;
		case ETextureFormat::D24S8:
		{
			switch (usage)
			{
			case EDXTextureFormatUsage::Resource: return DXGI_FORMAT_R24G8_TYPELESS;
			case EDXTextureFormatUsage::RTV:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			case EDXTextureFormatUsage::DSV:      return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case EDXTextureFormatUsage::SRV:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			case EDXTextureFormatUsage::AuxSRV:   return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
			}
			break;
		}
		case ETextureFormat::UInt128GUID: return DXGI_FORMAT_R32G32B32A32_UINT;
		}
		ionbreak("Invalid format.");
		return DXGI_FORMAT_UNKNOWN;
	}

	inline constexpr int32 DXCommon::GetTextureFormatPixelSize(ETextureFormat format)
	{
		switch (format)
		{
		case ETextureFormat::RGBA8:       return 4;
		case ETextureFormat::RGBAFloat32: return 16;
		case ETextureFormat::UInt32:      return 4;
		case ETextureFormat::UInt128GUID: return 16;
		}
		ionassert(false, "Invalid format.");
		return 0;
	}

	inline constexpr const char* DXCommon::GetSemanticName(const EVertexAttributeSemantic semantic)
	{
		switch (semantic)
		{
		case EVertexAttributeSemantic::Position:  return "POSITION";
		case EVertexAttributeSemantic::Normal:    return "NORMAL";
		case EVertexAttributeSemantic::TexCoord:  return "TEXCOORD";
		default:                                  return "";
		}
	}

	inline constexpr DXGI_FORMAT DXCommon::VertexAttributeToDXGIFormat(const DXVertexAttributeFormat attribute)
	{
		switch (attribute.ElementCount)
		{
		case 1:
		{
			switch (attribute.Type)
			{
			case EVertexAttributeType::Null:           return DXGI_FORMAT_UNKNOWN;
			case EVertexAttributeType::Byte:           return DXGI_FORMAT_R8_SINT;
			case EVertexAttributeType::UnsignedByte:   return DXGI_FORMAT_R8_UINT;
			case EVertexAttributeType::Short:          return DXGI_FORMAT_R16_SINT;
			case EVertexAttributeType::UnsignedShort:  return DXGI_FORMAT_R16_UINT;
			case EVertexAttributeType::Int:            return DXGI_FORMAT_R32_SINT;
			case EVertexAttributeType::UnsignedInt:    return DXGI_FORMAT_R32_UINT;
			case EVertexAttributeType::Float16:        return DXGI_FORMAT_R16_FLOAT;
			case EVertexAttributeType::Float: 		   return DXGI_FORMAT_R32_FLOAT;
			case EVertexAttributeType::Double:         return DXGI_FORMAT_UNKNOWN;
			}
		}
		case 2:
		{
			switch (attribute.Type)
			{
			case EVertexAttributeType::Null:           return DXGI_FORMAT_UNKNOWN;
			case EVertexAttributeType::Byte:           return DXGI_FORMAT_R8G8_SINT;
			case EVertexAttributeType::UnsignedByte:   return DXGI_FORMAT_R8G8_UINT;
			case EVertexAttributeType::Short:          return DXGI_FORMAT_R16G16_SINT;
			case EVertexAttributeType::UnsignedShort:  return DXGI_FORMAT_R16G16_UINT;
			case EVertexAttributeType::Int:            return DXGI_FORMAT_R32G32_SINT;
			case EVertexAttributeType::UnsignedInt:    return DXGI_FORMAT_R32G32_UINT;
			case EVertexAttributeType::Float16:        return DXGI_FORMAT_R16G16_FLOAT;
			case EVertexAttributeType::Float:          return DXGI_FORMAT_R32G32_FLOAT;
			case EVertexAttributeType::Double:         return DXGI_FORMAT_UNKNOWN;
			}
		}
		case 3:
		{
			switch (attribute.Type)
			{
			case EVertexAttributeType::Null:           return DXGI_FORMAT_UNKNOWN;
			case EVertexAttributeType::Byte:           return DXGI_FORMAT_UNKNOWN;
			case EVertexAttributeType::UnsignedByte:   return DXGI_FORMAT_UNKNOWN;
			case EVertexAttributeType::Short:          return DXGI_FORMAT_UNKNOWN;
			case EVertexAttributeType::UnsignedShort:  return DXGI_FORMAT_UNKNOWN;
			case EVertexAttributeType::Int:            return DXGI_FORMAT_R32G32B32_SINT;
			case EVertexAttributeType::UnsignedInt:    return DXGI_FORMAT_R32G32B32_UINT;
			case EVertexAttributeType::Float16:        return DXGI_FORMAT_UNKNOWN;
			case EVertexAttributeType::Float:          return DXGI_FORMAT_R32G32B32_FLOAT;
			case EVertexAttributeType::Double:         return DXGI_FORMAT_UNKNOWN;
			}
		}
		case 4:
		{
			switch (attribute.Type)
			{
			case EVertexAttributeType::Null:           return DXGI_FORMAT_UNKNOWN;
			case EVertexAttributeType::Byte:           return DXGI_FORMAT_R8G8B8A8_SINT;
			case EVertexAttributeType::UnsignedByte:   return DXGI_FORMAT_R8G8B8A8_UINT;
			case EVertexAttributeType::Short:          return DXGI_FORMAT_R16G16B16A16_SINT;
			case EVertexAttributeType::UnsignedShort:  return DXGI_FORMAT_R16G16B16A16_UINT;
			case EVertexAttributeType::Int:            return DXGI_FORMAT_R32G32B32A32_SINT;
			case EVertexAttributeType::UnsignedInt:	   return DXGI_FORMAT_R32G32B32A32_UINT;
			case EVertexAttributeType::Float16:        return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case EVertexAttributeType::Float:          return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case EVertexAttributeType::Double:         return DXGI_FORMAT_UNKNOWN;
			}
		}
		default: return DXGI_FORMAT_UNKNOWN;
		}
	}
}
