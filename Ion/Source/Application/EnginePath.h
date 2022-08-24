#pragma once

namespace Ion
{
	class ION_API EnginePath
	{
	public:
		static inline const FilePath& GetEnginePath()
		{
			return s_EnginePath;
		}

		static inline FilePath GetShadersPath()
		{
			return s_EnginePath / "Shaders";
		}

		static inline FilePath GetEngineContentPath()
		{
			return s_EnginePath / "Content";
		}

		static inline FilePath GetFontsPath()
		{
			return GetEngineContentPath() / "Fonts";
		}

		static inline FilePath GetEditorContentPath()
		{
			return GetEngineContentPath() / "Editor";
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
