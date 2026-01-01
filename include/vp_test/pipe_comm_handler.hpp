#ifndef PIPE_COMM_HANDLER_HPP
#define PIPE_COMM_HANDLER_HPP

#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>

#include "types.hpp"

namespace testing {

// testing_communication implementation for pipe communication.
class PipeCommHandler {
 public:
  // Creates a pipe communication interface with a request and response file
  // descriptor. Both pipes with the corresponding FDs must be created before.
  explicit PipeCommHandler(int fd_requests, int fd_response);

  ~PipeCommHandler();

  // Function returning the started indicator.
  [[nodiscard]] bool is_started() const;

  Request get_request();

  static void respond_malformed(Response &res);

  // Implemented start function which writes the string "ready" to response
  // pipe.
  bool start();

  // Implemented function to send a response. This will write the response
  // status, data length and then the data to the response pipe.
  bool send_response(Response &res) const;

  // Implemented function that checks for new requests. This function checks the
  // request pipe for new request and saves the first into the temporary
  // m_current_req object.
  bool receive_request();

 private:
  // Object holding the latest request.
  Request current_req;

  // Indicates if the communication was started.
  bool is_comm_started;
  // File desciptors of the request (read) and response (write) pipes.
  int fd_request;
  int fd_response;
};

}  // namespace testing

#endif
