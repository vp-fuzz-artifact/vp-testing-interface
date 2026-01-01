#include "vp_test/types.hpp"

namespace testing {

/* This is the payload required for each command (0 when we don't need a payload) */
std::map<testing::Command, int> command_length{
    {testing::Command::ENABLE_CODE_COVERAGE, 0},
    {testing::Command::ENABLE_MMIO_TRACKING, 0},
    {testing::Command::GET_CODE_COVERAGE_SHM, 0},
    {testing::Command::DO_RUN_SHM, 0},
    {testing::Command::RUN_INIT, 21},
    {testing::Command::GET_RETURN_CODE, 0},
    {testing::Command::KILL, 1}};

std::map<testing::Command, std::string> command_string{
    {testing::Command::ENABLE_CODE_COVERAGE, "ENABLE_CODE_COVERAGE"},
    {testing::Command::ENABLE_MMIO_TRACKING, "ENABLE_MMIO_TRACKING"},
    {testing::Command::GET_CODE_COVERAGE_SHM, "GET_CODE_COVERAGE_SHM"},
    {testing::Command::DO_RUN_SHM, "DO_RUN_SHM"},
    {testing::Command::RUN_INIT, "RUN_INIT"},
    {testing::Command::GET_RETURN_CODE, "GET_RETURN_CODE"},
    {testing::Command::KILL, "KILL"}};

std::map<testing::status, std::string> status_string{
    {STATUS_OK, "STATUS_OK"},
    {STATUS_MALFORMED, "STATUS_MALFORMED"},
    {STATUS_ERROR, "STATUS_ERROR"}};
}  // namespace testing
