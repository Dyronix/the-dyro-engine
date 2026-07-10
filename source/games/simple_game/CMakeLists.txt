MESSAGE(STATUS "Adding game - simple_game")
ADD_EXECUTABLE(simple_game)

# Sources: every .cpp/.h file under the private/ folder is part of this game.
# You never edit this file to add code - create the new file inside
# source/games/simple_game/private (NOT inside the build/ folder!) and build.
# CMake notices the new file, regenerates the project, and visual studio asks
# to reload; removing or renaming files works the same way.
FILE(GLOB_RECURSE game_sources CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_LIST_DIR}/private/*.cpp"
     "${CMAKE_CURRENT_LIST_DIR}/private/*.h"
     "${CMAKE_CURRENT_LIST_DIR}/private/*.hpp")
TARGET_SOURCES(simple_game PRIVATE ${game_sources})

# Create the project filters
GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/private private)

# This starter game keeps main.cpp deliberately simple and does not parse the
# command line, so - unlike buz_game - it does not need to link the args library.
TARGET_LINK_LIBRARIES(simple_game PRIVATE buz_engine)

# Compile all shaders in the /shaders folder as part of the build
BUZ_COMPILE_SHADERS(simple_game)

# Wait for the shared /content folder to be copied next to the exe
ADD_DEPENDENCIES(simple_game copy-content)

# Warn on every build about source files accidentally created in build/
ADD_DEPENDENCIES(simple_game check_stray_sources)

# Target properties
SET_TARGET_PROPERTIES(simple_game PROPERTIES
                      FOLDER "games"
                      VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:simple_game>")
