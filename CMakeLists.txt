cmake_minimum_required(VERSION 2.6)

include(CheckCXXCompilerFlag)
include(FindPkgConfig)

find_package(Qt4 4.5.0 REQUIRED QtCore QtGui QtNetwork QtWebKit QtSql)
find_package(PkgConfig REQUIRED)

pkg_check_modules(QJSON REQUIRED QJson)

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions(-DNDEBUG)
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif(${CMAKE_BUILD_TYPE} MATCHES "Release")

# Set up definitions and paths
add_definitions(${QT_DEFINITIONS})
link_directories(${QJSON_LIBRARY_DIRS})

include(${QT_USE_FILE})
include_directories(${QJSON_INCLUDE_DIRS})

# Subdirectories
add_subdirectory(src)

# Uninstall support
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

