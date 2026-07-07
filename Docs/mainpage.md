# Vulkan Starter

This is the generated API reference for the Vulkan Starter codebase: the `Ping` rendering
hardware interface (RHI), its Vulkan backend, and the `Mupfel` engine layer built on top of it.

## Layers

- **Ping** — the public, backend-agnostic RHI surface (`Device`, `SwapChain`, `Pipeline`,
  `CommandBuffer`, `Buffer`, and the plain-data types in `Types.h`). Application and engine code
  should only ever need to include `Ping/` headers.
- **Backend** — the Vulkan implementation of the `Ping` interface, built on `vk::raii`.
  `VKManager` holds the raw Vulkan calls; the `Vulkan*` RAII classes are the owning counterparts
  behind each `Ping` type. `VulkanTypeConversions` bridges `Ping` enums to their Vulkan equivalents.
- **Mupfel** — the application/engine layer (`Application`, `Renderer`, `World`) built on top of
  `Ping`, plus a vendored ECS and double-buffered event system under `MupfelDeps/`.

See `CLAUDE.md` in the repository root for the full architecture description and coding
conventions.
