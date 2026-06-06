include(ExternalProject)

function(SetupAssetCooker COOKER_SRC_DIR COOKER_BIN_DIR OUT_EXE_VAR)
    set(COOKER_EXECUTABLE "${COOKER_BIN_DIR}/Source/Cooker${CMAKE_EXECUTABLE_SUFFIX}")

    ExternalProject_Add(AssetCooker
        SOURCE_DIR       "${COOKER_SRC_DIR}"
        BINARY_DIR       "${COOKER_BIN_DIR}"
        CMAKE_ARGS       -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        BUILD_ALWAYS     TRUE
        INSTALL_COMMAND  ""
        BUILD_BYPRODUCTS "${COOKER_EXECUTABLE}"
    )

    set(${OUT_EXE_VAR} "${COOKER_EXECUTABLE}" PARENT_SCOPE)
endfunction()

function(CookAssets COOKER_EXECUTABLE ASSET_SRC_DIR ASSET_OUT_DIR)
    file(GLOB_RECURSE MODEL_SOURCES CONFIGURE_DEPENDS
        "${ASSET_SRC_DIR}/*.gltf"
        "${ASSET_SRC_DIR}/*.glb"
        "${ASSET_SRC_DIR}/*.fbx"
        "${ASSET_SRC_DIR}/*.obj"
    )

    set(COOKED_OUTPUTS "")

    foreach(SRC ${MODEL_SOURCES})
        file(RELATIVE_PATH REL "${ASSET_SRC_DIR}" "${SRC}")
        get_filename_component(REL_DIR  "${REL}" DIRECTORY)
        get_filename_component(REL_STEM "${REL}" NAME_WE)

        if(REL_DIR)
            set(OUT_FILE "${ASSET_OUT_DIR}/${REL_DIR}/${REL_STEM}.hmodel")
        else()
            set(OUT_FILE "${ASSET_OUT_DIR}/${REL_STEM}.hmodel")
        endif()

        add_custom_command(
            OUTPUT  "${OUT_FILE}"
            COMMAND ${COOKER_EXECUTABLE} "${SRC}" "${OUT_FILE}"
            DEPENDS "${SRC}" ${COOKER_EXECUTABLE}
            COMMENT "Cooking ${REL}"
            VERBATIM
        )

        list(APPEND COOKED_OUTPUTS "${OUT_FILE}")
    endforeach()

    add_custom_target(CookAssets ALL DEPENDS ${COOKED_OUTPUTS})
    add_dependencies(CookAssets AssetCooker)
endfunction()
