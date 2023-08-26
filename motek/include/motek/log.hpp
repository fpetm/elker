#pragma once

#include <spdlog/spdlog.h>

namespace motek {
class Log {
public:
  static void Init();
  static std::shared_ptr<spdlog::logger> &GetMTLogger() { return s_MTLogger; }
  static std::shared_ptr<spdlog::logger> &GetEKLogger() { return s_EKLogger; }

  static bool Initialized() { return m_Initialized; }

private:
  static std::shared_ptr<spdlog::logger> s_MTLogger, s_EKLogger;
  static bool m_Initialized;
};

} // namespace motek

#define MT_TRACE(...) ::motek::Log::GetMTLogger()->trace(__VA_ARGS__)
#define MT_INFO(...) ::motek::Log::GetMTLogger()->info(__VA_ARGS__)
#define MT_WARN(...) ::motek::Log::GetMTLogger()->warn(__VA_ARGS__)
#define MT_ERROR(...) ::motek::Log::GetMTLogger()->error(__VA_ARGS__)
#define MT_CRITICAL(...) ::motek::Log::GetMTLogger()->critical(__VA_ARGS__)

#define EK_TRACE(...) ::motek::Log::GetEKLogger()->trace(__VA_ARGS__)
#define EK_INFO(...) ::motek::Log::GetEKLogger()->info(__VA_ARGS__)
#define EK_WARN(...) ::motek::Log::GetEKLogger()->warn(__VA_ARGS__)
#define EK_ERROR(...) ::motek::Log::GetEKLogger()->error(__VA_ARGS__)
#define EK_CRITICAL(...) ::motek::Log::GetEKLogger()->critical(__VA_ARGS__)
