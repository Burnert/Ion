#include "IonPCH.h"

#include "DX10Shader.h"
#include "RHI/DirectX/DXInclude.h"
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
