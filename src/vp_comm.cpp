#include <sys/types.h>
#include <cerrno>
#include <cstdio>
#include "vp_test/conversion.hpp"
#include "vp_test/types.hpp"
#include "vp_test/vp_comm.hpp"
#include <sys/ioctl.h>
#include <unistd.h>

namespace testing {

VpComm::VpComm()
    : comm_state(CommState::UNINITIALIZED),
      m_request_read(100),
      m_request_write(101),
      m_response_read(102),
      m_response_write(103),
      m_request_pipe({-1, -1}),
      m_response_pipe({-1, -1}) {
}

VpComm::~VpComm() {
  if (comm_state == CommState::UNINITIALIZED) { return; }

  close(m_request_read);
  close(m_request_write);
  close(m_response_read);
  close(m_response_write);
}

void VpComm::close_temp_pipes() {
  close(m_request_pipe[0]);
  close(m_request_pipe[1]);
  close(m_response_pipe[0]);
  close(m_response_pipe[1]);
}

int VpComm::get_request_fd_0() const {
  return m_request_read;
}


int VpComm::get_request_fd_1() const {
  return m_request_write;
}


int VpComm::get_response_fd_0() const {
  return m_response_read;
}


int VpComm::get_response_fd_1() const {
  return m_response_write;
}


bool VpComm::init() 
{
  if (pipe(m_request_pipe.data()) == -1 || pipe(m_response_pipe.data()) == -1) {
    log_error_message("Error creating new pipes: %s", strerror(errno));
    return false;
  }

  if (dup2(m_request_pipe[0], m_request_read) == -1 ||
      dup2(m_request_pipe[1], m_request_write) == -1 || 
      dup2(m_response_pipe[0], m_response_read) == -1 || 
      dup2(m_response_pipe[1], m_response_write) == -1) {
    log_error_message("Error setting file descriptor of pipes: %s",
                      strerror(errno));
    return false;
  }

  close_temp_pipes();
  comm_state = CommState::INITIALIZED;
  return true;
}

void VpComm::reset() {
  comm_state = CommState::INITIALIZED; // The pipes exist, this correspond to an initialized state
}

void VpComm::set_ready() { 
  comm_state = CommState::READY;
}

/* Checks the byte available and returns true when the string ready was received */
bool VpComm::check_for_ready() {
  constexpr char ready_msg[] = "ready";

  if (comm_state != CommState::INITIALIZED) {
    log_error_message("Communication was not initialized!");
    return false;
  }

  
  int bytes_available = 0;
  if (ioctl(m_response_read, FIONREAD, &bytes_available) == -1) {
    perror("ioctl");
    return false;
  }
  if (bytes_available < 6) { return false; }

  char    buffer[6] = {0};
  ssize_t bytes_read = read(m_response_read, buffer, 6);
  if (bytes_read == -1) {
    log_error_message("An error occurred while waiting for ready message: %s",
                      strerror(errno));
    return false;
  }

  if (std::strncmp(buffer, ready_msg, sizeof(ready_msg) - 1) != 0) {
    log_info_message("Did not receive ready message");
    return false;
  }

  log_info_message("Received ready message");
  comm_state = CommState::READY;
  return true;
}

/* Checks is a loop if the string "ready" was received */
bool VpComm::wait_for_ready() {
  while (true) {
    if (check_for_ready()) break;
  }

  return true;
}


bool VpComm::receive_response(Response &res){
  // Check if communication started.
  if (comm_state != CommState::READY) {
    log_error_message("Communication is not ready!");
    return false;
  }

  std::vector<uint8_t> status_header = receive_status_header();
  if (status_header.empty()) { return false; }

  if (!res.data.empty()) { res.data.clear(); }

  res.response_status = static_cast<testing::status>(status_header[0]);

  if (res.response_status == STATUS_ERROR) {
    log_error_message("The status of the request indicated an error!");
    return false;
  } else if (res.response_status == STATUS_MALFORMED) {
    log_error_message("The the request was malformed!");
    return false;
  }

  res.data.resize(testing::conversion::bytes_to_int32(static_cast<const unsigned char*>(status_header.data()), 1));

  // Receive data if data is expected.
  if (!res.data.empty()) {
    if (!receive_data(res.data)) {
      res.response_status = STATUS_ERROR;
      res.data.clear();
    }
  }

  log_info_message("RECEIVED: %s with length %d.",
                   status_string[res.response_status].c_str(), res.data.size());

  return true;

}


bool VpComm::send_request(Request &req, Response &res, std::optional<std::chrono::milliseconds> timeout) {
  // Request structure:
  // 0     Byte: Command
  // 1 - 4 Byte: Data length (uint32)
  // 5 - ? Byte: Data

  // Response structure:
  // 0     Byte: Status
  // 1 - 4 Byte: Data length (uint32)
  // 5 - ? Byte: Data

  // Check if communication started.
  if (comm_state != CommState::READY) {
    log_error_message("Communication is not ready!");
    return false;
  }

  if (!send_request_header(req.request_command, req.data.size())) {
    return false;
  }
  if (!send_data(req.data)) {
    log_info_message("SENT: %s with length %d.",
                     command_string[req.request_command].c_str(),
                     req.data.size());

    return false;
  }

  std::vector<uint8_t> status_header = receive_status_header(timeout);
  if (status_header.empty()) { return false; }

  if (!res.data.empty()) { res.data.clear(); }

  res.response_status = static_cast<testing::status>(status_header[0]);

  if (res.response_status == STATUS_ERROR) {
    log_error_message("The status of the request indicated an error!");
    return false;
  } else if (res.response_status == STATUS_MALFORMED) {
    log_error_message("The the request was malformed!");
    return false;
  }

  res.data.resize(testing::conversion::bytes_to_int32(status_header.data(), 1));

  // Receive data if data is expected.
  if (!res.data.empty()) {
    if (!receive_data(res.data)) {
      res.response_status = STATUS_ERROR;
      res.data.clear();
    }
  }

  log_info_message("RECEIVED: %s with length %d.",
                   status_string[res.response_status].c_str(), res.data.size());

  return true;
}

bool VpComm::restore_send_request(Request &req, Response &res) {  // added this function to receive additional Status_OK leftover from restore
  // Request structure:
  // 0     Byte: Command
  // 1 - 4 Byte: Data length (uint32)
  // 5 - ? Byte: Data

  // Response structure:
  // 0     Byte: Status
  // 1 - 4 Byte: Data length (uint32)
  // 5 - ? Byte: Data

  // Check if communication started.
  if (comm_state != CommState::READY) {
    log_error_message("Communication is not ready!");
    return false;
  }

  if (!send_request_header(req.request_command, req.data.size())) {
    return false;
  }
  if (!send_data(req.data)) {
    log_info_message("SENT: %s with length %d.",
                     command_string[req.request_command].c_str(),
                     req.data.size());

    return false;
  }

  std::vector<uint8_t> status_header = receive_status_header();
  if (status_header.empty()) { return false; }

  if (!res.data.empty()) { res.data.clear(); }

  res.response_status = static_cast<testing::status>(status_header[0]);

  if (res.response_status == STATUS_ERROR) {
    log_error_message("The status of the request indicated an error!");
    return false;
  } else if (res.response_status == STATUS_MALFORMED) {
    log_error_message("The the request was malformed!");
    return false;
  }

  res.data.resize(testing::conversion::bytes_to_int32(status_header.data(), 1));

  // Receive data if data is expected.
  if (!res.data.empty()) {
    if (!receive_data(res.data)) {
      res.response_status = STATUS_ERROR;
      res.data.clear();
    }
  }

  log_info_message("RECEIVED: %s with length %d.",
                   status_string[res.response_status].c_str(), res.data.size());


  std::vector<uint8_t> status_header2 = receive_status_header();
  if (status_header2.empty()) { return false; }
  
  if (!res.data.empty()) { res.data.clear(); }

  res.response_status = static_cast<testing::status>(status_header2[0]);

  if (res.response_status == STATUS_ERROR) {
    log_error_message("The status of the request indicated an error!");
    return false;
  } else if (res.response_status == STATUS_MALFORMED) {
    log_error_message("The the request was malformed!");
    return false;
  }

  res.data.resize(testing::conversion::bytes_to_int32(status_header2.data(), 1));

  // Receive data if data is expected.
  if (!res.data.empty()) {
    if (!receive_data(res.data)) {
      res.response_status = STATUS_ERROR;
      res.data.clear();
    }
  }

  log_info_message("RECEIVED: %s with length %d.",
                   status_string[res.response_status].c_str(), res.data.size());
  return true;
}

bool VpComm::send_request_header(Command cmd, int32_t size) {
  uint8_t buffer[sizeof(uint32_t) + 1];

  buffer[0] = static_cast<uint8_t>(cmd);
  testing::conversion::int32_to_bytes(size, buffer, 1);

  if (write(m_request_write, buffer, sizeof(buffer)) == -1) {
    log_error_message(
        "Could not send command and data length to the request pipe: %s",
        strerror(errno));
    return false;
  }

  return true;
}

bool VpComm::send_data(std::vector<uint8_t> &data) {
  if (write(m_request_write, data.data(), data.size()) == -1) {
    log_error_message("Could not send data to the request pipe: %s",
                      strerror(errno));
    return false;
  }

  return true;
}

std::vector<uint8_t> VpComm::receive_status_header(std::optional<std::chrono::milliseconds> timeout) {
  std::vector<uint8_t> data(sizeof(uint32_t) + 1);

  if(timeout){
    fd_set read_fds;
    struct timeval tv;

    tv.tv_sec = timeout->count() / 1000;
    tv.tv_usec = (timeout->count() % 1000) * 1000;

    FD_ZERO(&read_fds);
    FD_SET(m_response_read, &read_fds);

    // Linux manual: nfds should be set to the highest-numbered fd of the set + 1.
    int ret = select(m_response_read + 1, &read_fds, nullptr, nullptr, &tv);
    if(ret == -1){
      log_error_message("Select failed");
      return {};
    }else if(ret == 0){
      log_error_message("Pipe communication has failed: a timeout occurred");
      return {};
    }
  }
  
  ssize_t bytes_read = read(m_response_read, data.data(), data.size());

  if (bytes_read != data.size()) {
    log_error_message(
        "There was an error reading the status and data length from the "
        "request pipe: %s",
        strerror(errno));
    log_error_message(
        "bytes read: %d. "
        "Should have read %d",
        bytes_read, data.size());
    return {};
  }

  return data;
}

bool VpComm::receive_data(std::vector<uint8_t> &data) {
  int    error_count = 0;
  size_t received_length = 0;

  while (received_length < data.size()) {
    ssize_t bytes_read = read(m_response_read, data.data() + received_length,
                              data.size() - received_length);

    if (bytes_read == -1) {
      log_error_message(
          "There was an error reading the request data from the "
          "request pipe: %s",
          strerror(errno));
      error_count++;
    } else if (bytes_read == 0) {
      log_error_message(
          "Request pipe end of data reached, but data was not fully received!");
      error_count++;
    } else {
      received_length += bytes_read;
    }

    if (error_count >= PIPE_READ_ERROR_MAX) {
      log_error_message("Maximum errors reached while receiving data.");
      return false;
    }
  }
  return true;
}

void VpComm::clear_pipe(int fd) const {
  char    buffer[1024];  // Temporary buffer for clearing
  ssize_t bytesRead;

  // Set the pipe to non-blocking mode to avoid hanging on empty reads
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  // Read until the pipe is empty
  while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
    log_info_message("Cleared %d bytes from pipe with fd %d.", bytesRead, fd);
  }

  if (bytesRead == -1 && errno != EAGAIN) {
    log_error_message("Error reading from pipe while clearing: %s",
                      strerror(errno));
  }

  // Restore original pipe flags
  fcntl(fd, F_SETFL, flags);
}
}  // namespace testing
