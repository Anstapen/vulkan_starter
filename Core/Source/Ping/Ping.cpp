#include "Ping.h"
#include "Vulkan/VKManager.h"

bool Ping::Init()
{
	Backend::VKManager::Init();
	return true;
}

void Ping::Shutdown() { Backend::VKManager::Shutdown(); }
