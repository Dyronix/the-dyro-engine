MESSAGE(STATUS "Adding tool - shader_compiler")
ADD_EXECUTABLE(shader_compiler)

# Sources
TARGET_SOURCES(shader_compiler PRIVATE
    ${SOURCE_TOOLS_DIRECTORY}/shader_compiler/private/shader_compiler_main.cpp)

# Create the project filters
GROUPSOURCES(${SOURCE_TOOLS_DIRECTORY}/shader_compiler/private private)

# The dxc compiler api (dxcapi.h + dxcompiler.lib) ships with the windows sdk
TARGET_LINK_LIBRARIES(shader_compiler PRIVATE dxcompiler)

# Target properties
SET_TARGET_PROPERTIES(shader_compiler PROPERTIES FOLDER "tools")

# -------------------------------
# dxcompiler.lib is only a thin wrapper: at runtime the tool needs
# dxcompiler.dll (the actual compiler) and dxil.dll (signs the bytecode so
# directx accepts it). Both ship with the windows sdk, so copy them from the
# newest installed sdk next to the tool executable.
# -------------------------------
GET_FILENAME_COMPONENT(WINDOWS_KITS_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE)

FILE(GLOB DXCOMPILER_DLL_CANDIDATES "${WINDOWS_KITS_ROOT}/bin/10.*/x64/dxcompiler.dll")
LIST(SORT DXCOMPILER_DLL_CANDIDATES)
LIST(POP_BACK DXCOMPILER_DLL_CANDIDATES DXCOMPILER_DLL)

IF(NOT DXCOMPILER_DLL)
    MESSAGE(FATAL_ERROR "Could not find dxcompiler.dll in any installed windows sdk (looked in ${WINDOWS_KITS_ROOT}/bin). Install a recent windows 10/11 sdk.")
ENDIF()

GET_FILENAME_COMPONENT(DXC_BIN_DIRECTORY ${DXCOMPILER_DLL} DIRECTORY)
MESSAGE(STATUS "Using dxc from: ${DXC_BIN_DIRECTORY}")

ADD_CUSTOM_COMMAND(TARGET shader_compiler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DXC_BIN_DIRECTORY}/dxcompiler.dll" "$<TARGET_FILE_DIR:shader_compiler>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DXC_BIN_DIRECTORY}/dxil.dll" "$<TARGET_FILE_DIR:shader_compiler>"
    COMMENT "Copying dxcompiler.dll and dxil.dll next to shader_compiler"
    VERBATIM)
