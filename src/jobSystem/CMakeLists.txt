
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_library(JobSystem ${SOURCES})
construct_target(JobSystem . ${SOURCES})

target_link_libraries(JobSystem Utils)