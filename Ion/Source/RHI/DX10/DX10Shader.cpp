#include "IonPCH.h"

#include "DX10Shader.h"
#include "RHI/DX11/DX11Include.h"
#include "Renderer/RendererCore.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace Ion
{
	DX10Shader::DX10Shader() :
		m_bCompiled(false)
	{
		DX10Logger.Trace("Created DX10Shader object.");
	}

	DX10Shader::~DX10Shader()
	{
		if (!m_bCompiled)
			return;

		IterateShaders([](DXShader& shader)
		{
			COMRelease(shader.ShaderPtr);
			COMRelease(shader.ShaderBlob);
		});
	}

	void DX10Shader::AddShaderSource(EShaderType type, const String& source)
	{
		TRACE_FUNCTION();

		if (!m_bCompiled && m_Shaders.find(type) == m_Shaders.end())
		{
			DXShader shader { };
			shader.Type = type;
			shader.Source = source;
			m_Shaders[type] = Move(shader);
		}
	}

	Result<void, ShaderCompilationError> DX10Shader::Compile()
	{
		TRACE_FUNCTION();

		ionassert(!m_bCompiled, "Shader has already been compiled.");

		DX11Include includeHandler;

		for (auto& entry : m_Shaders)
		{
			DXShader& shader = entry.second;

			if (shader.Source.empty())
			{
				shader.ShaderPtr = nullptr;
				continue;
			}

			HRESULT hResult;
			ID3D10Device* device = DX10::GetDevice();

			uint32 compileFlags =
#if SHADER_DEBUG_ENABLED
				D3DCOMPILE_SKIP_OPTIMIZATION |
				D3DCOMPILE_DEBUG;
#else
				D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
			ID3DBlob* errorMessagesBlob = nullptr;

			hResult = D3DCompile(
				shader.Source.c_str(),
				shader.Source.length(),
				nullptr, nullptr,
				&includeHandler,
				ShaderTypeToEntryPoint(shader.Type),
				FormatShaderTarget(shader.Type),
				compileFlags, 0,
				&shader.ShaderBlob,
				&errorMessagesBlob);
			if (FAILED(hResult) || errorMessagesBlob)
			{
				const char* errorMessage = (char*)errorMessagesBlob->GetBufferPointer();

				DX10Logger.Error("{0} compilation failed!", ShaderTypeToString(shader.Type));
				DX10Logger.Error(errorMessage);

				ionthrow(ShaderCompilationError, "{0} compilation failed!\n{1}", ShaderTypeToString(shader.Type), errorMessage);
			}

			void* blobPtr = shader.ShaderBlob->GetBufferPointer();
			size_t blobSize = shader.ShaderBlob->GetBufferSize();

			if (shader.Type == EShaderType::Vertex)
			{
				dxcall_t(device->CreateVertexShader(blobPtr, blobSize, (ID3D10VertexShader**)&shader.ShaderPtr),
					ionthrow(ShaderCompilationError, "Could not create Vertex Shader"),
					"Could not create Vertex Shader");
			}
			else if (shader.Type == EShaderType::Pixel)
			{
				dxcall_t(device->CreatePixelShader(blobPtr, blobSize, (ID3D10PixelShader**)&shader.ShaderPtr),
					ionthrow(ShaderCompilationError, "Could not create Pixel Shader"),
					"Could not create Pixel Shader");
			}
			else
			{
				// Unknown shader type
				debugbreak();
				shader.ShaderBlob->Release();
				shader.ShaderBlob = nullptr;
				
				ionthrow(ShaderCompilationError, "{0} compilation failed!\nUnknown shader type.", ShaderTypeToString(shader.Type));
			}
		}

		m_bCompiled = true;

		return Void();
	}

	bool DX10Shader::IsCompiled()
	{
		return m_bCompiled;
	}

	bool DX10Shader::HasUniform(const String& name) const
	{
		return true;
	}
	void DX10Shader::SetUniform1f(const String& name, float value) const
	{
	}
	void DX10Shader::SetUniform2f(const String& name, const Vector2& value) const
	{
	}
	void DX10Shader::SetUniform3f(const String& name, const Vector3& value) const
	{
	}
	void DX10Shader::SetUniform4f(const String& name, const Vector4& value) const
	{
	}
	void DX10Shader::SetUniform1i(const String& name, int32 value) const
	{
	}
	void DX10Shader::SetUniform2i(const String& name, const IVector2& value) const
	{
	}
	void DX10Shader::SetUniform3i(const String& name, const IVector3& value) const
	{
	}
	void DX10Shader::SetUniform4i(const String& name, const IVector4& value) const
	{
	}
	void DX10Shader::SetUniform1ui(const String& name, uint32 value) const
	{
	}
	void DX10Shader::SetUniform2ui(const String& name, const UVector2& value) const
	{
	}
	void DX10Shader::SetUniform3ui(const String& name, const UVector3& value) const
	{
	}
	void DX10Shader::SetUniform4ui(const String& name, const UVector4& value) const
	{
	}
	void DX10Shader::SetUniformMatrix2f(const String& name, const Matrix2& value) const
	{
	}
	void DX10Shader::SetUniformMatrix2x3f(const String& name, const Matrix2x3& value) const
	{
	}
	void DX10Shader::SetUniformMatrix2x4f(const String& name, const Matrix2x4& value) const
	{
	}
	void DX10Shader::SetUniformMatrix3f(const String& name, const Matrix3& value) const
	{
	}
	void DX10Shader::SetUniformMatrix3x2f(const String& name, const Matrix3x2& value) const
	{
	}
	void DX10Shader::SetUniformMatrix3x4f(const String& name, const Matrix3x4& value) const
	{
	}
	void DX10Shader::SetUniformMatrix4f(const String& name, const Matrix4& value) const
	{
	}
	void DX10Shader::SetUniformMatrix4x2f(const String& name, const Matrix4x2& value) const
	{
	}
	void DX10Shader::SetUniformMatrix4x3f(const String& name, const Matrix4x3& value) const
	{
	}

	void DX10Shader::Bind() const
	{
		IterateShaders([](const DXShader& shader)
		{
			ID3D10Device* device = DX10::GetDevice();

			switch (shader.Type)
			{
				case EShaderType::Vertex:
				{
					device->VSSetShader((ID3D10VertexShader*)shader.ShaderPtr);
					break;
				}
				case EShaderType::Pixel:
				{
					device->PSSetShader((ID3D10PixelShader*)shader.ShaderPtr);
					break;
				}
			}
		});
	}

	void DX10Shader::Unbind() const
	{
		ID3D10Device* device = DX10::GetDevice();

		device->VSSetShader(nullptr);
		device->PSSetShader(nullptr);
	}
}
