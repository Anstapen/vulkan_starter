#include "Renderer.h"

using namespace Mupfel;

void Mupfel::Renderer::Init(Ping::Device& device)
{
	logger = Logger::Create("Renderer");
	logger->info("Init");
}
