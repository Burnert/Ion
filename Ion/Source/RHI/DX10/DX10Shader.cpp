#include "IonPCH.h"

#include "RHI/RHICore.h"

#if RHI_BUILD_DX10

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
		DX10Logger.Info("DX10Shader has been created.");
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

		DX10Logger.Info("DX10Shader has been destroyed.");
	}

	void DX10Shader::AddShaderSource(EShaderType type, const String& source)
	{
		TRACE_FUNCTION();

		if (!m_bCompiled && m_Shaders.find(type) == m_Shaders.end())
		{
			m_Shaders.emplace(type, DXShader(DXShaderSource(source, FilePath()), type));
		}
	}

	void DX10Shader::AddShaderSource(EShaderType type, const String& source, const FilePath& sourcePath)
	{
		TRACE_FUNCTION();

		if (!m_bCompiled && m_Shaders.find(type) == m_Shaders.end())
		{
			m_Shaders.emplace(type, DXShader(DXShaderSource(source, sourcePath), type));
		}
	}

	Result<void, RHIError, ShaderCompilationError> DX10Shader::Compile()
	{
		TRACE_FUNCTION();

		ionassert(!m_bCompiled, "Shader has already been compiled.");

		for (auto& entry : m_Shaders)
		{
			DXShader& shader = entry.second;

			if (shader.Source.Code.empty())
			{
				shader.ShaderPtr = nullptr;
				continue;
			}

			ID3D10Device* device = DX10::GetDevice();

			uint32 compileFlags =
#if SHADER_DEBUG_ENABLED
				D3DCOMPILE_SKIP_OPTIMIZATION |
				D3DCOMPILE_DEBUG;
#else
				D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
			ID3DBlob* errorMessagesBlob = nullptr;

			String sourceName =
				shader.Source.bFromFile ?
				shader.Source.Path.ToString() :
				"[TODO: SHADER FROM MEMORY NAME]";

			DXInclude includeHandler =
				shader.Source.bFromFile ?
				DXInclude(shader.Source.Path / "..") :
				DXInclude();

			DX10Logger.Trace("DX10Shader compiling {}...", ShaderTypeToString(shader.Type));

			dxcall_throw(
				D3DCompile(
					shader.Source.Code.c_str(),
					shader.Source.Code.length(),
					sourceName.c_str(),
					nullptr,
					&includeHandler,
					DXCommon::ShaderTypeToEntryPoint(shader.Type),
					DXCommon::FormatShaderTarget((D3D_FEATURE_LEVEL)DX10::GetFeatureLevel(), shader.Type).c_str(),
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

			DX10Logger.Trace("DX10Shader {} has been compiled.", ShaderTypeToString(shader.Type));

			if (errorMessagesBlob)
			{
				DX10Logger.Warn("{} compilation generated some warnings.\n{}", ShaderTypeToString(shader.Type), (char*)errorMessagesBlob->GetBufferPointer());
				errorMessagesBlob->Release();
			}

			void* blobPtr = shader.ShaderBlob->GetBufferPointer();
			size_t blobSize = shader.ShaderBlob->GetBufferSize();

			if (shader.Type == EShaderType::Vertex)
			{
				dxcall(device->CreateVertexShader(blobPtr, blobSize, (ID3D10VertexShader**)&shader.ShaderPtr),
					"Could not create Vertex Shader");

				DX10Logger.Debug("DX10Shader Vertex Shader object has been created.");
			}
			else if (shader.Type == EShaderType::Pixel)
			{
				dxcall(device->CreatePixelShader(blobPtr, blobSize, (ID3D10PixelShader**)&shader.ShaderPtr),
					"Could not create Pixel Shader");

				DX10Logger.Debug("DX10Shader Pixel Shader object has been created.");
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
					dxcall_nocheck(device->VSSetShader((ID3D10VertexShader*)shader.ShaderPtr));
					break;
				}
				case EShaderType::Pixel:
				{
					dxcall_nocheck(device->PSSetShader((ID3D10PixelShader*)shader.ShaderPtr));
					break;
				}
			}
		});
	}

	void DX10Shader::Unbind() const
	{
		ID3D10Device* device = DX10::GetDevice();

		dxcall_nocheck(device->VSSetShader(nullptr));
		dxcall_nocheck(device->PSSetShader(nullptr));
	}
}

#endif // RHI_BUILD_DX10
