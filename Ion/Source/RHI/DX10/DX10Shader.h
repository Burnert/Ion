#pragma once

#include "RHI/Shader.h"
#include "DX10.h"

namespace Ion
{
	class ION_API DX10Shader : public RHIShader
	{
	public:
		DX10Shader();
		virtual ~DX10Shader() override;

		virtual void AddShaderSource(EShaderType type, const String& source) override;
		virtual void AddShaderSource(EShaderType type, const String& source, const FilePath& sourcePath) override;

		virtual Result<void, RHIError, ShaderCompilationError> Compile() override;
		virtual bool IsCompiled() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

	protected:
		template<typename T>
		void IterateShaders(T callback);
		template<typename T>
		void IterateShaders(T callback) const;
		ID3DBlob* GetVertexShaderByteCode() const;

	private:
		THashMap<EShaderType, DXShader> m_Shaders;
		bool m_bCompiled;

		friend class DX10Renderer;
		friend class DX10VertexBuffer;
	};

	template<typename T>
	inline void DX10Shader::IterateShaders(T callback)
	{
		for (auto& entry : m_Shaders)
		{
			DXShader& shader = entry.second;
			callback(shader);
		}
	}

	template<typename T>
	inline void DX10Shader::IterateShaders(T callback) const
	{
		for (const auto& entry : m_Shaders)
		{
			const DXShader& shader = entry.second;
			callback(shader);
		}
	}

	inline ID3DBlob* DX10Shader::GetVertexShaderByteCode() const
	{
		auto it = m_Shaders.find(EShaderType::Vertex);
		if (it == m_Shaders.end())
			return nullptr;

		const DXShader& shader = (*it).second;
		return shader.ShaderBlob;
	}
}
