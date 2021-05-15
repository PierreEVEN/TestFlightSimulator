
file(GLOB_RECURSE SOURCES ./*)
add_library(Config ${SOURCES})
configure_project(Config ${SOURCES})
target_include_directories(Config PUBLIC public)