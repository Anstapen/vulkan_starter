#include "Logger/Logger.h"

using namespace Mupfel;

std::shared_ptr<spdlog::sinks::basic_file_sink_mt> Logger::my_file_sink;
std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> Logger::my_console_sink;

bool Logger::Init()
{
    my_console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    my_console_sink->set_level(spdlog::level::info);
    my_console_sink->set_pattern("[%d.%m.%Y %T.%e] [%=15n] [%^%=10l%$] %v");

    my_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt", true);
    my_file_sink->set_level(spdlog::level::trace);
    my_file_sink->set_pattern("[%d.%m.%Y %T.%e] [%=15n] [%^%=10l%$] %v");

    spdlog::flush_every(std::chrono::milliseconds(500));

    return true;
}

std::shared_ptr<spdlog::logger> Logger::Create(const std::string& name)
{
    spdlog::logger* my_logger = new spdlog::logger(name, { my_console_sink, my_file_sink });
    std::shared_ptr<spdlog::logger> safe_log_ptr(my_logger);
    spdlog::register_logger(safe_log_ptr);

    return spdlog::get(name);
}
