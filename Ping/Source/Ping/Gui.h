#pragma once
#include <memory>

namespace Backend
{
class VulkanGui;
}

namespace Ping
{
class Device;
class CommandBuffer;

class Gui
{
	friend class CommandBuffer;

public:
	Gui(Backend::VulkanGui&& in_gui) noexcept;
	~Gui();
	Gui(const Gui& other) = delete;
	Gui(Gui&& other) noexcept;
	Gui& operator=(const Gui& other) = delete;
	Gui& operator=(Gui&& other) noexcept;

	void NewFrame();

private:
	std::unique_ptr<Backend::VulkanGui> vulkanGuiPtr;
};

} // namespace Ping
