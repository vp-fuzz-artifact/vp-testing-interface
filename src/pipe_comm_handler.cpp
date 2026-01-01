#include "vp_test/pipe_comm_handler.hpp"
#include "vp_test/base_logger.hpp"
#include "vp_test/conversion.hpp"
#include "vp_test/types.hpp"
#include <cstring>
#include <string>

namespace testing {

  PipeCommHandler::PipeCommHandler(int fd_requests, int fd_response)
    : is_comm_started(false),
      fd_request(fd_requests),
      fd_response(fd_response) {
}

bool PipeCommHandler::start() {
  // Writes "ready" string to Response pipe.
  std::string ready = "ready";
  ssize_t     written = write(fd_response, ready.c_str(), ready.size() + 1);
  if (written != -1 && written == (ssize_t)ready.size() + 1) {
    TESTING_LOG(testing::LogLevel::Info,
                "Communication ready, waiting for requests.");
  } else {
    TESTING_LOG(testing::LogLevel::Error,
                std::string("Error sending ready message: ") + strerror(errno));
    return false;
  }

  is_comm_started = true;

  return true;
}

PipeCommHandler::~PipeCommHandler() {
  // Closes bothe pipes.
  close(fd_request);
  close(fd_request);
}

bool PipeCommHandler::send_response(Response &res) const {
  // Check if communication started.
  if (!is_comm_started) {
    TESTING_LOG(testing::LogLevel::Info, "Communication not started!");
    return false;
  }

  // Creating a buffer for the command and data length.
  uint8_t buffer[sizeof(uint32_t) + 1];

  // Copy status and data length into one buffer.
  buffer[0] = res.response_status;
  testing::conversion::int32_to_bytes(res.data.size(), buffer, 1);

  // Write the Response status.
  ssize_t written = write(fd_response, buffer, sizeof(uint32_t) + 1);
  if (written == -1) {
    TESTING_LOG(testing::LogLevel::Error,
                "Could not send command and data length to the Response pipe!");
    return false;
  }

  // Write the whole data at once.
  written = write(fd_response, res.data.data(), res.data.size());
  if (written == -1) {
    TESTING_LOG(testing::LogLevel::Error,
                "Could not send the data to the Response pipe!");
    return false;
  }

  return true;
}

bool PipeCommHandler::receive_request() {
  // Request structure:
  // 0     Byte: Command
  // 1 - 4 Byte: Data length (uint32)
  // 5 - ? Byte: Data

  // Check if communication started.
  if (!is_comm_started) {
    TESTING_LOG(testing::LogLevel::Error, "Communication not started!");
    return false;
  }

  // Creating a buffer for the command and data length (see request structure).
  uint8_t buffer[sizeof(uint32_t) + 1];

  // Read the length. This is blocking until sizeof(length) bytes are there (or
  // an error).
  size_t  length = 0;
  ssize_t bytes_read = read(fd_request, buffer, sizeof(uint32_t) + 1);
  
  if (bytes_read != sizeof(uint32_t) + 1) {
    TESTING_LOG(testing::LogLevel::Error,
                "There was an error reading the command and "
                "length of the Request from the Request pipe.");
    return false;
  }

  if (bytes_read < 1) {
    TESTING_LOG(testing::LogLevel::Error,
                "Message was too short for a valid Request!");
    return false;
  }

  // Creating new Request.
  current_req = Request();

  current_req.request_command = static_cast<testing::Command>(buffer[0]);
  current_req.data.resize(testing::conversion::bytes_to_int32(buffer, 1));

  if (current_req.data.size() <= 1) { return true; }

    /* If we have some data to read, let's start reading the command payload: */
  int    error_count = 0;
  size_t received_length = 0;
  while (received_length < current_req.data.size()) {
    
    // Read as much data as possible (up to the wanted length).
    bytes_read = read(fd_request, current_req.data.data() + received_length,
                      current_req.data.size() - received_length);

    // Error handling
    if (bytes_read == -1) {
      TESTING_LOG(testing::LogLevel::Error,
                  "There was an error reading the data of the "
                  "Request from the Request pipe.");
      error_count++;
    } else if (bytes_read == 0) {
      TESTING_LOG(
          testing::LogLevel::Error,
          "Request pipe end of data reached, but not full data received (" +
              std::to_string(received_length) + " of " +
              std::to_string(current_req.data.size()) + ") ");
      error_count++;
    } else {
      received_length += bytes_read;
    }

    if (error_count >= PIPE_READ_ERROR_MAX) {
      TESTING_LOG(testing::LogLevel::Error,
                  "Maximum of " + std::to_string(PIPE_READ_ERROR_MAX) +
                      " error reached while receiving data.");

      current_req.data.clear();

      return false;
    }
  }

  return true;
}

bool PipeCommHandler::is_started() const {
  return is_comm_started;
}

void PipeCommHandler::respond_malformed(Response &res) {
  res.response_status = STATUS_MALFORMED;
}

Request PipeCommHandler::get_request() {
  return current_req;
}

};  // namespace testing
