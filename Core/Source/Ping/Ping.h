#pragma once
#include "Device.h"

namespace Ping
{
/**
 * Initializes the Vulkan backend (currently just `Backend::VKManager::Init`). Call once before
 * constructing a `Device`.
 */
bool Init();

/** Tears down the Vulkan backend. Call once after all `Ping` resources have been destroyed. */
void Shutdown();
}; // namespace Ping
