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
    # Shaders live in one shared /shaders folder, so every game that calls this
    # function depends on the same "compile_shaders" target; only build it once.
    IF(NOT TARGET compile_shaders)
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
    ENDIF()

    ADD_DEPENDENCIES(${target_name} compile_shaders)
ENDFUNCTION()
