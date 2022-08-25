#include "IonPCH.h"

#include "DXInclude.h"
#include "Application/EnginePath.h"

namespace Ion
{
	DXInclude::DXInclude()
	{
	}

	DXInclude::DXInclude(const FilePath& sourceDir) :
		m_SourceDir(sourceDir)
	{
		ionassert(sourceDir.IsDirectory());
	}

	HRESULT DXInclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		// @TODO: Finish the local includes

		FilePath fullIncludePath;

		switch (IncludeType)
		{
			case D3D_INCLUDE_LOCAL:
			{
				if (!m_SourceDir.IsEmpty())
				{
					FilePath includeRelativeToSource = m_SourceDir / pFileName;
					if (includeRelativeToSource.IsFile())
					{
						fullIncludePath = includeRelativeToSource;
						break;
					}
				}

				// @TODO: Depends on the current project include paths
				fullIncludePath = EnginePath::GetShadersPath() / pFileName;
			}
			break;
			case D3D_INCLUDE_SYSTEM:
			{
				// Engine shaders library
				fullIncludePath = EnginePath::GetShadersPath() / pFileName;
			}
			break;
			default: return E_FAIL;
		}

		File includeFile(fullIncludePath);
		includeFile.Open();

		int64 size = includeFile.GetSize();
		ionassert(size <= std::numeric_limits<UINT>::max(), "The include file is too big.");
		char* buffer = (char*)malloc(size);
		ionverify(buffer);

		if (!includeFile.Read(buffer, size))
		{
			return E_FAIL;
		}

		*ppData = buffer;
		*pBytes = (uint32)size;

		return S_OK;
	}

	HRESULT DXInclude::Close(LPCVOID pData)
	{
		if (pData)
		{
			free((void*)pData);
		}

		return S_OK;
	}
}
