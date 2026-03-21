cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

cmake_policy(SET CMP0091 NEW)

set(TARGET BlackScreen)
project(${TARGET} VERSION 1.0 LANGUAGES C CXX)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

add_executable( ${TARGET} WIN32
  BlackScreen.cpp
)

target_compile_definitions( ${TARGET} PUBLIC
  NOMINMAX
  WIN32_LEAN_AND_MEAN
)

set_target_properties( ${TARGET} PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_STANDARD 98

  WIN32_EXECUTABLE TRUE
)

target_link_libraries(${TARGET} PUBLIC
  Gdi32
)

if ( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
  target_link_options( ${TARGET} PUBLIC
    /DEBUG
  )
endif()


if( ${BUILD_SHARED_LIBS} )
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(${TARGET} PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
  endif()

else()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(${TARGET} PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
  else()
    target_link_libraries(${TARGET} PUBLIC
      -static
      -static-libgcc
      -static-libstdc++
    )
  endif()

endif()
