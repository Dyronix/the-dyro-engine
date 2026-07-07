MESSAGE(STATUS "Adding third party libraries")

ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/stb)
ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/rnd)
ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/args)
ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/nlohmann)

# glm is vendored as the full upstream repository, which ships its own build
# scripts (tests, install rules, ...) that we do not need. Instead of adding
# that subdirectory we define our own thin header-only target here, following
# the same pattern as nlohmann.
ADD_LIBRARY(glm INTERFACE)
TARGET_INCLUDE_DIRECTORIES(glm INTERFACE ${SOURCE_THIRDPARTY_DIRECTORY}/glm)
