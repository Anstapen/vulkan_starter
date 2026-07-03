#pragma once
#include <string>
#include <optional>
#include "Renderer/Renderer.h"
#include "Logger/Logger.h"
#include "Window/Window.h"

/* Graphics API */
#include "Ping/Device.h"

namespace Mupfel
{
	class Application
	{
    public:
        Application(const std::string& in_name);

    public:
        void Run();

    private:
        void Init();
        void MainLoop();
        void CleanUp();

    private:
        const std::string name;
        Renderer renderer;
        Logger::SafeLoggerPtr logger;
		std::optional<Ping::Device> device;
		std::optional<Window> window;
	};

}