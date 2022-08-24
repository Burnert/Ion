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
	struct DXShader
	{
		void* ShaderPtr;
		ID3DBlob* ShaderBlob;
		String Source;
		EShaderType Type;
	};

	class DXCommon
	{
	public:
		static constexpr const char* D3DFeatureLevelToString(D3D_FEATURE_LEVEL level)
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

		static constexpr const char* GetShaderModelString(D3D_FEATURE_LEVEL level)
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

		template<typename TEnumUsage>
		inline static constexpr DXGI_FORMAT TextureFormatToDXGIFormat(ETextureFormat format, TEnumUsage usage)
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
					case TEnumUsage::Resource: return DXGI_FORMAT_R24G8_TYPELESS;
					case TEnumUsage::RTV:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					case TEnumUsage::DSV:      return DXGI_FORMAT_D24_UNORM_S8_UINT;
					case TEnumUsage::SRV:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					case TEnumUsage::AuxSRV:   return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
					}
					break;
				}
				case ETextureFormat::UInt128GUID: return DXGI_FORMAT_R32G32B32A32_UINT;
			}
			ionbreak("Invalid format.");
			return DXGI_FORMAT_UNKNOWN;
		}

		static constexpr const char* GetSemanticName(const EVertexAttributeSemantic semantic)
		{
			switch (semantic)
			{
			case EVertexAttributeSemantic::Position:  return "POSITION";
			case EVertexAttributeSemantic::Normal:    return "NORMAL";
			case EVertexAttributeSemantic::TexCoord:  return "TEXCOORD";
			default:                                  return "";
			}
		}

		struct VertexAttributeFormat
		{
			EVertexAttributeType Type;
			uint8 ElementCount;
		};

		static constexpr DXGI_FORMAT VertexAttributeToDXGIFormat(const VertexAttributeFormat attribute)
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
	};
}
