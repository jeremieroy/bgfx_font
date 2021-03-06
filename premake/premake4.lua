-- Dependencies and others directories
BGFX_DIR = "../../bgfx/"
BX_DIR = "../../bx/"

RUNTIME_DIR = "../_runtime/"
BIN_DIR = "../_bin/"
BUILD_DIR = "../_build/"

INPUT_SHADERS_DIR = "../shaders/"

solution "bgfx_font"
configurations {
		"Debug",
		"Release",
	}

	platforms {
		"x32"
	}

	language "C++"

-- bx toolchain
dofile (BX_DIR .. "premake/toolchain.lua")

local BGFX_BUILD_DIR = BUILD_DIR
local BGFX_THIRD_PARTY_DIR = (BGFX_DIR .. "3rdparty/")
toolchain(BGFX_BUILD_DIR, BGFX_THIRD_PARTY_DIR)

 -- setup bgfx project
function copyLib()
end
dofile (BGFX_DIR .. "premake/bgfx.lua")

-- bgfx_font as static lib
project "bgfx_font" 
	includedirs {  BX_DIR .. "include", BGFX_DIR .. "include", "../include", BGFX_DIR .. "3rdparty/edtaa3" }
	files { "../include/**.*", 
			"../src/**.h" , "../src/**.cpp",
			BGFX_DIR .. "3rdparty/edtaa3/**.*"
	}
	flags{ "Symbols" } 
	kind "StaticLib" 

function exampleProject(_name, _uuid)

	project (_name)
		uuid (_uuid)
		kind "WindowedApp"

	configuration {}

	debugdir (RUNTIME_DIR)

	includedirs {
		BX_DIR .. "include",
		BGFX_DIR .. "include",
		BGFX_DIR .. "examples",
		"../include"
	}

	files {
		BGFX_DIR .. "examples/common/**.cpp",
		BGFX_DIR .. "examples/common/**.h",
		"../samples/" .. _name .. "/**.cpp",
		"../samples/" .. _name .. "/**.h"
	}

	links {
		"bgfx", "bgfx_font"
	}

	configuration { "emscripten" }
		targetextension ".bc"

	configuration { "nacl or nacl-arm or pnacl" }
		targetextension ".nexe"
		links {
			"ppapi",
			"ppapi_gles2",
			"pthread",
		}

	configuration { "nacl", "Release" }
		postbuildcommands {
			"@echo Stripping symbols.",
			"@$(NACL)/bin/x86_64-nacl-strip -s \"$(TARGET)\""
		}

	configuration { "linux" }
		links {
			"GL",
			"pthread",
		}

	configuration { "macosx" }
		files {
			BGFX_DIR .. "examples/common/**.mm",
		}
		links {
			"Cocoa.framework",
			"OpenGL.framework",
		}
end

-- bgfxGwenSample project

--"057AB1A7-8246-412F-ACC6-0186DA1749E9"
--"B48F9186-FF39-46BC-9038-4A6AD288B972"
--"B48F9186-FF39-46BC-9038-4A6AD288B972"
--"CD39C9B9-6DB0-4323-8AD3-FC147D9C91A4"
--"CB4B9765-24BE-418B-A36B-817482B7AE32"
exampleProject("01_basics", "EF6FD5B3-B52A-41C2-A257-9DFE709AF9E1")
exampleProject("02_distance_field_text", "F4E6F96F-3DAA-4C68-8DF8-BF2A3ECD9092")
--exampleProject("03_show_texture_atlas", "EF6FD5B3-B52A-41C2-A257-9DFE709AF9E1")
--exampleProject("04_buffer_type", "EF6FD5B3-B52A-41C2-A257-9DFE709AF9E1")



--helpers for shader
dofile "bgfx_compile_shader.lua"

--explicitely compile shaders during a project, the hacky way :/
if _ACTION ~= "clean" and _ACTION ~= "shaders" then
  bgfx_compile_shaders(INPUT_SHADERS_DIR, RUNTIME_DIR, BUILD_DIR)
end
--

-- define a new global action that can be used to recompile shaders easily
-- usage: "premake4 shaders"  (e.g. instead of premake4 vs2010)
newaction {
    trigger = 'shaders',
    description = 'Bake shaders',
    shortname = "Bake shaders",
    --valid_kinds = premake.action.get("*").valid_kinds,
    --valid_languages = premake.action.get("*").valid_languages,
    --valid_tools = premake.action.get("*").valid_tools,
    execute = function()
		bgfx_compile_shaders(INPUT_SHADERS_DIR, RUNTIME_DIR, BUILD_DIR)
    end
}

