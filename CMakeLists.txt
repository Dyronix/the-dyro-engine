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
SET(SOURCE_GAME_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/game)
SET(SOURCE_THIRDPARTY_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/third_party)
SET(SOURCE_SHADER_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/shaders)

# -------------------------------
# Macro that will define our source groups
# -------------------------------
MACRO(GROUPSOURCES curdir folder_name)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    FOREACH(child ${children})
           IF(IS_DIRECTORY ${curdir}/${child})
              GROUPSOURCES(${curdir}/${child} ${folder_name}/${child})
           ELSE()
              SOURCE_GROUP(${folder_name} FILES ${curdir}/${child})
           ENDIF()
    ENDFOREACH()
ENDMACRO()

# -------------------------------
# Function that hooks shader compilation into the build.
#
# Every *.hlsl file inside the /shaders folder is compiled to a *.cso file
# in the runtime output folder (content/shaders) using our own shader_compiler
# tool. CMake tracks the dependencies for us: a shader is only recompiled when
# its source file changed or when it has not been compiled yet, exactly like
# regular C++ files.
#
# The shader profile is derived from the file name suffix:
#     my_shader_vs.hlsl -> vertex shader  (vs_6_0)
#     my_shader_ps.hlsl -> pixel shader   (ps_6_0)
# -------------------------------
FUNCTION(DYRO_COMPILE_SHADERS target_name)
    FILE(GLOB shader_sources CONFIGURE_DEPENDS "${SOURCE_SHADER_DIRECTORY}/*.hlsl")

    # Stop visual studio from compiling the hlsl files with its own built-in
    # shader compiler; our shader_compiler tool does the compiling.
    SET_SOURCE_FILES_PROPERTIES(${shader_sources} PROPERTIES VS_TOOL_OVERRIDE "None")

    SET(compiled_shaders)
    FOREACH(shader_source ${shader_sources})
        GET_FILENAME_COMPONENT(shader_name ${shader_source} NAME_WE)

        IF(shader_name MATCHES "_vs$")
            SET(shader_profile "vs_6_0")
        ELSEIF(shader_name MATCHES "_ps$")
            SET(shader_profile "ps_6_0")
        ELSE()
            MESSAGE(FATAL_ERROR "Cannot derive a shader profile from \"${shader_name}.hlsl\". Shader files must end in _vs or _ps.")
        ENDIF()

        SET(shader_output "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/content/shaders/${shader_name}.cso")

        ADD_CUSTOM_COMMAND(
            OUTPUT ${shader_output}
            COMMAND shader_compiler ${shader_source} ${shader_output} ${shader_profile}
            DEPENDS ${shader_source} shader_compiler
            COMMENT "Compiling shader ${shader_name}.hlsl (${shader_profile})"
            VERBATIM)

        LIST(APPEND compiled_shaders ${shader_output})
    ENDFOREACH()

    ADD_CUSTOM_TARGET(compile_shaders DEPENDS ${compiled_shaders} SOURCES ${shader_sources})
    SET_PROPERTY(TARGET compile_shaders PROPERTY FOLDER "events")
    ADD_DEPENDENCIES(${target_name} compile_shaders)
ENDFUNCTION()

ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY})
ADD_SUBDIRECTORY(${SOURCE_ENGINE_DIRECTORY})
ADD_SUBDIRECTORY(${SOURCE_TOOLS_DIRECTORY})
ADD_SUBDIRECTORY(${SOURCE_GAME_DIRECTORY})

# Make the game the startup project when opening the solution in Visual Studio
SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT dyro_game)

# -------------------------------
# Copy the content folder (textures, ...) next to the executable so the
# engine can load assets with paths relative to the executable.
# -------------------------------
FILE(GLOB_RECURSE CONTENT_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/content/*")
SOURCE_GROUP(TREE "${CMAKE_SOURCE_DIR}/content" PREFIX "content" FILES ${CONTENT_FILES})

ADD_CUSTOM_TARGET(copy-content ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/content" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/content"
    COMMENT "Copying \"${CMAKE_SOURCE_DIR}/content\" folder to \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/content\""
    SOURCES ${CONTENT_FILES}
    VERBATIM)

SET_PROPERTY(TARGET copy-content PROPERTY FOLDER "events")
ADD_DEPENDENCIES(dyro_game copy-content)
