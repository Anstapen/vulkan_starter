# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Premake5-based, Windows/MSVC (Visual Studio 2022) is the primary target; a Linux `gmake2` setup script also exists but is unverified.

- Requires the `VULKAN_SDK` environment variable to point at an installed Vulkan SDK — `Build.lua` aborts if it's unset.
- Generate project files: `Scripts\Setup-Windows.bat` (runs `premake5 --file=Build.lua vs2022` from the repo root) or `Scripts/Setup-Linux.sh` for `gmake2`.
- On first generation, Premake auto-downloads and unzips missing dependencies (GLM, GLFW 3.4, spdlog 1.17.0) into `Vendor/Sources/` — see `Dependencies.lua`.
- Build via the generated `Vulkan Starter.sln` / MSBuild; there is no CLI test runner or lint step in this repo.
- Configurations: `Debug`, `Release`, `Dist`. `Core` builds `warnings "Extra"` and (Release/Dist) `fatalwarnings "All"` — warnings in `Core/Source` will break non-Debug builds.
- Shaders are compiled manually, not as part of the main build: `App/Shaders/compile.bat` invokes `slangc` on `shader.slang` to produce `slang.spv` (targets SPIR-V 1.4, entry points `vertMain`/`fragMain`).

## Architecture

Two-project split: `Core` (static lib, all engine/RHI code) and `App` (executable, currently just `App/Source/App.cpp` constructing and running `Mupfel::Application`). `App` links `Core`, `vulkan`, `glfw3`, `spdlog`.

`Core/Source` is layered into three namespaces that must not be conflated:

- **`Ping::`** (`Core/Source/Ping/`) — the public, backend-agnostic RHI surface: `Device`, `SwapChain`, `Pipeline`, `CommandBuffer`, `Buffer`, and the plain-data types/enums in `Types.h` (`ImageLayout`, `BufferUsage`, `MemoryProperty`, etc.). Nothing outside `Core/Source/Vulkan` should need to include Vulkan headers directly — code should go through `Ping` types.
- **`Backend::`** (`Core/Source/Vulkan/`) — the Vulkan implementation of the `Ping` interface, built on `vk::raii` (vulkan-hpp RAII wrappers). `VKManager` is a static-method class holding essentially all raw Vulkan logic (instance/device creation, swapchain, pipeline, command buffers/pools, buffers, memory type selection, layout transitions, dynamic rendering setup). `VulkanContext`, `VulkanSwapChain`, `VulkanPipeline`, `VulkanCommandBuffer`, `VulkanCommandPool`, `VulkanQueue`, `VulkanBuffer` are the RAII-owning counterparts behind each `Ping` type. `VulkanTypeConversions.h/.cpp` holds the `Backend::ToVulkan(Ping::X)` mapping functions that bridge the two namespaces — new `Ping` enum values need a corresponding conversion here.
- **`Mupfel::`** (`Core/Source/Application/`, `Core/Source/Renderer/`) — the application/engine layer built on top of `Ping`. `Application` owns the `Window`, `Ping::Device`, `Renderer`, and `World`, and drives `Init()`/`MainLoop()`. `Renderer::RenderNextFrame` is the per-frame entry point, tracking its own swapchain/pipeline/command buffers/vertex buffers and a fixed `frames_in_flight = 2`.

`Core/Source/MupfelDeps/` is a vendored ECS + event system used by the `Mupfel` layer (not part of the RHI):

- `ECS/Registry` stores components in per-type sparse-set arrays (`CPUComponentArray`, swap-and-pop removal) and exposes `View<Components...>` for iteration plus `ParallelForEach` (chunked across `Application::GetCurrentThreadPool()`). Component type IDs come from `ComponentIndex::Index<T>()`, entity membership from a `std::bitset<64>` signature per entity.
- `Core/EventSystem` double-buffers events per-type: `AddEvent` is consumed one frame later, `AddImmediateEvent` additionally invokes registered listeners synchronously. `World` bundles a `Registry` and an `EventSystem`.

When adding a new RHI feature, the typical path touches four places in order: `Ping/Types.h` (if a new enum/flag is needed) → `Ping/<Thing>.h` (interface) → `Vulkan/Vulkan<Thing>.h/.cpp` (RAII object) → `Vulkan/VKManager.h/.cpp` (the actual Vulkan calls), plus `VulkanTypeConversions` if new `Ping` enums were added.

## Vulkan related Information

The actual information around the Vulkan API is retrieved from there websites:

