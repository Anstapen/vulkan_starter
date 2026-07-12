#pragma once
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <memory>
#include <string>

namespace Mupfel
{

/**
 * Static factory for named spdlog loggers, all sharing one color console sink and one file sink
 * (`logs/log.txt`).
 */
class Logger
{
public:
	/** Shared-ownership handle to a logger returned by `Create`. */
	typedef std::shared_ptr<spdlog::logger> SafeLoggerPtr;

public:
	/**
	 * Creates the shared console (info level) and file (trace level, `logs/log.txt`, truncated) sinks,
	 * and starts spdlog's background flush timer. Must be called once before `Create`.
	 */
	static bool Init();

	/** Creates and registers a new named logger writing to both shared sinks. */
	static std::shared_ptr<spdlog::logger> Create(const std::string& name);

private:
	/** Shared color console sink. */
	static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> my_console_sink;
	/** Shared file sink (`logs/log.txt`). */
	static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> my_file_sink;
};
} // namespace Mupfel