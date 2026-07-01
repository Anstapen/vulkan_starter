project "App"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp", "Shaders/**.glsl" }

   includedirs
   {
      "Source",

	  -- Include Core
	  "../Core/Source",
       "../Vendor/Sources/glfw-3.4.bin.WIN64/include",
       vulkan_sdk_path .. "/Include",
       "../" .. spdlog_dir .. "/include"
   }
   
   libdirs {vulkan_sdk_path .. "/Lib"}
   libdirs {"../" .. glfw_dir .. "/lib-vc2022"}

   links
   {
      "Core",
      "vulkan",
      "glfw3",
      "spdlog"
   }

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"
