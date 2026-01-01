#ifndef BASE_LOGGER_HPP
#define BASE_LOGGER_HPP

#include <memory>
#include <string_view>
namespace testing {

enum class LogLevel { /*Trace, Debug, */ Info, /*Warn,*/ Error };

struct BaseLogger {
  virtual ~BaseLogger() = default;
  virtual void write_log(LogLevel level, std::string_view message) = 0;
};

struct NullLogger : BaseLogger {
  void write_log(LogLevel level, std::string_view message) override {};
};

void set_global_logger(std::shared_ptr<BaseLogger> logger);
std::shared_ptr<BaseLogger> get_global_logger();

#define TESTING_LOG(level, message) get_global_logger()->write_log((level), (message))

}  // namespace testing
#endif  // !BASE_LOGGER_HPP
