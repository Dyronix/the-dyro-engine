MESSAGE(STATUS "Adding game - buz_game")
ADD_EXECUTABLE(buz_game)

# Sources: every .cpp/.h file under the private/ folder is part of this game.
# You never edit this file to add code - create the new file inside
# source/games/buz_game/private (NOT inside the build/ folder!) and build.
# CMake notices the new file, regenerates the project, and visual studio asks
# to reload; removing or renaming files works the same way.
FILE(GLOB_RECURSE game_sources CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_LIST_DIR}/private/*.cpp"
     "${CMAKE_CURRENT_LIST_DIR}/private/*.h"
     "${CMAKE_CURRENT_LIST_DIR}/private/*.hpp")
TARGET_SOURCES(buz_game PRIVATE ${game_sources})

# Create the project filters
GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/private private)

# Dependencies. args is a small header-only command line parser used in main.cpp
# to override engine_settings at launch; new games link it the same way.
TARGET_LINK_LIBRARIES(buz_game PRIVATE buz_engine args)

# Compile all shaders in the /shaders folder as part of the build
BUZ_COMPILE_SHADERS(buz_game)

# Wait for the shared /content folder to be copied next to the exe
ADD_DEPENDENCIES(buz_game copy-content)

# Warn on every build about source files accidentally created in build/
ADD_DEPENDENCIES(buz_game check_stray_sources)

# Target properties
SET_TARGET_PROPERTIES(buz_game PROPERTIES
                      FOLDER "games"
                      VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:buz_game>")
