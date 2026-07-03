#pragma once

#include "Logger/Logger.h"
#include "Ping/Device.h"

namespace Mupfel {

	class Renderer
	{
	public:
		void Init(Ping::Device &device);


	private:

		Logger::SafeLoggerPtr logger;
	};

}


