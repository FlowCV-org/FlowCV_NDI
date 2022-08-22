
# NDI external Library Dependency
IF( DEFINED ENV{NDI_SDK_DIR} )
    SET( NDI_DIR "$ENV{NDI_SDK_DIR}" )
ELSE()
    find_path(NDI_DIR "NDI_DIR")
ENDIF()

if ("${NDI_DIR}" STREQUAL "NDI_DIR-NOTFOUND")
    if(APPLE)
        set(NDI_DIR "/Library/NDI SDK for Apple")
        if (NOT EXISTS ${NDI_DIR})
            message(FATAL_ERROR "No NDI SDK Directory Found")
        endif()
    else()
        message(FATAL_ERROR "No NDI SDK Directory Found")
    endif()
endif()

message("Found NDI SDK Dir: ${NDI_DIR}")
include_directories("${NDI_DIR}/include")

if (WIN32)
    if (NOT "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        message(FATAL_ERROR "Only 64-bit supported on Windows")
    endif()
    LIST(APPEND NDI_LIBS ${NDI_DIR}/Lib/x64/Processing.NDI.Lib.x64.lib)
    # Copy DLL to Build Dir
    file(COPY "${NDI_DIR}/Bin/x64/Processing.NDI.Lib.x64.dll" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
elseif(APPLE)
    LIST(APPEND NDI_LIBS ${NDI_DIR}/lib/macOS/libndi.dylib)
else()
    LIST(APPEND NDI_LIBS ${NDI_DIR}/lib/x86_64-linux-gnu/libndi.so)
endif()
