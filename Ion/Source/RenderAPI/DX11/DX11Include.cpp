#include "IonPCH.h"

#include "Application/Application.h"
#include "DX11Include.h"

namespace Ion
{
	DX11Include::DX11Include() { }

	HRESULT DX11Include::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		FilePath includePath(L"");

		switch (IncludeType)
		{
		case D3D_INCLUDE_LOCAL:
			includePath = Application::GetCheckedEnginePath() + L"Shaders";
			break;
		// case D3D_INCLUDE_SYSTEM:
			// @TODO: Add system includes
		default:
			return E_FAIL;
		}

		File includeFile(includePath + StringConverter::StringToWString(pFileName), EFileMode::Read);

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

	HRESULT DX11Include::Close(LPCVOID pData)
	{
		if (pData)
		{
			free((void*)pData);
		}

		return S_OK;
	}
}