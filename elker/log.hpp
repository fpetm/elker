#pragma once

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace elker {
	class Log {
	public:
		static void Init();
		static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }
	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};

}

#define EK_TRACE(...)		::elker::Log::GetLogger()->trace(__VA_ARGS__)
#define EK_INFO(...)		::elker::Log::GetLogger()->info(__VA_ARGS__)
#define EK_WARN(...)		::elker::Log::GetLogger()->warn(__VA_ARGS__)
#define EK_ERROR(...)		::elker::Log::GetLogger()->error(__VA_ARGS__)
#define EK_CRITICAL(...)	::elker::Log::GetLogger()->critical(__VA_ARGS__)