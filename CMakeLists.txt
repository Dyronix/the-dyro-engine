cmake_minimum_required(VERSION 3.21)

project(dyro_engine LANGUAGES CXX)

IF(NOT WIN32)
    MESSAGE(FATAL_ERROR "DyroEngine uses DirectX 12 and can only be built on Windows.")
ENDIF()

# Use C++ 17 as a standard
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folder structure in the IDE
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# Set output directories for each configuration (Debug, Release, etc.)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)

# Debug postfix
SET(CMAKE_DEBUG_POSTFIX _d)

# Some easy access to folders within this repository
SET(SOURCE_ENGINE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/engine)
SET(SOURCE_TOOLS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/tools)
SET(SOURCE_GAMES_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/games)
SET(SOURCE_THIRDPARTY_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/third_party)
SET(SOURCE_SHADER_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/shaders)

# The heavier build logic lives in its own script per topic inside the /cmake
# folder; every script is commented if you are curious how it works.
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
INCLUDE(group_sources)       # GROUPSOURCES: mirror a folder tree as IDE filters
INCLUDE(compile_shaders)     # DYRO_COMPILE_SHADERS: compile the /shaders hlsl files
INCLUDE(copy_content)        # copy-content: copy /content next to the executable
INCLUDE(check_stray_sources) # warn about source files accidentally created in build/
INCLUDE(docs_target)         # optional "docs" target when doxygen is installed

ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY})
ADD_SUBDIRECTORY(${SOURCE_ENGINE_DIRECTORY})
ADD_SUBDIRECTORY(${SOURCE_TOOLS_DIRECTORY})
ADD_SUBDIRECTORY(${SOURCE_GAMES_DIRECTORY})

# Make the demo game the startup project when opening the solution in Visual
# Studio; right-click a different game and "Set as Startup Project" to debug
# that one instead.
SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT dyro_game)
