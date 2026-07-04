MESSAGE(STATUS "Adding game - dyro_game")
ADD_EXECUTABLE(dyro_game)

# Sources
TARGET_SOURCES(dyro_game PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/private/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/private/demo_game.h
    ${CMAKE_CURRENT_LIST_DIR}/private/demo_game.cpp)

# Create the project filters
GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/private private)

# Dependencies
TARGET_LINK_LIBRARIES(dyro_game PRIVATE dyro_engine)

# Compile all shaders in the /shaders folder as part of the build
DYRO_COMPILE_SHADERS(dyro_game)

# Wait for the shared /content folder to be copied next to the exe
ADD_DEPENDENCIES(dyro_game copy-content)

# Target properties
SET_TARGET_PROPERTIES(dyro_game PROPERTIES
                      FOLDER "games"
                      VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:dyro_game>")
