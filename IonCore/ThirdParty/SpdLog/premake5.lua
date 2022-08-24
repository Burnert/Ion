project "SpdLog"
    kind "StaticLib"
    language "C++"
	cppdialect "C++17"

	pchheader "pch.h.in"
	pchsource "spdlog_pch.cpp"

    targetdir ("Build/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        "SpdLog/include/spdlog/**.h",
		"SpdLog/cmake/pch.h.in",
		"spdlog_pch.cpp",
		"pchinject/*.cpp"
    }

    includedirs {
		"SpdLog/cmake",
		"SpdLog/include",
		"SpdLog/src"
    }

    links {
    }

	flags {
		"MultiProcessorCompile"
	}

	defines {
		"SPDLOG_COMPILED_LIB",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT",
	}

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"
		
		defines {
		}

    filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"

	filter "configurations:Distribution"
		optimize "On"
