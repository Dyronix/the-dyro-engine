MESSAGE(STATUS "Adding game - dyro_game")
ADD_EXECUTABLE(dyro_game)

# Sources
TARGET_SOURCES(dyro_game PRIVATE
    ${SOURCE_GAME_DIRECTORY}/private/main.cpp
    ${SOURCE_GAME_DIRECTORY}/private/demo_game.h
    ${SOURCE_GAME_DIRECTORY}/private/demo_game.cpp)

# Create the project filters
GROUPSOURCES(${SOURCE_GAME_DIRECTORY}/private private)

# Dependencies
TARGET_LINK_LIBRARIES(dyro_game PRIVATE dyro_engine)

# Compile all shaders in the /shaders folder as part of the build
DYRO_COMPILE_SHADERS(dyro_game)

# Target properties
SET_TARGET_PROPERTIES(dyro_game PROPERTIES
                      FOLDER "games"
                      VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:dyro_game>")
