#include "Renderer/Renderer.h"
#include <stdexcept>
#include <iostream>
#include <cstdlib>

using namespace Mupfel;

int main()
{
	try
	{
		Renderer renderer;
		renderer.Init();
		renderer.Render();
		renderer.DeInit();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}