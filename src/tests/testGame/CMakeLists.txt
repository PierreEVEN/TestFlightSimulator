
file(GLOB_RECURSE SOURCES *.cpp *.h)
add_executable(Game_test ${SOURCES})
configure_project(Game_test ${SOURCES})
target_link_libraries(Game_test GameEngine)

set_target_properties(Game_test PROPERTIES FOLDER Tests)
target_include_directories(GameEngine PUBLIC public)