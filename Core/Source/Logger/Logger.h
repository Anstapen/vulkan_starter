#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <memory>
#include <string>

namespace Mupfel
{

	class Logger
	{
	public:
		typedef std::shared_ptr<spdlog::logger> SafeLoggerPtr;
	public:
		static bool Init();
		static std::shared_ptr<spdlog::logger> Create(const std::string& name);

	private:
		static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> my_console_sink;
		static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> my_file_sink;
	};
}