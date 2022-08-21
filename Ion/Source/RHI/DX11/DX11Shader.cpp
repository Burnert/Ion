#include "IonPCH.h"

#include "DX11Shader.h"
#include "RHI/DirectX/DXInclude.h"
#include "Renderer/RendererCore.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace Ion
{
	DX11Shader::DX11Shader() :
		m_bCompiled(false)
	{
		DX11Logger.Trace("Created DX11Shader object.");
	}

	DX11Shader::~DX11Shader()
	{
		if (!m_bCompiled)
			return;

		IterateShaders([](DXShader& shader)
		{
			COMRelease(shader.ShaderPtr);
			COMRelease(shader.ShaderBlob);
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

	Result<void, RHIError, ShaderCompilationError> DX11Shader::Compile()
	{
		TRACE_FUNCTION();

		ionassert(!m_bCompiled, "Shader has already been compiled.");

		DXInclude includeHandler;

		for (auto& entry : m_Shaders)
		{
			DXShader& shader = entry.second;

			if (shader.Source.empty())
			{
				shader.ShaderPtr = nullptr;
				continue;
			}

			ID3D11Device* device = DX11::GetDevice();

			uint32 compileFlags =
#if SHADER_DEBUG_ENABLED
				D3DCOMPILE_SKIP_OPTIMIZATION |
				D3DCOMPILE_DEBUG;
#else
				D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
			ID3DBlob* errorMessagesBlob = nullptr;

			dxcall_throw(
				D3DCompile(
					shader.Source.c_str(),
					shader.Source.length(),
					nullptr, nullptr,
					&includeHandler,
					ShaderTypeToEntryPoint(shader.Type),
					FormatShaderTarget(shader.Type),
					compileFlags, 0,
					&shader.ShaderBlob,
					&errorMessagesBlob
				),
				ShaderCompilationError,
				"{} compilation failed.\n{}", ShaderTypeToString(shader.Type),
				[&] {
					String errorMessage = (char*)errorMessagesBlob->GetBufferPointer();
					errorMessagesBlob->Release();
					return errorMessage;
				}());

			if (errorMessagesBlob)
			{
				DX11Logger.Warn("{} compilation generated some warnings.\n{}", ShaderTypeToString(shader.Type), (char*)errorMessagesBlob->GetBufferPointer());
				errorMessagesBlob->Release();
			}

			void* blobPtr = shader.ShaderBlob->GetBufferPointer();
			size_t blobSize = shader.ShaderBlob->GetBufferSize();

			if (shader.Type == EShaderType::Vertex)
			{
				dxcall(device->CreateVertexShader(blobPtr, blobSize, nullptr, (ID3D11VertexShader**)&shader.ShaderPtr),
					"Could not create Vertex Shader");
			}
			else if (shader.Type == EShaderType::Pixel)
			{
				dxcall(device->CreatePixelShader(blobPtr, blobSize, nullptr, (ID3D11PixelShader**)&shader.ShaderPtr),
					"Could not create Pixel Shader");
			}
			else
			{
				ionbreak("Unknown shader type.");
				shader.ShaderBlob->Release();
				shader.ShaderBlob = nullptr;
			}
		}

		m_bCompiled = true;

		return Ok();
	}

	bool DX11Shader::IsCompiled()
	{
		return m_bCompiled;
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
					dxcall_nocheck(context->VSSetShader((ID3D11VertexShader*)shader.ShaderPtr, nullptr, 0));
					break;
				}
				case EShaderType::Pixel:
				{
					dxcall_nocheck(context->PSSetShader((ID3D11PixelShader*)shader.ShaderPtr, nullptr, 0));
					break;
				}
			}
		});
	}

	void DX11Shader::Unbind() const
	{
		ID3D11DeviceContext* context = DX11::GetContext();

		dxcall_nocheck(context->VSSetShader(nullptr, nullptr, 0));
		dxcall_nocheck(context->PSSetShader(nullptr, nullptr, 0));
	}
}
