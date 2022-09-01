#pragma once

namespace Ion
{
	class ION_API EnginePath
	{
	public:
		static const FilePath& GetEnginePath();
		static FilePath GetShadersPath();
		static FilePath GetEngineContentPath();
		static FilePath GetFontsPath();
		static FilePath GetEditorContentPath();

	protected:
		template<typename T>
		static void SetEnginePath(const T& path);

	private:
		EnginePath() = delete;
		~EnginePath() = delete;

	private:
		static inline FilePath s_EnginePath = "";

		friend void ParseCommandLineArgs(int32 argc, tchar* argv[]);
	};

	inline const FilePath& EnginePath::GetEnginePath()
	{
		return s_EnginePath;
	}

	inline FilePath EnginePath::GetShadersPath()
	{
		return s_EnginePath / "Shaders";
	}

	inline FilePath EnginePath::GetEngineContentPath()
	{
		return s_EnginePath / "Content";
	}

	inline FilePath EnginePath::GetFontsPath()
	{
		return GetEngineContentPath() / "Fonts";
	}

	inline FilePath EnginePath::GetEditorContentPath()
	{
		return GetEngineContentPath() / "Editor";
	}

	template<typename T>
	inline void EnginePath::SetEnginePath(const T& path)
	{
		s_EnginePath.Set(path);
	}
}
