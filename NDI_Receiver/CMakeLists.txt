project(NDI_Receiver)

add_library(
        ${PROJECT_NAME} SHARED
        ndi_receiver.cpp
        ${FlowCV_SRC}
        ${IMGUI_SRC}
        ${DSPatch_SRC}
)

target_link_libraries(
        ${PROJECT_NAME}
        ${OpenCV_LIBS}
        ${OpenVino_LIBS}
        ${NDI_LIBS}
)

if(WIN32)
set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        SUFFIX ".fp"
        )
elseif(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            SUFFIX ".fp"
            INSTALL_RPATH "${ORIGIN}"
            BUILD_WITH_INSTALL_RPATH ON
            )
elseif(APPLE)
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            SUFFIX ".fp"
            INSTALL_NAME_DIR "${ORIGIN}"
            BUILD_WITH_INSTALL_NAME_DIR ON
            )
endif()
