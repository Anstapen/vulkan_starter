#include "Gui.h"
#include "Vulkan/VulkanGui.h"

using namespace Ping;

Gui::Gui(Backend::VulkanGui&& in_gui) noexcept : vulkanGuiPtr(std::make_unique<Backend::VulkanGui>(std::move(in_gui)))
{
}

Gui::~Gui() {}

Gui::Gui(Gui&& other) noexcept : vulkanGuiPtr(std::move(other.vulkanGuiPtr)) {}

Gui& Gui::operator=(Gui&& other) noexcept
{
	vulkanGuiPtr = std::move(other.vulkanGuiPtr);
	return *this;
}

void Gui::NewFrame() { vulkanGuiPtr->NewFrame(); }
