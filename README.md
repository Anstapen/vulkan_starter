# Vulkan Starter

A Vulkan-based rendering hardware interface (RHI), split into two Premake projects.

`Ping` builds into a static library: the backend-agnostic RHI surface (`Ping::`, see `Ping/Source/Ping`) and its Vulkan implementation (`Backend::`, see `Ping/Source/Vulkan`), plus a small logging utility. `Examples` builds into an executable demonstrating `Ping`: the windowing, application, renderer, and ECS/event-system code, plus the runtime shaders/images/resources it uses. `Examples` links the `Ping` static library and provides an include path to its public headers.

The `Scripts/` directory contains build scripts for Windows and Linux, and the `Vendor/` directory contains Premake binaries.

## Getting Started
1. Set the `VULKAN_SDK` environment variable to your installed Vulkan SDK.
2. Open the `Scripts/` directory and run the appropriate `Setup` script to generate project files (Visual Studio 2026 for Windows, gmake2 for Linux — the Linux setup is unverified).
3. Compile shaders manually via `Examples/Shaders/compile.bat` (invokes `slangc`) before running.
4. Build via the generated solution/Makefiles and run the `Examples` project.

## Included
- The `Ping` RHI library (`Ping/Source`) and the `Examples` demo application (`Examples/Source`)
- Simple `.gitignore` to ignore project files and binaries
- Premake binaries for Windows (`5.0.0-beta8`)

## License
- UNLICENSE for this repository (see `UNLICENSE.txt` for more details)
- Premake is licensed under BSD 3-Clause (see included LICENSE.txt file for more details)
