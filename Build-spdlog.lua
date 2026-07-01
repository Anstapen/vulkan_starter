project "spdlog"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"
   
   spdlog_header = spdlog_dir .. "/include"
   spdlog_sources = spdlog_dir .. "/src"

   files { spdlog_header .. "/**.h", spdlog_sources .. "/**.cpp" }

   includedirs
   {
      spdlog_header
   }

   targetdir ("Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")
   
   defines { "SPDLOG_COMPILED_LIB" }

   filter "action:vs*"
       defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
       characterset ("Unicode")

   filter "system:windows"
       systemversion "latest"
       defines { }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"
       warnings "Extra"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"
       fatalwarnings {"All"}
       warnings "Extra"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"
       fatalwarnings {"All"}
       warnings "Extra"