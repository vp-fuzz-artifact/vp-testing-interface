#include "vp_test/base_logger.hpp"
#include <mutex>

namespace testing {
namespace {
std::shared_ptr<BaseLogger> g_logger = std::make_shared<NullLogger>();
std::mutex g_logger_mutex;
} // namespace

void set_global_logger(std::shared_ptr<BaseLogger> logger) {
  std::lock_guard lock(g_logger_mutex);
  if (logger) {
    g_logger = std::move(logger);
  } else {
    g_logger = std::make_shared<NullLogger>();
  }
}

std::shared_ptr<BaseLogger> get_global_logger() {
  std::lock_guard lock(g_logger_mutex);
  return g_logger;
}

} // namespace testing
