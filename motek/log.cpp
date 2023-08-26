#include <motek/log.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace motek {
std::shared_ptr<spdlog::logger> Log::s_MTLogger, Log::s_EKLogger;

void Log::Init() {
  std::vector<spdlog::sink_ptr> sinks;

  sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
  sinks.emplace_back(
      std::make_shared<spdlog::sinks::basic_file_sink_mt>("motek.log"));

  sinks[0]->set_pattern("%^[%T] %n: %v%$");
  sinks[1]->set_pattern("[%T] [%l] %n: %v");

  s_MTLogger =
      std::make_shared<spdlog::logger>("motek", begin(sinks), end(sinks));
  spdlog::register_logger(s_MTLogger);
  s_MTLogger->set_level(spdlog::level::trace);
  s_MTLogger->flush_on(spdlog::level::trace);

  s_EKLogger =
      std::make_shared<spdlog::logger>("elker", begin(sinks), end(sinks));
  spdlog::register_logger(s_EKLogger);
  s_EKLogger->set_level(spdlog::level::trace);
  s_EKLogger->flush_on(spdlog::level::trace);
}
} // namespace motek