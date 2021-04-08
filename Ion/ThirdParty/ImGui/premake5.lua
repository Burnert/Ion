project "ImGui"
    kind "StaticLib"
    language "C"

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

    filter "system:windows"
        systemversion "latest"
        staticruntime "Off"

    -- filter "configurations:Debug"
	-- 	defines "ION_DEBUG"
	-- 	symbols "On"

	-- filter "configurations:Release"
	-- 	defines "ION_RELEASE"
	-- 	optimize "On"

	-- filter "configurations:Distribution"
	-- 	defines "ION_DIST"
	-- 	optimize "On"
