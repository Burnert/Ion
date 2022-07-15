#pragma once

#include "RHI/Shader.h"
#include "DX11.h"

class IonExample;

namespace Ion
{
	struct DXShader
	{
		void* ShaderPtr;
		ID3DBlob* ShaderBlob;
		String Source;
		EShaderType Type;
	};

	class ION_API DX11Shader : public RHIShader
	{
	public:
		DX11Shader();
		virtual ~DX11Shader() override;

		virtual void AddShaderSource(EShaderType type, const String& source) override;

		virtual bool Compile() override;
		virtual bool IsCompiled() override;

		virtual bool HasUniform(const String& name) const override;

		virtual void SetUniform1f(const String& name, float value) const override;
		virtual void SetUniform2f(const String& name, const Vector2& value) const override;
		virtual void SetUniform3f(const String& name, const Vector3& value) const override;
		virtual void SetUniform4f(const String& name, const Vector4& value) const override;
		virtual void SetUniform1i(const String& name, int32 value) const override;
		virtual void SetUniform2i(const String& name, const IVector2& value) const override;
		virtual void SetUniform3i(const String& name, const IVector3& value) const override;
		virtual void SetUniform4i(const String& name, const IVector4& value) const override;
		virtual void SetUniform1ui(const String& name, uint32 value) const override;
		virtual void SetUniform2ui(const String& name, const UVector2& value) const override;
		virtual void SetUniform3ui(const String& name, const UVector3& value) const override;
		virtual void SetUniform4ui(const String& name, const UVector4& value) const override;
		virtual void SetUniformMatrix2f(const String& name, const Matrix2& value) const override;
		virtual void SetUniformMatrix2x3f(const String& name, const Matrix2x3& value) const override;
		virtual void SetUniformMatrix2x4f(const String& name, const Matrix2x4& value) const override;
		virtual void SetUniformMatrix3f(const String& name, const Matrix3& value) const override;
		virtual void SetUniformMatrix3x2f(const String& name, const Matrix3x2& value) const override;
		virtual void SetUniformMatrix3x4f(const String& name, const Matrix3x4& value) const override;
		virtual void SetUniformMatrix4f(const String& name, const Matrix4& value) const override;
		virtual void SetUniformMatrix4x2f(const String& name, const Matrix4x2& value) const override;
		virtual void SetUniformMatrix4x3f(const String& name, const Matrix4x3& value) const override;

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
		virtual void Bind() const override;
		virtual void Unbind() const override;

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