* https://docs.vulkan.org/tutorial/latest/00_Introduction.html and all its sub-pages

* https://registry.khronos.org/vulkan/ and all its sub-pages

* the source code in the Vulkan SDK (pointed to by `VULKAN_SDK`)

When Vulkan-specific code is reviewed or vulkan-specifc questions are asked, these pages shall be visited first. If no answer could be found, ask before continuing the search elsewhere.

## Important Design Contraints

As this software project is a Rendering Hardware Interface (RHI), to be used by a Game Engine, these contraints are the most important and should be always considered:

1. the C++ source code should not create resource leaks

2. All resources should have a clearly defined lifetime, optimally with RAII

3. as this code will be used in a Game Engine, the code should not introduce unnecessary performance penalties

4. The usage of the C++ classes and function should be simple

## Coding Style & Naming Conventions

Formatting (indentation, brace placement, pointer/reference alignment) is enforced by the root `.clang-format` — see that file rather than this doc for the specifics.

Naming conventions aren't something `clang-format` enforces, so they're documented here instead:

- Header guards: `#pragma once` (no `#ifndef` guards).
- Types, namespaces, and public/private methods: `PascalCase` (`Device`, `CreateSwapChain`, `Ping::`, `Backend::`, `Mupfel::`). This applies to internal static helpers too (e.g. `VKManager`'s private methods) — use `PascalCase` even for methods with no external callers; don't use casing to signal "internal-only".
- Member variables: `camelCase` (e.g. `frameIndex`, `vulkanBufferPtr`, `commandBuffers`).
- Constructor/function parameters that shadow a member name get an `in_` prefix (e.g. `in_name`, `in_buffer`, `in_e`).
- Bitmask enums follow the `Ping/Types.h` pattern: `enum class X : uint32_t`, a constexpr `operator|`, and a free `HasFlag(value, flag)` helper — don't reach for `std::bitset` or raw integer flags for new bitmask types.

These naming/style rules were only just established (existing code predates them and is inconsistent in places, e.g. `Renderer.h` mixing `camelCase` and `snake_case` members) — apply them to new code, but don't drive-by reformat unrelated existing code to match.

## Documentation

API reference docs are generated from source comments with Doxygen (`Doxyfile` at the repo root,
`Scripts/Build-Docs.bat` / `.sh` runs it, output goes to the gitignored `Docs/Generated/`).
`JAVADOC_AUTOBRIEF` is on and `WARN_IF_UNDOCUMENTED` is enabled, so a class/method/member with *no*
documentation at all shows up as a Doxygen warning — running the build script is the way to check
coverage. `WARN_NO_PARAMDOC` is deliberately off: a one-line prose description is enough for a
trivial method, individual `@param`/`@return` tags are for where the parameter/return value needs
explanation beyond its name (matching this project's general no-comments-unless-non-obvious style).

Conventions:

- Doc comments live on the declaration in the header, not repeated on the `.cpp` definition.
- Classes, methods, and free functions use a `/** */` block. The first sentence becomes the brief
  description (`JAVADOC_AUTOBRIEF`); add a blank comment line before any further detail (lifetime/
  ownership notes, threading constraints, preconditions):
  ```cpp
  /**
   * Creates a swapchain for the given window.
   *
   * The returned SwapChain must not outlive this Device.
   *
   * @param in_window The window to present to.
   * @param frames_in_flight Number of frames to pipeline; must be >= 1.
   * @return A SwapChain ready for use with CommandBuffer submission.
   */
  SwapChain CreateSwapChain(const Window& in_window, uint32_t frames_in_flight) const;
  ```
  A single-line `/** Brief description. */` is fine for trivial methods with no parameters worth
  documenting.
- `@param` names match the actual parameter name, including the `in_` prefix where used.
- Every doc comment uses `/** ... */`, never `///` — including one-line ones, e.g. `/** Brief
  description. */`. A comment always goes on its own line(s) directly above the thing it documents,
  never trailing after it on the same line (so member variables, struct fields, and enumerators get
  a `/** Description. */` line above each one, not an after-the-fact `///<` comment).
- Use `@note` for non-obvious invariants and `@warning` for foot-guns (e.g. lifetime/ownership
  traps), reflecting this project's RAII/no-leak design constraints — that's the highest-value
  content in an RHI's docs, more so than restating what a method's name already says.
- Not every private implementation detail needs a comment — favor documenting the "why" and any
  caller-visible contract; skip comments that would just restate the signature.
