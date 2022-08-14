project "ImGui"
    kind "StaticLib"
    language "C++"

    targetdir ("Build/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

    files {
        "imgui/*.cpp",
        "imgui/*.h",
        "imgui/backends/imgui_impl_opengl3.cpp",
        "imgui/backends/imgui_impl_opengl3.h",
        "imgui/backends/imgui_impl_dx11.cpp",
        "imgui/backends/imgui_impl_dx11.h",
        "imgui/backends/imgui_impl_dx10.cpp",
        "imgui/backends/imgui_impl_dx10.h",
        "imgui/backends/imgui_impl_win32.cpp",
        "imgui/backends/imgui_impl_win32.h",
        "imgui/misc/freetype/imgui_freetype.h",
        "imgui/misc/freetype/imgui_freetype.cpp",
    }

    includedirs {
        "imgui",
        "../Glad/include",
        "../FreeType/freetype/include"
    }

    links {
        "Glad",
        "FreeType"
    }

    defines {
        "IMGUI_ENABLE_FREETYPE"
    }

	flags {
		"MultiProcessorCompile"
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
