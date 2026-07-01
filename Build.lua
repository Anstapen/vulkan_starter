-- premake5.lua


-- we need the VULKAN_SDK as an environment variable
vulkan_sdk_path = os.getenv("VULKAN_SDK")
print(vulkan_sdk_path)
if not vulkan_sdk_path then
error("VULKAN_SDK not set. Please set this variable to the path of the installed Vulakn SDK. Exiting...")
end

include "Dependencies.lua"

workspace "Vulkan Starter"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "App"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/utf-8"}

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Core"
	include "Core/Build-Core.lua"
group ""

include "Build-spdlog.lua"

include "App/Build-App.lua"
