workspace "Ion"
	architecture "x64"
	startproject "IonExample"
	
	configurations {
		"Debug",
		"Release",
		"Distribution"
	}

outputdir = "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}"

ThirdParty = {
	Glad = "Ion/ThirdParty/Glad",
	ImGui = "Ion/ThirdParty/ImGui"
}

-- for i, dir in ipairs(ThirdParty) do
-- 	include(dir)
-- end

include "Ion/ThirdParty/Glad"
include "Ion/ThirdParty/ImGui"

project "Ion"
	location "Ion"
	kind "SharedLib"
	language "C++"
	characterset "Unicode"

	pchheader "IonPCH.h"
	pchsource "Ion/Source/IonPCH.cpp"

	targetdir ("Build/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/Source/**.h",
		"%{prj.name}/Source/**.cpp",
	}

	includedirs {
		"%{prj.name}/ThirdParty/Glad/include",
		"%{prj.name}/ThirdParty/ImGui",
		"%{prj.name}/ThirdParty/glm",
		"%{prj.name}/ThirdParty/SpdLog/include",
		"%{prj.name}/Source"
	}

	links {
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "Off"
		systemversion "latest"

		defines {
			"ION_PLATFORM_WINDOWS",
			"ION_ENGINE"
		}

		postbuildcommands {
			"{MKDIR} ../Build/" .. outputdir .. "/IonExample",
			"{COPY} %{cfg.buildtarget.relpath} ../Build/" .. outputdir .. "/IonExample"
		}

	filter "configurations:Debug"
		defines "ION_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "ION_RELEASE"
		optimize "On"

	filter "configurations:Distribution"
		defines "ION_DIST"
		optimize "On"
	
project "IonExample"
	location "IonExample"
	kind "ConsoleApp"
	language "C++"
	characterset "Unicode"

	targetdir ("Build/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/Source/**.h",
		"%{prj.name}/Source/**.cpp",
	}

	includedirs {
		"Ion/Source",
		"Ion/ThirdParty/ImGui",
		"Ion/ThirdParty/glm",
		"Ion/ThirdParty/SpdLog/include",
		"%{prj.name}/Source"
	}

	links {
		"Ion"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "Off"
		systemversion "latest"

		defines {
			"ION_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "ION_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "ION_RELEASE"
		optimize "On"

	filter "configurations:Distribution"
		defines "ION_DIST"
		optimize "On"
