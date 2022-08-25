#pragma once

#include "DXCommon.h"

namespace Ion
{
	class ION_API DXInclude : public ID3DInclude
	{
	public:
		DXInclude();
		DXInclude(const FilePath& sourceDir);

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

	private:
		FilePath m_SourceDir;
	};
}
