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

	Result<void, ShaderCompilationError> DX11Shader::Compile()
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

			HRESULT hResult;
			ID3D11Device* device = DX11::GetDevice();

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

				DX11Logger.Error("{0} compilation failed!", ShaderTypeToString(shader.Type));
				DX11Logger.Error(errorMessage);

				ionthrow(ShaderCompilationError, "{0} compilation failed!\n{1}", ShaderTypeToString(shader.Type), errorMessage);
			}

			void* blobPtr = shader.ShaderBlob->GetBufferPointer();
			size_t blobSize = shader.ShaderBlob->GetBufferSize();

			if (shader.Type == EShaderType::Vertex)
			{
				dxcall_t(device->CreateVertexShader(blobPtr, blobSize, nullptr, (ID3D11VertexShader**)&shader.ShaderPtr),
					ionthrow(ShaderCompilationError, "Could not create Vertex Shader"),
					"Could not create Vertex Shader");
			}
			else if (shader.Type == EShaderType::Pixel)
			{
				dxcall_t(device->CreatePixelShader(blobPtr, blobSize, nullptr, (ID3D11PixelShader**)&shader.ShaderPtr),
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
