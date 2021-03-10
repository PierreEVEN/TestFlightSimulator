
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_library(Utils ${SOURCES})
construct_target(Utils . ${SOURCES})

target_link_libraries(Utils Config)