-- c4core

project "c4core"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	targetdir ("Build/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

	files {
		"rapidyaml/ext/c4core/src/c4/allocator.hpp",
		"rapidyaml/ext/c4core/src/c4/base64.hpp",
		"rapidyaml/ext/c4core/src/c4/base64.cpp",
		"rapidyaml/ext/c4core/src/c4/blob.hpp",
		"rapidyaml/ext/c4core/src/c4/bitmask.hpp",
		"rapidyaml/ext/c4core/src/c4/charconv.hpp",
		"rapidyaml/ext/c4core/src/c4/c4_pop.hpp",
		"rapidyaml/ext/c4core/src/c4/c4_push.hpp",
		"rapidyaml/ext/c4core/src/c4/char_traits.cpp",
		"rapidyaml/ext/c4core/src/c4/char_traits.hpp",
		"rapidyaml/ext/c4core/src/c4/common.hpp",
		"rapidyaml/ext/c4core/src/c4/compiler.hpp",
		"rapidyaml/ext/c4core/src/c4/config.hpp",
		"rapidyaml/ext/c4core/src/c4/cpu.hpp",
		"rapidyaml/ext/c4core/src/c4/ctor_dtor.hpp",
		"rapidyaml/ext/c4core/src/c4/dump.hpp",
		"rapidyaml/ext/c4core/src/c4/enum.hpp",
		"rapidyaml/ext/c4core/src/c4/error.cpp",
		"rapidyaml/ext/c4core/src/c4/error.hpp",
		"rapidyaml/ext/c4core/src/c4/export.hpp",
		"rapidyaml/ext/c4core/src/c4/format.hpp",
		"rapidyaml/ext/c4core/src/c4/format.cpp",
		"rapidyaml/ext/c4core/src/c4/hash.hpp",
		"rapidyaml/ext/c4core/src/c4/language.hpp",
		"rapidyaml/ext/c4core/src/c4/language.cpp",
		"rapidyaml/ext/c4core/src/c4/memory_resource.cpp",
		"rapidyaml/ext/c4core/src/c4/memory_resource.hpp",
		"rapidyaml/ext/c4core/src/c4/memory_util.cpp",
		"rapidyaml/ext/c4core/src/c4/memory_util.hpp",
		"rapidyaml/ext/c4core/src/c4/platform.hpp",
		"rapidyaml/ext/c4core/src/c4/preprocessor.hpp",
		"rapidyaml/ext/c4core/src/c4/restrict.hpp",
		"rapidyaml/ext/c4core/src/c4/span.hpp",
		"rapidyaml/ext/c4core/src/c4/std/std.hpp",
		"rapidyaml/ext/c4core/src/c4/std/std_fwd.hpp",
		"rapidyaml/ext/c4core/src/c4/std/string.hpp",
		"rapidyaml/ext/c4core/src/c4/std/string_fwd.hpp",
		"rapidyaml/ext/c4core/src/c4/std/tuple.hpp",
		"rapidyaml/ext/c4core/src/c4/std/vector.hpp",
		"rapidyaml/ext/c4core/src/c4/std/vector_fwd.hpp",
		"rapidyaml/ext/c4core/src/c4/substr.hpp",
		"rapidyaml/ext/c4core/src/c4/substr_fwd.hpp",
		"rapidyaml/ext/c4core/src/c4/szconv.hpp",
		"rapidyaml/ext/c4core/src/c4/type_name.hpp",
		"rapidyaml/ext/c4core/src/c4/types.hpp",
		"rapidyaml/ext/c4core/src/c4/unrestrict.hpp",
		"rapidyaml/ext/c4core/src/c4/utf.hpp",
		"rapidyaml/ext/c4core/src/c4/utf.cpp",
		"rapidyaml/ext/c4core/src/c4/windows.hpp",
		"rapidyaml/ext/c4core/src/c4/windows_pop.hpp",
		"rapidyaml/ext/c4core/src/c4/windows_push.hpp",
		"rapidyaml/ext/c4core/src/c4/c4core.natvis",
		-- External
		"rapidyaml/ext/c4core/src/c4/ext/debugbreak/debugbreak.h",
		"rapidyaml/ext/c4core/src/c4/ext/rng/rng.hpp",
		"rapidyaml/ext/c4core/src/c4/ext/sg14/inplace_function.h",
		-- Fast Float
		"rapidyaml/ext/c4core/src/c4/ext/fast_float.hpp",
		"rapidyaml/ext/c4core/src/c4/ext/fast_float_all.h"
	}

	includedirs {
		"rapidyaml/ext/c4core/src"
	}

	flags {
		"MultiProcessorCompile"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"

	filter "configurations:Distribution"
		optimize "On"

-- rapidyaml

project "rapidyaml"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	targetdir ("Build/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

	files {
		"rapidyaml/src/ryml.hpp",
		"rapidyaml/src/ryml_std.hpp",
		"rapidyaml/src/c4/yml/detail/checks.hpp",
		"rapidyaml/src/c4/yml/detail/parser_dbg.hpp",
		"rapidyaml/src/c4/yml/detail/print.hpp",
		"rapidyaml/src/c4/yml/detail/stack.hpp",
		"rapidyaml/src/c4/yml/common.hpp",
		"rapidyaml/src/c4/yml/common.cpp",
		"rapidyaml/src/c4/yml/emit.def.hpp",
		"rapidyaml/src/c4/yml/emit.hpp",
		"rapidyaml/src/c4/yml/export.hpp",
		"rapidyaml/src/c4/yml/node.hpp",
		"rapidyaml/src/c4/yml/node.cpp",
		"rapidyaml/src/c4/yml/parse.hpp",
		"rapidyaml/src/c4/yml/parse.cpp",
		"rapidyaml/src/c4/yml/preprocess.hpp",
		"rapidyaml/src/c4/yml/preprocess.cpp",
		"rapidyaml/src/c4/yml/std/map.hpp",
		"rapidyaml/src/c4/yml/std/std.hpp",
		"rapidyaml/src/c4/yml/std/string.hpp",
		"rapidyaml/src/c4/yml/std/vector.hpp",
		"rapidyaml/src/c4/yml/tree.hpp",
		"rapidyaml/src/c4/yml/tree.cpp",
		"rapidyaml/src/c4/yml/writer.hpp",
		"rapidyaml/src/c4/yml/yml.hpp",
		"rapidyaml/src/ryml.natvis"
	}

	includedirs {
		"rapidyaml/ext/c4core/src",
		"rapidyaml/src"
	}

	links {
		"c4core"
	}

	flags {
		"MultiProcessorCompile"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"

	filter "configurations:Distribution"
		optimize "On"
