project "imgui"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files
   {
      imgui_dir .. "/*.h",
      imgui_dir .. "/*.cpp",
      imgui_dir .. "/backends/imgui_impl_glfw.h",
      imgui_dir .. "/backends/imgui_impl_glfw.cpp"
   }

   includedirs
   {
      imgui_dir,
      imgui_dir .. "/backends",
      glfw_dir .. "/include"
   }

   targetdir ("Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

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
