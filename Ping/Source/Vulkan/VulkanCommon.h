#pragma once
/**
 * Common vulkan-hpp include for every `Backend::` file: disables the aggregate-init struct
 * constructors (Vulkan structs are built with designated initializers instead) and makes
 * `vk::raii` swallow `VK_ERROR_OUT_OF_DATE_KHR` as success so swapchain acquire/present errors are
 * handled explicitly via return values (see `VulkanSwapChain::AcquireNextImage`) rather than
 * exceptions.
 */
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>