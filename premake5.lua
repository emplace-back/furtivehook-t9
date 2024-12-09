workspace "furtivehook-t9"
    startproject "furtivehook-t9"
    location "build/"

    targetdir "%{wks.location}/bin/%{cfg.buildcfg}/"
    objdir "%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}/"
    buildlog "%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}.log"

    systemversion "latest"
    architecture "x64"
    characterset "mbcs"
    warnings "off"
    editandcontinue "off"
    staticruntime "on"

    disablewarnings {
        "4005",
        "4099",
        "5054",
        "26812"
    }
    
    includedirs {
        "src/",
        "$(DXSDK_DIR)include/",
    }

    buildoptions {
        "/Zm200",
        "/utf-8",
        "/std:c++latest"
    }

    flags {
        "shadowedvariables",
        "noincrementallink",
        "undefinedidentifiers",
        "multiprocessorcompile",
    }

    defines {
        "NOMINMAX",
        "WIN32_LEAN_AND_MEAN",
        "_CRT_SECURE_NO_WARNINGS",
        "ASMJIT_STATIC",
    }

    platforms {
        "x64",
    }

    configurations {
        "debug",
        "release",
    }

    configuration "debug"
        defines {
             "DEBUG",
        }

        optimize "debug"
        runtime "debug"
        symbols "on"

        targetname "furtivehook-t9-debug"

        postbuildcommands { "copy /Y \"%{wks.location}\\bin\\%{cfg.buildcfg}\\furtivehook-t9-debug.dll\" \"E:\\Games\\Steam\\steamapps\\common\\Call of Duty Black Ops Cold War\\webio.dll\"" }

    configuration "release"
        defines {
            "NDEBUG",
        }
        
        optimize "full"
        runtime "release"
        symbols "on"

        targetname "furtivehook-t9-release"

        postbuildcommands { "copy /Y \"%{wks.location}\\bin\\%{cfg.buildcfg}\\furtivehook-t9-release.dll\" \"E:\\Games\\Steam\\steamapps\\common\\Call of Duty Black Ops Cold War\\webio.dll\"" }

project "furtivehook-t9"
    kind "sharedlib"
    language "c++"
    
	dependson {
        "imgui",
    }
    
    files {
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
        "src/**.asm",
    }
    
	links {
        "imgui",
        "MinHook.lib",
        "ws2_32.lib",
    }
    
    includedirs {
        "src/",
        "$(DXSDK_DIR)include/",
    }

    libdirs {
        "thirdparty/minhook/",
        "$(DXSDK_DIR)lib/x64/",
    }

project "imgui"
    targetname "imgui"
    warnings "off"

    language "c++"
    kind "staticlib"

    files {
        "thirdparty/imgui/*.h",
        "thirdparty/imgui/*.hpp",
        "thirdparty/imgui/*.cpp",
    }

    includedirs {
        "thirdparty/imgui",
    }