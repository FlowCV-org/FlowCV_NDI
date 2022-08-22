project(NDI_Sender)

add_library(
        ${PROJECT_NAME} SHARED
        ndi_sender.cpp
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

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        SUFFIX ".fp"
        )