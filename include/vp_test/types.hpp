#ifndef TESTING_TYPES_H
#define TESTING_TYPES_H

#include <map>
#include <string>
#include <vector>
#define MQ_MAX_LENGTH 256
#define MQ_MAX_MSG 10

#define PIPE_READ_ERROR_MAX 5

namespace testing {

// Possible commands.
enum class Command {
  KILL,
  ENABLE_MMIO_TRACKING,
  ENABLE_CODE_COVERAGE,
  GET_CODE_COVERAGE_SHM,
  GET_RETURN_CODE,
  DO_RUN_SHM,
  INIT_SHM,
  RUN_INIT
};

extern std::map<testing::Command, int>         command_length;
extern std::map<testing::Command, std::string> command_string;

// Possible return status codes.
enum status { STATUS_OK, STATUS_ERROR, STATUS_MALFORMED };

extern std::map<testing::status, std::string> status_string;

// Represents a request send to the implemented testing interface with a command
// ID and flexible length data.
struct Request {
  Command           request_command;
  std::vector<uint8_t> data;
};

// Represents a response of the testing interface, which contains of an response
// status and the response data.
struct Response {
  status            response_status;
  std::vector<uint8_t> data;
};
}  // namespace testing

#endif
