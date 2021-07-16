cmake_minimum_required(VERSION 3.4)
project(Exercise06)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
if (GLEW_FOUND)

include_directories( ${GLEW_INCLUDE_DIRS}  "C:\\Libraries\\glfw-3.3.bin.WIN64\\include")
set(Exercise06_SOURCES
        Exercise-6.cpp DrawPrimitives)
add_executable(Exercise06 ${Exercise06_SOURCES})
target_link_libraries (Exercise06 ${GLEW_LIBRARIES} "C:\\Libraries\\glfw-3.3.bin.WIN64\\lib-vc2019\\glfw3.lib" ${OPENGL_LIBRARIES}) 
endif()