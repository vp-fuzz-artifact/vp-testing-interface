#ifndef VP_COMM_HPP
#define VP_COMM_HPP

#include <cstring>
#include <fcntl.h>
#include <array>
#include <optional>
#include <chrono>

#include "types.hpp"

namespace testing {

enum class CommState { UNINITIALIZED, INITIALIZED, READY };

class VpComm {
 public:
  VpComm();
  ~VpComm();

  bool init();
  void reset();
  void set_ready();
  void close_temp_pipes();
  bool wait_for_ready();
  void clear_pipe(int fd) const;

  [[nodiscard]] int get_request_fd_0() const;
  [[nodiscard]] int get_request_fd_1() const;
  [[nodiscard]] int get_response_fd_0() const;
  [[nodiscard]] int get_response_fd_1() const;


  bool send_request(Request &req, Response &res, std::optional<std::chrono::milliseconds> timeout = std::nullopt);
  bool receive_response(Response &res);
  bool restore_send_request(Request &req, Response &res); // added this function to receive additional Status_Ok leftover from restore

  static void no_logging(const char *fmt, ...) {};
  void (*log_info_message)(const char *fmt, ...) = no_logging;
  void (*log_error_message)(const char *fmt, ...) = no_logging;

 private:
  CommState comm_state;

  int  m_request_read;
  int  m_request_write;
  int  m_response_read;
  int  m_response_write;

  std::array<int, 2> m_request_pipe;
  std::array<int, 2> m_response_pipe;

  /* Helper functions */
  bool check_for_ready();

  bool              send_request_header(Command cmd, int32_t size);
  bool              send_data(std::vector<uint8_t> &data);
  std::vector<uint8_t> receive_status_header(std::optional<std::chrono::milliseconds> timeout = std::nullopt);
  bool              receive_data(std::vector<uint8_t> &data);

};
}  // namespace testing

#endif  // !VP_COMM_HPP
