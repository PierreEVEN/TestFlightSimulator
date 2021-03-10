
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_library(Config ${SOURCES})
construct_target(Config . ${SOURCES})