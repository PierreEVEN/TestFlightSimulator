
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_executable(JobSystem_Test ${SOURCES})
construct_target(JobSystem_Test . ${SOURCES})
target_link_libraries(JobSystem_Test JobSystem)

set_target_properties(JobSystem_Test PROPERTIES FOLDER Tests)