MESSAGE(STATUS "Adding module - dyro_engine")
ADD_LIBRARY(dyro_engine STATIC)

# Sources
TARGET_SOURCES(dyro_engine PRIVATE
    # core
    ${SOURCE_ENGINE_DIRECTORY}/private/core/assert.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/core/json.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/core/log.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/core/noise.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/core/paths.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/core/random.cpp
    # graphics
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/d3d_utils.h
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/adapter_selection.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/shader_model.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/device.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/command_queue.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/descriptor_heap.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/swap_chain.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/shader_library.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/pso_cache.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/texture_loader.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/graphics/renderer_2d.cpp
    # application
    ${SOURCE_ENGINE_DIRECTORY}/private/application/input.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/application/window.cpp
    ${SOURCE_ENGINE_DIRECTORY}/private/application/engine.cpp)

TARGET_SOURCES(dyro_engine PUBLIC
    # core
    ${SOURCE_ENGINE_DIRECTORY}/public/core/assert.h
    ${SOURCE_ENGINE_DIRECTORY}/public/core/json.h
    ${SOURCE_ENGINE_DIRECTORY}/public/core/log.h
    ${SOURCE_ENGINE_DIRECTORY}/public/core/math.h
    ${SOURCE_ENGINE_DIRECTORY}/public/core/noise.h
    ${SOURCE_ENGINE_DIRECTORY}/public/core/paths.h
    ${SOURCE_ENGINE_DIRECTORY}/public/core/random.h
    ${SOURCE_ENGINE_DIRECTORY}/public/core/rect.h
    ${SOURCE_ENGINE_DIRECTORY}/public/core/timer.h
    # graphics
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/adapter_selection.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/shader_model.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/device.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/command_queue.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/descriptor_heap.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/swap_chain.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/shader_library.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/pso_cache.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/texture.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/texture_loader.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/font.h
    ${SOURCE_ENGINE_DIRECTORY}/public/graphics/renderer_2d.h
    # application
    ${SOURCE_ENGINE_DIRECTORY}/public/application/input.h
    ${SOURCE_ENGINE_DIRECTORY}/public/application/window.h
    ${SOURCE_ENGINE_DIRECTORY}/public/application/game.h
    ${SOURCE_ENGINE_DIRECTORY}/public/application/engine.h)

# Create the project filters
GROUPSOURCES(${SOURCE_ENGINE_DIRECTORY}/private private)
GROUPSOURCES(${SOURCE_ENGINE_DIRECTORY}/public public)

# Include self
TARGET_INCLUDE_DIRECTORIES(dyro_engine PRIVATE ${SOURCE_ENGINE_DIRECTORY}/private)
TARGET_INCLUDE_DIRECTORIES(dyro_engine PUBLIC ${SOURCE_ENGINE_DIRECTORY}/public)

# Stop windows.h from defining min/max macros, which would break glm::min,
# std::min and friends. PUBLIC so game code gets the same treatment.
TARGET_COMPILE_DEFINITIONS(dyro_engine PUBLIC NOMINMAX)

# Dependencies
# glm, fmt and nlohmann are PUBLIC because the public headers use them:
# math.h exposes the glm types, log.h and assert.h format their messages
# with fmt, and json.h exposes the nlohmann json type.
TARGET_LINK_LIBRARIES(dyro_engine PUBLIC d3d12 dxgi glm fmt nlohmann)
TARGET_LINK_LIBRARIES(dyro_engine PRIVATE stb rnd)

# Target properties
SET_TARGET_PROPERTIES(dyro_engine PROPERTIES FOLDER "modules")
