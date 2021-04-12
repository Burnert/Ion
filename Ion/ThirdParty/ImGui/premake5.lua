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
        "imgui/backends/imgui_impl_win32.cpp",
        "imgui/backends/imgui_impl_win32.h",
    }

    includedirs {
        "imgui",
        "../Glad/include"
    }

    links {
        "Glad"
    }

	flags {
		"MultiProcessorCompile"
	}

    filter "system:windows"
        systemversion "latest"
        staticruntime "Off"
		
		defines {
			"IMGUI_API=__declspec(dllexport)"
		}

    filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"

	filter "configurations:Distribution"
		optimize "On"
