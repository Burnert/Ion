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
			return s_EnginePath + "Shaders";
		}

		static inline FilePath GetCheckedShadersPath()
		{
			return GetCheckedEnginePath() + "Shaders";
		}

		static inline FilePath GetEngineContentPath()
		{
			return s_EnginePath + "Content";
		}

		static inline FilePath GetCheckedContentPath()
		{
			return GetCheckedEnginePath() + "Content";
		}

		static inline FilePath GetFontsPath()
		{
			return GetEngineContentPath() + "Fonts";
		}

		static inline FilePath GetCheckedFontsPath()
		{
			return GetCheckedContentPath() + "Fonts";
		}

		static inline FilePath GetEditorContentPath()
		{
			return GetEngineContentPath() + "Editor";
		}

		static inline FilePath GetCheckedEditorContentPath()
		{
			return GetCheckedContentPath() + "Editor";
		}

	protected:
		template<typename T>
		static inline void SetEnginePath(const T& path)
		{
			s_EnginePath.Set(path);
		}

	private:
		EnginePath() = delete;
		~EnginePath() = delete;

	private:
		static inline FilePath s_EnginePath = "";

		template<typename T>
		friend void ParseCommandLineArgs(int32 argc, T* argv[]);
	};
}
