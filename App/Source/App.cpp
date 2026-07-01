#include <stdexcept>
#include <cstdlib>
#include "Logger/Logger.h"
#include "Application/Application.h"

using namespace Mupfel;

int main()
{
	Logger::Init();

	auto main_logger = Logger::Create("main");
	main_logger->info("Starting application...");
	try
	{
		Application app("Vulkan Playground");
		app.Run();
	}
	catch (const std::exception &e)
	{
		main_logger->error("Exception: {}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}