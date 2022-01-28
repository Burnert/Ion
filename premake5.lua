workspace "Ion"
	architecture "x64"
	startproject "IonEditor"
	
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

-- Ion -----------------------------------------------------------

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
		"%{prj.name}/Source/**.inl",
	}

	includedirs {
		"%{prj.name}/ThirdParty/Glad/include",
		"%{prj.name}/ThirdParty/ImGui",
		"%{prj.name}/ThirdParty/glm",
		"%{prj.name}/ThirdParty/SpdLog/include",
		"%{prj.name}/ThirdParty/stb",
		"%{prj.name}/ThirdParty/rapidxml",
		"%{prj.name}/Source"
	}

	links {
		"Glad",
		"ImGui",
		"opengl32.lib"
	}
	
	flags {
		"MultiProcessorCompile"
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
			"{COPY} %{cfg.buildtarget.relpath} ../Build/" .. outputdir .. "/IonExample",
			"{MKDIR} ../Build/" .. outputdir .. "/IonEditor",
			"{COPY} %{cfg.buildtarget.relpath} ../Build/" .. outputdir .. "/IonEditor"
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
	
-- IonEditor -----------------------------------------------------------

project "IonEditor"
	location "IonEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	characterset "Unicode"

	pchheader "EditorPCH.h"
	pchsource "IonEditor/Source/EditorPCH.cpp"

	targetdir ("Build/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/Source/**.h",
		"%{prj.name}/Source/**.cpp",
		"%{prj.name}/Source/**.inl",
	}

	includedirs {
		"Ion/Source",
		"Ion/ThirdParty/Glad/include",
		"Ion/ThirdParty/ImGui",
		"Ion/ThirdParty/glm",
		"Ion/ThirdParty/SpdLog/include",
		"Ion/ThirdParty/stb",
		"Ion/ThirdParty/rapidxml",
		"%{prj.name}/Source"
	}

	links {
		"Ion"
	}

	flags {
		"MultiProcessorCompile"
	}

	defines {
		"ION_EDITOR"
	}

	filter "system:windows"
		staticruntime "Off"
		systemversion "latest"

		debugargs { "--enginePath", "%{wks.location}Ion" }

		defines {
			"ION_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "ION_EDITOR_DEBUG"
		defines "ION_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "ION_EDITOR_RELEASE"
		defines "ION_RELEASE"
		optimize "On"

	filter "configurations:Distribution"
		defines "ION_EDITOR_DIST"
		defines "ION_DIST"
		optimize "On"

-- IonExample -----------------------------------------------------------

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
		"%{prj.name}/Source/**.inl",
	}

	includedirs {
		"Ion/Source",
		"Ion/ThirdParty/ImGui",
		"Ion/ThirdParty/glm",
		"Ion/ThirdParty/SpdLog/include",
		"Ion/ThirdParty/rapidxml",
		"%{prj.name}/Source"
	}

	links {
		"Ion"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "Off"
		systemversion "latest"
		
		debugargs { "--enginePath", "%{wks.location}Ion" }

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
