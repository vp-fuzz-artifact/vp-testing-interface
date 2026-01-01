cmake_minimum_required(VERSION 3.12)

project(vp-testing-interface)
set(PROJECT_VERSION v0.5)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(SOURCES
    ${SRC}/pipe_comm_handler.cpp
    ${SRC}/vp_comm.cpp
    ${SRC}/conversion.cpp
    ${SRC}/base_logger.cpp
    ${SRC}/types.cpp
)

add_library(vp-testing-interface SHARED ${SOURCES})

target_include_directories(vp-testing-interface
  PUBLIC
    $<BUILD_INTERFACE:${INC}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(vp-testing-interface PROPERTIES
                      CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

target_link_libraries(vp-testing-interface PUBLIC pthread rt)

install(TARGETS vp-testing-interface
  EXPORT      vp-testing-interface-targets
  ARCHIVE     DESTINATION ${CMAKE_INSTALL_LIBDIR}       # .a
  LIBRARY     DESTINATION ${CMAKE_INSTALL_LIBDIR}       # .so / .dylib
  RUNTIME     DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${INC}/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vp-testing-interfaceConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vp-testing-interfaceConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vp-testing-interface"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/vp-testing-interfaceConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(EXPORT vp-testing-interface-targets
  FILE vp-testing-interfaceTargets.cmake
  NAMESPACE vp-testing-interface::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vp-testing-interface
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/vp-testing-interfaceConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/vp-testing-interfaceConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vp-testing-interface
)
