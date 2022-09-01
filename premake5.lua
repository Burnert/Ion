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
	SpdLog   = "IonCore/ThirdParty/SpdLog",
	FreeType = "Ion/ThirdParty/FreeType",
	Glad     = "Ion/ThirdParty/Glad",
	ImGui    = "Ion/ThirdParty/ImGui",
}

for name, dir in pairs(ThirdParty) do
	include(dir)
end

-- IonCore -----------------------------------------------------------

project "IonCore"
	location "IonCore"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	characterset "Unicode"

	pchheader "Core/CorePCH.h"
	pchsource "IonCore/Source/Core/CorePCH.cpp"

	targetdir ("Build/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/Source/**.h",
		"%{prj.name}/Source/**.cpp",
		"%{prj.name}/Source/**.inl",
	}

	includedirs {
		"%{prj.name}/Source",
		"%{prj.name}/ThirdParty/glm",
		"%{prj.name}/ThirdParty/SpdLog/SpdLog/include",
		"%{prj.name}/ThirdParty/stb",
		"%{prj.name}/ThirdParty/rapidxml",
	}

	links {
		"SpdLog"
	}
	
	flags {
		"MultiProcessorCompile"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines {
			"ION_STATIC_LIB",
			"ION_PLATFORM_WINDOWS",
			"ION_CORE",
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

-- Ion -----------------------------------------------------------

project "Ion"
	location "Ion"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
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
		"IonCore/Source",
		"IonCore/ThirdParty/glm",
		"IonCore/ThirdParty/SpdLog/SpdLog/include",
		"IonCore/ThirdParty/stb",
		"IonCore/ThirdParty/rapidxml",
		"%{prj.name}/Source",
		"%{prj.name}/ThirdParty/FreeType/freetype/include",
		"%{prj.name}/ThirdParty/Glad/include",
		"%{prj.name}/ThirdParty/ImGui",
	}

	links {
		"IonCore",
		"FreeType",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}
	
	flags {
		"MultiProcessorCompile"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines {
			"ION_STATIC_LIB",
			"ION_PLATFORM_WINDOWS",
			"ION_ENGINE",
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
		"IonCore/Source",
		"IonCore/ThirdParty/glm",
		"IonCore/ThirdParty/SpdLog/SpdLog/include",
		"IonCore/ThirdParty/stb",
		"IonCore/ThirdParty/rapidxml",
		"Ion/Source",
		"Ion/ThirdParty/Glad/include",
		"Ion/ThirdParty/ImGui",
		"%{prj.name}/Source",
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
		staticruntime "On"
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
	cppdialect "C++17"
	characterset "Unicode"

	pchheader "IonPCH.h"
	pchsource "IonExample/Source/PCH.cpp"

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
		"Ion/ThirdParty/SpdLog/SpdLog/include",
		"Ion/ThirdParty/rapidxml",
		"%{prj.name}/Source"
	}

	links {
		"Ion"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"
		
		debugargs { "-e", "%{wks.location}Ion" }

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
