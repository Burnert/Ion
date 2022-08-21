#pragma once

#include "RHI/Shader.h"
#include "DX11.h"

class IonExample;

namespace Ion
{
	class ION_API DX11Shader : public RHIShader
	{
	public:
		DX11Shader();
		virtual ~DX11Shader() override;

		virtual void AddShaderSource(EShaderType type, const String& source) override;

		virtual Result<void, RHIError, ShaderCompilationError> Compile() override;
		virtual bool IsCompiled() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		static constexpr const char* ShaderTypeToTarget(EShaderType type)
		{
			switch (type)
			{
			case EShaderType::Vertex:  return "vs_%i_%i";
			case EShaderType::Pixel:   return "ps_%i_%i";
			default:                   return "";
			}
		}

		static char* FormatShaderTarget(EShaderType type)
		{
			static char c_Target[10];
			memset(c_Target, 0, 10);

			int32 modelMajor = 0;
			int32 modelMinor = 0;

			switch (DX11::GetFeatureLevel())
			{
			case D3D_FEATURE_LEVEL_11_1: modelMajor = 5; modelMinor = 0; break;
			case D3D_FEATURE_LEVEL_11_0: modelMajor = 5; modelMinor = 0; break;
			case D3D_FEATURE_LEVEL_10_1: modelMajor = 4; modelMinor = 1; break;
			case D3D_FEATURE_LEVEL_10_0: modelMajor = 4; modelMinor = 0; break;
			}

			sprintf_s(c_Target, ShaderTypeToTarget(type), modelMajor, modelMinor);
			return c_Target;
		}

		static constexpr const char* ShaderTypeToEntryPoint(EShaderType type)
		{
			switch (type)
			{
			case EShaderType::Vertex:  return "VSMain";
			case EShaderType::Pixel:   return "PSMain";
			default:                   return "main";
			}
		}

	protected:
		template<typename T>
		void IterateShaders(T callback)
		{
			for (auto& entry : m_Shaders)
			{
				DXShader& shader = entry.second;
				callback(shader);
			}
		}

		template<typename T>
		void IterateShaders(T callback) const
		{
			for (const auto& entry : m_Shaders)
			{
				const DXShader& shader = entry.second;
				callback(shader);
			}
		}

		ID3DBlob* GetVertexShaderByteCode() const
		{
			auto it = m_Shaders.find(EShaderType::Vertex);
			if (it == m_Shaders.end())
				return nullptr;

			const DXShader& shader = (*it).second;
			return shader.ShaderBlob;
		}

	private:
		THashMap<EShaderType, DXShader> m_Shaders;
		bool m_bCompiled;

		friend class DX11Renderer;
		friend class DX11VertexBuffer;
		friend class ::IonExample;
	};
}
