
package.name = "http"
package.kind = "lib"
package.language = "c++"
package.config["Debug"].objdir = "obj/debug/libhttp"
package.config["Release"].objdir = "obj/release/libhttp"
package.buildflags = { "optimize-speed"}


AddCompFlags = ""
if(options["compiler_flags"]) then
    AddCompFlags = options["compiler_flags"]
end
 
if (linux) then
    package.buildoptions = { "-W -Wall -Wno-long-long --std=c++98 -pedantic -O3 " , AddCompFlags }
    package.defines = { "LIBVW_LINUX" }
end
if (windows) then
    package.buildflags = { "static-runtime" }
    package.defines = { "WIN32", "LIBVW_WINDOWS" }
    package.config["Debug"].defines = { "_DEBUG" }
    package.config["Release"].defines = { "NDEBUG" }
end

package.files = { 
    matchrecursive("../src/*.cpp", "../src/*.hpp" )
}

