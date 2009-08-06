solution "libnetpp"
    configurations { "Debug" , "Release" }
    
    project "netpp"
        kind "StaticLib"
        uuid "d7409f46-a711-4f20-8ef5-bc5f28b20be8"
        basedir "."
        files { "net/**.cpp", "net/**.hpp" }
        language "C++"
        includedirs { "." }
        flags { "Symbols" }

        configuration "Debug"
            targetdir "bin/debug"
            defines { "DEBUG" }
            flags { "Symbols" }
 
        configuration "Release"
            targetdir "bin/release"
            defines { "NDEBUG" }
            flags { "Optimize" }         


    project "tests"
        kind "ConsoleApp"
        language "C++"
        uuid "338c5481-613c-4770-8db9-d0e654901ced"
        basedir "."
        files { "test/**.cpp" }
        includedirs { "." }
        links { "netpp" }

        configuration "linux"
            buildoptions { "-W", "-Wall", "-Wno-long-long", "-std=c++98", "-pedantic"}

        configuration "windows"
            defines { "WIN32" }
    
        configuration "Debug"
            targetdir "bin/debug"
            defines { "DEBUG" }
            flags { "Symbols" }
 
        configuration "Release"
            targetdir "bin/release"
            defines { "NDEBUG" }
            flags { "Optimize" }         



