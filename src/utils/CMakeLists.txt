
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_library(Utils SHARED ${SOURCES})
configure_project(Utils ${SOURCES})

target_link_libraries(Utils Config)
target_link_libraries(Utils cpputils)
target_include_directories(Utils PUBLIC public)