#include "IonPCH.h"

#include "DXInclude.h"
#include "Application/EnginePath.h"

namespace Ion
{
	DXInclude::DXInclude() { }

	HRESULT DXInclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		FilePath includePath("");

		switch (IncludeType)
		{
		case D3D_INCLUDE_LOCAL:
			includePath = EnginePath::GetShadersPath();
			break;
		// case D3D_INCLUDE_SYSTEM:
			// @TODO: Add system includes
		default:
			return E_FAIL;
		}

		File includeFile(includePath + pFileName);
		includeFile.Open();

		int64 size = includeFile.GetSize();
		ionassert(size <= std::numeric_limits<UINT>::max(), "The include file is too big.");
		char* buffer = (char*)malloc(size);

		if (!buffer || !includeFile.Read(buffer, size))
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
