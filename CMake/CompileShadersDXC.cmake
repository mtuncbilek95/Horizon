find_program(DXC_EXECUTABLE dxc
    HINTS
        "$ENV{WindowsSdkVerBinPath}/x64"
        "$ENV{VULKAN_SDK}/Bin"
    REQUIRED
)

function(CompileShader SRC PROFILE ENTRY OUT_NAME)
    set(OUT_FILE "${SHADER_OUT_DIR}/${OUT_NAME}")

    file(GLOB SHADER_INCLUDES "${SHADER_SRC_DIR}/*.hlsli")

    add_custom_command(
        OUTPUT  ${OUT_FILE}
        COMMAND ${DXC_EXECUTABLE}
                -T ${PROFILE}
                -E ${ENTRY}
                $<$<CONFIG:Debug>:-Zi>
                $<$<CONFIG:Debug>:-Od>
                $<$<CONFIG:Debug>:-Qembed_debug>
                $<$<CONFIG:Debug>:-Zss>
                $<$<NOT:$<CONFIG:Debug>>:-O3>
                -Fo ${OUT_FILE}
                ${SHADER_SRC_DIR}/${SRC}
        DEPENDS ${SHADER_SRC_DIR}/${SRC} ${SHADER_INCLUDES}
        COMMENT "Compiling ${SRC} -> ${OUT_NAME}"
        VERBATIM
    )

    set(SHADER_OUTPUTS ${SHADER_OUTPUTS} ${OUT_FILE} PARENT_SCOPE)
endfunction()
