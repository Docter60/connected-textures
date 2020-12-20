-- premake5.lua
workspace "ct"
   configurations { "Debug", "Release" }
   platforms { "win32", "win64", "linux86", "linux64" }
   cppdialect "C++17"

project "ct"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   
   includedirs "lib/glm"

   files { "src/**.h", "src/**.c", "src/**.hpp", "src/**.cpp" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   filter "system:linux"
      links { "pthread" }
