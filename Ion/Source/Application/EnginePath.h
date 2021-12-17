#pragma once

#include "Core/File/File.h"

namespace Ion
{
	class ION_API EnginePath
	{
	public:
		static inline const FilePath& GetEnginePath()
		{
			return s_EnginePath;
		}

		/* Gets the engine path with path validation */
		static inline FilePath GetCheckedEnginePath()
		{
			return FilePath(s_EnginePath, EFilePathValidation::Checked);
		}

		static inline FilePath GetShadersPath()
		{
			return s_EnginePath + L"Shaders";
		}

		static inline FilePath GetCheckedShadersPath()
		{
			return GetCheckedEnginePath() + L"Shaders";
		}

	protected:
		static inline void SetEnginePath(const WString& path)
		{
			s_EnginePath.Set(path);
		}

	private:
		EnginePath() = delete;
		~EnginePath() = delete;

	private:
		static inline FilePath s_EnginePath = L"";

		template<typename T>
		friend void ParseCommandLineArgs(int32 argc, T* argv[]);
	};
}
