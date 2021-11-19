#include "IonPCH.h"

#include "DX11Shader.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace Ion
{
	DX11Shader::DX11Shader() :
		m_bCompiled(false)
	{ }

	DX11Shader::~DX11Shader()
	{
		if (!m_bCompiled)
			return;

		IterateShaders([](DXShader& shader)
		{
			if (shader.ShaderPtr)
				((IUnknown*)shader.ShaderPtr)->Release();

			if (shader.ShaderBlob)
				shader.ShaderBlob->Release();
		});
	}

	void DX11Shader::AddShaderSource(EShaderType type, const String& source)
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

	bool DX11Shader::Compile()
	{
		TRACE_FUNCTION();

		ionassert(!m_bCompiled, "Shader has already been compiled.");

		for (auto& entry : m_Shaders)
		{
			DXShader& shader = entry.second;

			HRESULT hResult;
			ID3D11Device* device = DX11::GetDevice();

			uint32 compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if ION_DEBUG
			compileFlags |= D3DCOMPILE_DEBUG;
#endif
			ID3DBlob* errorMessagesBlob = nullptr;

			hResult = D3DCompile(
				shader.Source.c_str(),
				shader.Source.length(),
				nullptr, nullptr, nullptr,
				"main",
				ShaderTypeToTarget(shader.Type),
				compileFlags, 0,
				&shader.ShaderBlob,
				&errorMessagesBlob);
			if (FAILED(hResult) || errorMessagesBlob)
			{
				const char* errorMessage = (char*)errorMessagesBlob->GetBufferPointer();

				LOG_ERROR("{0} compilation failed!", ShaderTypeToString(shader.Type));
				LOG_ERROR(errorMessage);

				return false;
			}

			void* blobPtr = shader.ShaderBlob->GetBufferPointer();
			size_t blobSize = shader.ShaderBlob->GetBufferSize();

			if (shader.Type == EShaderType::Vertex)
			{
				dxcall_f(device->CreateVertexShader(blobPtr, blobSize, nullptr, (ID3D11VertexShader**)&shader.ShaderPtr),
					"Could not create Vertex Shader");
			}
			else if (shader.Type == EShaderType::Pixel)
			{
				dxcall_f(device->CreatePixelShader(blobPtr, blobSize, nullptr, (ID3D11PixelShader**)&shader.ShaderPtr),
					"Could not create Pixel Shader");
			}
			else
			{
				// Unknown shader type
				debugbreak();
				shader.ShaderBlob->Release();
				shader.ShaderBlob = nullptr;
				return false;
			}
		}

		return true;
	}

	// @TODO: Add all that uniform related shit

	bool DX11Shader::HasUniform(const String& name) const
	{
		return false;
	}
	void DX11Shader::SetUniform1f(const String& name, float value) const
	{
	}
	void DX11Shader::SetUniform2f(const String& name, const Vector2& value) const
	{
	}
	void DX11Shader::SetUniform3f(const String& name, const Vector3& value) const
	{
	}
	void DX11Shader::SetUniform4f(const String& name, const Vector4& value) const
	{
	}
	void DX11Shader::SetUniform1i(const String& name, int32 value) const
	{
	}
	void DX11Shader::SetUniform2i(const String& name, const IVector2& value) const
	{
	}
	void DX11Shader::SetUniform3i(const String& name, const IVector3& value) const
	{
	}
	void DX11Shader::SetUniform4i(const String& name, const IVector4& value) const
	{
	}
	void DX11Shader::SetUniform1ui(const String& name, uint32 value) const
	{
	}
	void DX11Shader::SetUniform2ui(const String& name, const UVector2& value) const
	{
	}
	void DX11Shader::SetUniform3ui(const String& name, const UVector3& value) const
	{
	}
	void DX11Shader::SetUniform4ui(const String& name, const UVector4& value) const
	{
	}
	void DX11Shader::SetUniformMatrix2f(const String& name, const Matrix2& value) const
	{
	}
	void DX11Shader::SetUniformMatrix2x3f(const String& name, const Matrix2x3& value) const
	{
	}
	void DX11Shader::SetUniformMatrix2x4f(const String& name, const Matrix2x4& value) const
	{
	}
	void DX11Shader::SetUniformMatrix3f(const String& name, const Matrix3& value) const
	{
	}
	void DX11Shader::SetUniformMatrix3x2f(const String& name, const Matrix3x2& value) const
	{
	}
	void DX11Shader::SetUniformMatrix3x4f(const String& name, const Matrix3x4& value) const
	{
	}
	void DX11Shader::SetUniformMatrix4f(const String& name, const Matrix4& value) const
	{
	}
	void DX11Shader::SetUniformMatrix4x2f(const String& name, const Matrix4x2& value) const
	{
	}
	void DX11Shader::SetUniformMatrix4x3f(const String& name, const Matrix4x3& value) const
	{
	}

	void DX11Shader::Bind() const
	{
		IterateShaders([](const DXShader& shader)
		{
			ID3D11DeviceContext* context = DX11::GetContext();

			switch (shader.Type)
			{
				case EShaderType::Vertex:
				{
					context->VSSetShader((ID3D11VertexShader*)shader.ShaderPtr, nullptr, 0);
					break;
				}
				case EShaderType::Pixel:
				{
					context->PSSetShader((ID3D11PixelShader*)shader.ShaderPtr, nullptr, 0);
					break;
				}
			}
		});
	}

	void DX11Shader::Unbind() const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		context->VSSetShader(nullptr, nullptr, 0);
		context->PSSetShader(nullptr, nullptr, 0);
	}
}
