cmake_minimum_required(VERSION 3.21)
project(FlowCV_NDI)

set(CMAKE_CXX_STANDARD 17)

include(CMake/NDI_Config.cmake)

find_package( FlowCV REQUIRED )

add_subdirectory(NDI_Sender)
add_subdirectory(NDI_Receiver)

