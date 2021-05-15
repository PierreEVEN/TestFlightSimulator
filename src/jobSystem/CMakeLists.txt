
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_library(JobSystem ${SOURCES})
configure_project(JobSystem ${SOURCES})

target_link_libraries(JobSystem Utils)
target_include_directories(JobSystem PUBLIC public)