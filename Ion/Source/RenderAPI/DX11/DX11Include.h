#pragma once

#include "DX11.h"

namespace Ion
{
	class ION_API DX11Include : public ID3DInclude
	{
	public:
		DX11Include();

		virtual HRESULT WINAPI Open(
			D3D_INCLUDE_TYPE IncludeType,
			LPCSTR           pFileName,
			LPCVOID          pParentData,
			LPCVOID* ppData,
			UINT* pBytes
		) override;

		virtual HRESULT WINAPI Close(
			LPCVOID pData
		) override;
	};
}
